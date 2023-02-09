#include "Headers/Header.h"
#include "../import/include/VExDebugger.h"
#include "VEH/VEH.h"
#include "Tools/WinWrap.h"
#include "Tools/Logs.h"
#include "Config/Config.h"
#include "Headers/LogsException.hpp"
#include "HwBkp/MgrHwBkp.h"

std::map<uintptr_t, ExceptionInfoList>  AddressAssocExceptionList = { };

std::map<uintptr_t, BkpInfo>            BreakpointList = {};

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

//https://en.wikipedia.org/wiki/X86_debug_register

#define SET_TRAP_FLAG(ctx) ctx->EFlags |= (1 << 8);

#define UNSET_TRAP_FLAG(ctx) ctx->EFlags &= ~(1 << 8);

#define SET_RESUME_FLAG(ctx) ctx->EFlags |= 0x10000u; // RF

#define IS_TRAP_FLAG(ctx) (ctx->Dr6 & 0x4000) // check is TF

#define UNSET_HWBKP_POS( ctx, pos ) (&ctx->Dr0)[pos] = 0; \
ctx->Dr7 &= ~( 1 << pos );


#define SET_TRAP_FLAG_STEP_OUT(ctx) ctx->EFlags &= ~0x100;
//
//eflags &= ~0x100;

CRITICAL_SECTION HandlerCS = {};
long __stdcall HandlerFilter( EXCEPTION_POINTERS* pExceptionInfo )
{
	EnterCriticalSection( &HandlerCS );

	auto* const pContext			= pExceptionInfo->ContextRecord;

	auto* const pExceptionRec		= pExceptionInfo->ExceptionRecord;

	DisplayContextLogs( pContext, pExceptionRec ); // tests

	constexpr auto STATUS_WX86_SINGLE_STEP = 0x4000001E; // CE check this old flag too

	auto ExceptionAddress			= reinterpret_cast<uintptr_t>( pExceptionRec->ExceptionAddress );

	if ( EXCEPTION_SINGLE_STEP == pExceptionRec->ExceptionCode || 
		STATUS_WX86_SINGLE_STEP == pExceptionRec->ExceptionCode )
	{
		// to identify which order address was triggered
		//auto const FlagPos			= s_cast<int>( pContext->Dr6 & 0xE ); // result vals, 0 / 2 / 4 / 8

		auto IsTF = ( pContext->Dr6 & 0x4000 ); // Trap flag (BS) is defined in DR6 // BS == single step
		//ExceptionInfo->ContextRecord->EFlags |= 0x10000u; // RF Flag

		for ( const auto & [ Address, BpInfo ] : VExDebugger::GetBreakpointList( ) )
		{
			if ( BpInfo.Method != BkpMethod::Hardware )             // only support hardware breakpoint
				continue;


			if ( ( pContext->Dr6 & ( static_cast<uintptr_t>( 1 ) << BpInfo.Pos ) ) == 0 ) // check if this position was setted
				continue;

			const auto itHwBkp		= MgrHwBkp::GetHwBrkpList( ).find( Address );

			if ( itHwBkp == MgrHwBkp::GetHwBrkpList( ).end( ) )
				continue;

			const auto HwBkp		= itHwBkp->second;

			// if it's doesnt have this address, add or update info
			auto & ExceptionList	= VExDebugger::GetAssocExceptionList( )[ Address ];

			auto& Info				=  ExceptionList[ 
				( HwBkp->GetTriggerType( ) != BkpTrigger::Execute ) ? ExceptionAddress : GetCurrentThreadId( ) 
			];
		
			++Info.Details.Count;                               // inc occurrences

			Info.Details.ThreadId	= GetCurrentThreadId( );    // last thread triggered

			Info.Details.Ctx		= *pContext;                // save context
			
			//if ( Config::i( )->m_Logs )
			//	DisplayContextLogs( pContext, pExceptionRec );  // save in txt

			//pContext->Dr6			= 0xFFFF0FF0;               // reset switch

			if ( HwBkp->GetTriggerType( ) == BkpTrigger::Execute )
			{
				//SET_TRAP_FLAG( pContext );

				//SET_RESUME_FLAG( pContext );

				//SET_TRAP_FLAG_STEP_OUT( pContext );

				pContext->EFlags &= ~( 1 << 8 );

			}

			LeaveCriticalSection( &HandlerCS );

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	if ( OriginalHandlerFilter )
	{
		LeaveCriticalSection( &HandlerCS );

		return reinterpret_cast<decltype( HandlerFilter )*>( OriginalHandlerFilter )( pExceptionInfo );
	}

	//MessageBoxA( 0, "a", "af", 0 );

	LeaveCriticalSection( &HandlerCS );

	//return EXCEPTION_CONTINUE_EXECUTION;
	return EXCEPTION_EXECUTE_HANDLER;
}

bool VExDebugger::StartMonitorAddress( const uintptr_t Address, const BkpTrigger Trigger, const BkpSize Size )
{
	EnterCriticalSection( &HandlerCS );

	auto r = MgrHwBkp::SetBkpAddressInAllThreads( Address, Trigger, Size );

	LeaveCriticalSection( &HandlerCS );

	return r;
}

void VExDebugger::RemoveMonitorAddress( const uintptr_t Address )
{
	EnterCriticalSection( &HandlerCS );

	MgrHwBkp::RemoveBkpAddressInAllThreads( Address );

	LeaveCriticalSection( &HandlerCS );
}

bool VExDebugger::Init( HandlerType Type, bool SpoofHwbkp, bool Logs )
{
	if ( HandlerCS.SpinCount == 0 )
	{
		InitializeCriticalSection( &HandlerCS );
	}

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

