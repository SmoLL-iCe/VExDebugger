#include "Headers/Header.h"
#include "../import/include/VExDebug.h"
#include "HwBkp/Threads/ManagerThreads.h"
#include "HwBkp/HwBkp.h"
#include "Veh/VEH.h"
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

std::map<int, ExceptionAddressCount>& VExDebug::GetExceptionAssocAddress( )
{
	return ExceptionAssocAddressList;
}

std::map<int, uintptr_t>& VExDebug::GetAddressAssocException( )
{
	return AddressAssocExceptionList;
}

void* p_VExDebug = nullptr;

void* OriginalHandlerFilter = nullptr;

long __stdcall HandlerFilter( EXCEPTION_POINTERS* pExceptionInfo )
{
	auto* const pExceptionRec	= pExceptionInfo->ExceptionRecord;

	if ( EXCEPTION_SINGLE_STEP == pExceptionRec->ExceptionCode )
	{
		auto* const pContext	= pExceptionInfo->ContextRecord;

		auto const FlagPos		= s_cast<int>( pContext->Dr6 & 0xE );

		for ( auto & EcxpAssc : ExceptionAssocAddressList )
		{
			if ( EcxpAssc.first == FlagPos )
			{
				++EcxpAssc.second[ pExceptionRec->ExceptionAddress ];

				if ( Config::i( )->m_Logs )
				{
					DisplayContextLogs( pContext, pExceptionRec );
				}

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

bool VExDebug::StartMonitorAddress( const uintptr_t Address, const HwbkpType Type, const HwbkpSize Size )
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

void VExDebug::RemoveMonitorAddress( const uintptr_t Address )
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

void VExDebug::PrintExceptions( )
{
	for ( const auto& EcxpAssoc : VExDebug::GetExceptionAssocAddress( ) )
	{
		auto* const Address = r_cast<void*>( VExDebug::GetAddressAssocException( )[ EcxpAssoc.first ] );

		log_file( "[#] => Index: %d, Address: %p", EcxpAssoc.first, Address );

		for ( const auto EcxpInfo : EcxpAssoc.second )
			log_file( "[#] === Count %d, Address: %p", EcxpInfo.second, EcxpInfo.first );

		log_file( "\n" );
	}
}

//auto base = r_cast<uint8_t*>( GetModuleHandle( nullptr ) ) + 0x1000;

//void main_thread( )
//{
//	printf( "load\n" );
//
//	VExDebug::StartMonitorAddress( r_cast<uintptr_t>( base ), HwbkpType::ReadWrite, HwbkpSize::Size_1 );
//
//	Sleep( 5000 );
//
//	for ( const auto& exp_assoc : ExceptionAssocAddressList )
//	{
//		auto* const ha_address = r_cast<void*>( AddressAssocExceptionList[ exp_assoc.first ] );
//
//		if ( !ha_address || exp_assoc.second.empty( ) )
//			continue;
//
//		printf( "== address %p, index %u\n", ha_address, exp_assoc.first );
//
//		for ( const auto exp_info : exp_assoc.second )
//			printf( "===> Exception in: %p, count: %d\n", exp_info.first, exp_info.second );
//
//		printf( "*********************************************\n" );
//	}
//}

bool VExDebug::Init( HandlerType Type, bool SpoofHwbkp, bool Logs )
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
		p_VExDebug = RtlAddVectoredExceptionHandler( 1, HandlerFilter );
		break;
	case HandlerType::UnhandledExceptionFilter:
		p_VExDebug = SetUnhandledExceptionFilter( HandlerFilter );
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

