#include "Headers/Header.h"
#include "../import/include/VExDebugger.h"
#include "HwBkp/Threads/ManagerThreads.h"
#include "HwBkp/HwBkp.h"
#include "VEH/VEH.h"
#include "Tools/WinWrap.h"
#include "Tools/Logs.h"
#include "Config/Config.h"
#include "Headers/LogsException.hpp"

std::vector<uint32_t> ThreadIdList = { };

std::map<int, ExceptionAddressCount> ExceptionAssocAddressList =
{
	{ 0, { } },
	{ 2, { } },
	{ 4, { } },
	{ 8, { } }
};

std::map<int, uintptr_t> AddressAssocExceptionList =
{
	{ 0, 0 },
	{ 2, 0 },
	{ 4, 0 },
	{ 8, 0 }
};

std::vector<HwBkp*> AddressAdded = { };

std::map<int, ExceptionAddressCount>& VExDebugger::GetExceptionAssocAddress( )
{
	return ExceptionAssocAddressList;
}

std::map<int, uintptr_t>& VExDebugger::GetAddressAssocException( )
{
	return AddressAssocExceptionList;
}

void* p_VExDebugger = nullptr;

void* OriginalHandlerFilter = nullptr;

long __stdcall HandlerFilter( EXCEPTION_POINTERS* pExceptionInfo )
{
	auto* const pContext		= pExceptionInfo->ContextRecord;

	auto* const pExceptionRec	= pExceptionInfo->ExceptionRecord;

	//DisplayContextLogs( pContext, pExceptionRec ); // tests

	constexpr auto STATUS_WX86_SINGLE_STEP = 0x4000001E; // CE check this old flag too

	if ( EXCEPTION_SINGLE_STEP == pExceptionRec->ExceptionCode || 
		STATUS_WX86_SINGLE_STEP == pExceptionRec->ExceptionCode )
	{
		// to identify which order address was triggered
		auto const FlagPos		= s_cast<int>( pContext->Dr6 & 0xE ); // result vals, 0 / 2 / 4 / 6
		
		int e = -1;

		for ( auto & EcxpAssc : ExceptionAssocAddressList )
		{
			if ( 
				( pContext->Dr6 & ( static_cast<uintptr_t>( 1 ) << ++e ) ) != 0 && // check if this position was setted
				EcxpAssc.first == FlagPos ) // confirm if was the same pos
			{
				pContext->Dr6				= 0xFFFF0FF0; // reset switch

				// if it's doesnt have this address, add or update info
				auto& CatchInfo				= EcxpAssc.second[ pExceptionRec->ExceptionAddress ];

				++CatchInfo.Count;										// inc occurrences

				CatchInfo.ThreadId			= GetCurrentThreadId( );	// last thread triggered

				CatchInfo.Ctx				= *pContext;				// save context

				if ( Config::i( )->m_Logs )
					DisplayContextLogs( pContext, pExceptionRec ); // save in txt
				
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}
	}

	if ( OriginalHandlerFilter )
		return reinterpret_cast<decltype( HandlerFilter )*>( OriginalHandlerFilter )( pExceptionInfo );

	return EXCEPTION_EXECUTE_HANDLER;
}

void UpdateInfo( )
{
	MgrThreads::UpdateThreads( );

	const auto IsEmptyThreadList = ThreadIdList.empty( );

	for ( const auto& ThreadInfo : MgrThreads::GetThreadList( ) )
	{
		if ( ThreadInfo.first == GetCurrentThreadId( ) )
			continue;

		if ( IsEmptyThreadList )
		{
			ThreadIdList.push_back( ThreadInfo.first );

			continue;
		}

		if ( AddressAdded.empty( ) )
			return;

		auto Add = true;

		for ( auto ThreadId : ThreadIdList )
			if ( ThreadInfo.first == ThreadId )
			{
				Add = true;

				break;
			}

		if ( Add )
		{
			for ( auto* Added : AddressAdded )
				if ( WinWrap::IsValidHandle( ThreadInfo.second ) )
				{
					Added->ApplyHwbkpDebugConfig( ThreadInfo.second, ThreadInfo.first, true );
				}

			ThreadIdList.push_back( ThreadInfo.first );
		}
	}
}

bool VExDebugger::StartMonitorAddress( const uintptr_t Address, const HwbkpType Type, const HwbkpSize Size )
{
	UpdateInfo( );

	if ( ThreadIdList.empty( ) )
		return false;

	auto FailCount = 0;

	new HwBkp( Address, Size, Type );

	for ( const auto& ThreadId : ThreadIdList )
	{
		if ( ThreadId == GetCurrentThreadId( ) )
			continue;

		auto* const hThread = MgrThreads::GetThreadList( )[ ThreadId ];

		if ( WinWrap::IsValidHandle( hThread ) )
		{
			if ( !HwBkp::i( )->ApplyHwbkpDebugConfig( hThread, ThreadId ) )
				++FailCount;
		}
		else log_file( "[-] Fail open ThreadId [%u]\n", ThreadId );
	}

	for ( auto& AddressAssoc : AddressAssocExceptionList )
		if ( !AddressAssoc.second )
		{
			AddressAssoc.second = Address;

			break;
		}

	AddressAdded.push_back( HwBkp::i( ) );

	return true;
}

void VExDebugger::RemoveMonitorAddress( const uintptr_t Address )
{
	HwBkp* HwbkpRemove		= nullptr;

	auto IndexRemoveHwbkp	= -1;

	for ( auto* pHwbkp : AddressAdded )
	{
		++IndexRemoveHwbkp;

		if ( pHwbkp->GetAddress( ) == Address )
		{
			HwbkpRemove = pHwbkp;

			break;
		}
	}

	if ( HwbkpRemove )
	{
		HwbkpRemove->AddBkp( ) = false;

		for ( auto ThreadId : ThreadIdList )
		{
			auto* const hThread = MgrThreads::GetThreadList( )[ ThreadId ];

			HwbkpRemove->ApplyHwbkpDebugConfig( hThread, ThreadId );
		}

		for ( auto& AddressAssoc : AddressAssocExceptionList )
		{
			if ( AddressAssoc.second != Address )
				continue;

			AddressAssoc.second = 0;

			ExceptionAddressCount( ).swap( ExceptionAssocAddressList[ AddressAssoc.first ] );
		}

		AddressAdded.erase( AddressAdded.begin( ) + IndexRemoveHwbkp );
	}
}

void VExDebugger::PrintExceptions( )
{
	for ( const auto& EcxpAssoc : VExDebugger::GetExceptionAssocAddress( ) )
	{
		auto* const Address = r_cast<void*>( VExDebugger::GetAddressAssocException( )[ EcxpAssoc.first ] );

		log_file( "[#] => Index: %d, Address: %p", EcxpAssoc.first, Address );

		for ( const auto EcxpInfo : EcxpAssoc.second )
			log_file( "[#] === Count %d, Address: %p", EcxpInfo.second, EcxpInfo.first );

		log_file( "\n" );
	}
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
		VEH_Internal::InterceptVEHHandler( HandlerFilter, OriginalHandlerFilter );
		break;
	case HandlerType::KiUserExceptionDispatcherHook:
		VEH_Internal::HookKiUserExceptionDispatcher( HandlerFilter );
		break;
	default:
		break;
	}

	return true;
}

