#include "Headers/Header.h"
#include "../import/include/VExDebugger.h"
#include "VEH/VEH.h"
#include "Tools/WinWrap.h"
#include "Tools/Logs.h"
#include "Config/Config.h"
#include "Headers/LogsException.hpp"
#include "HwBkp/MgrHwBkp.h"


//std::vector<ExceptionInfo> ExceptionAddressList =
//{
//	//{ 0, { } },
//	//{ 2, { } },
//	//{ 4, { } },
//	//{ 8, { } }
//};

std::map<uintptr_t, ExceptionInfoList> AddressAssocExceptionList =
{
	//{ 0, { } },
	//{ 2, { } },
	//{ 4, { } },
	//{ 8, { } }
};

std::map<uintptr_t, BkpInfo> BreakpointList =
{
	//{ 0, 0 },
	//{ 2, 0 },
	//{ 4, 0 },
	//{ 8, 0 }
};

std::map<uintptr_t, ExceptionInfoList>& VExDebugger::GetAssocExceptionList( )
{
	return AddressAssocExceptionList;
}

std::map<uintptr_t, BkpInfo>& VExDebugger::GetBreakpointList( )
{
	return BreakpointList;
}


void* p_VExDebugger = nullptr;

void* OriginalHandlerFilter = nullptr;

long __stdcall HandlerFilter( EXCEPTION_POINTERS* pExceptionInfo )
{
	auto* const pContext			= pExceptionInfo->ContextRecord;

	auto* const pExceptionRec		= pExceptionInfo->ExceptionRecord;

	//DisplayContextLogs( pContext, pExceptionRec ); // tests

	constexpr auto STATUS_WX86_SINGLE_STEP = 0x4000001E; // CE check this old flag too

	auto ExceptionAddress			= reinterpret_cast<uintptr_t>( pExceptionRec->ExceptionAddress );

	if ( EXCEPTION_SINGLE_STEP == pExceptionRec->ExceptionCode || 
		STATUS_WX86_SINGLE_STEP == pExceptionRec->ExceptionCode )
	{
		// to identify which order address was triggered
		//auto const FlagPos			= s_cast<int>( pContext->Dr6 & 0xE ); // result vals, 0 / 2 / 4 / 8
		
		for ( const auto & [ Address, BpInfo ] : VExDebugger::GetBreakpointList( ) )
		{
			if ( BpInfo.Type != BkpType::Hardware )             // only support hardware breakpoint
				continue;

			if ( ( pContext->Dr6 & ( static_cast<uintptr_t>( 1 ) << BpInfo.Pos ) ) == 0 ) // check if this position was setted
				continue;

			// if it's doesnt have this address, add or update info
			auto & ExceptionList	= VExDebugger::GetAssocExceptionList( )[ Address ];

			auto & Info				= ExceptionList[ ExceptionAddress ];
		
			++Info.Details.Count;                               // inc occurrences

			Info.Details.ThreadId	= GetCurrentThreadId( );    // last thread triggered

			Info.Details.Ctx		= *pContext;                // save context

			if ( Config::i( )->m_Logs )
				DisplayContextLogs( pContext, pExceptionRec );  // save in txt

			pContext->Dr6 = 0xFFFF0FF0;               // reset switch

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	if ( OriginalHandlerFilter )
		return reinterpret_cast<decltype( HandlerFilter )*>( OriginalHandlerFilter )( pExceptionInfo );

	return EXCEPTION_EXECUTE_HANDLER;
}

bool VExDebugger::StartMonitorAddress( const uintptr_t Address, const HwbkpType Type, const HwbkpSize Size )
{
	return MgrHwBkp::SetBkpAddressInAllThreads( Address, Type, Size );
}

void VExDebugger::RemoveMonitorAddress( const uintptr_t Address )
{
	return MgrHwBkp::RemoveBkpAddressInAllThreads( Address );
}

bool VExDebugger::Init( HandlerType Type, bool SpoofHwbkp, bool Logs )
{
	Config::i( )->m_HandlerType = Type;

	Config::i( )->m_SpoofHwbkp	= SpoofHwbkp;

	Config::i( )->m_Logs		= Logs;

	if ( !WinWrap::Init( ) )
		return false;

	nLog::Init( );

	switch ( Type )
	{
	case HandlerType::VectoredExceptionHandler:
		p_VExDebugger = RtlAddVectoredExceptionHandler( 1, HandlerFilter );
		break;
	case HandlerType::UnhandledExceptionFilter:
		p_VExDebugger = SetUnhandledExceptionFilter( HandlerFilter );
		break;
	case HandlerType::VectoredExceptionHandlerIntercept:
		return VEH_Internal::InterceptVEHHandler( HandlerFilter, OriginalHandlerFilter );
	default:
		return false;
	}

	return true;
}

