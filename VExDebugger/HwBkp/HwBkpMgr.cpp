#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "HwBkpMgr.h"
#include "HwBkp.h"
#include "../Tools/WinWrap.h"
#include "../Tools/Logs.h"
#include "Threads/ManagerThreads.h"

std::map<uintptr_t, HwBkp*> AddressAdded = { };

std::list<uint32_t> ThreadIdList = { };

std::map<uintptr_t, HwBkp*>& HwBkpMgr::GetHwBrkpList( )
{
	return AddressAdded;
}

void HwBkpMgr::UpdateInfo( )
{
	MgrThreads::UpdateThreads( );

	const auto IsEmptyThreadList = ThreadIdList.empty( );

	for ( const auto& [ ThreadId, hThread ] : MgrThreads::GetThreadList( ) )
	{
		if ( ThreadId == GetCurrentThreadId( ) )
			continue;

		if ( IsEmptyThreadList )
		{
			ThreadIdList.push_back( ThreadId );

			continue;
		}

		if ( AddressAdded.empty( ) )
			return;

		// check if is new thread
		auto it  = std::find( ThreadIdList.begin( ), ThreadIdList.end( ), ThreadId );

		if ( it == ThreadIdList.end( ) )
		{ //if a new thread was created it added all hwbkp breakpoints

			for ( const auto& [ Address, Hwbkp ] : AddressAdded )
				if ( WinWrap::IsValidHandle( hThread ) )
				{
					Hwbkp->ApplyHwbkpDebugConfig( hThread, ThreadId, true );
				}

			ThreadIdList.push_back( ThreadId );
		}
	}
}

bool HwBkpMgr::SetBkpAddressInAllThreads( const uintptr_t Address, const BkpTrigger Trigger, const BkpSize Size, TCallback Callback )
{
	UpdateInfo( );

	if ( ThreadIdList.empty( ) )
		return false;

	for ( auto& [ AddedAdr, AddedHwbkp ] : AddressAdded )
	{
		std::int32_t CurrentSize = 0;

		CurrentSize = ( BkpSize::Size_8 == AddedHwbkp->GetSize( ) ) ? 7 : ( static_cast<int>( AddedHwbkp->GetSize( ) ) ) ;
	
		if ( Address >= AddedAdr && Address <= ( AddedAdr + CurrentSize ) ) // check is the same range
		{
			return false;
		}
	}

	auto FailCount = 0;

	new HwBkp( Address, Size, Trigger );

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

	// if any thread was setted
	if ( HwBkp::i( )->GetAnySuccess( ) )
	{
		VExInternal::GetBreakpointList( )[ Address ] = {

			.Method		= BkpMethod::Hardware,

			.Trigger	= HwBkp::i( )->GetTriggerType( ),

			.Size		= HwBkp::i( )->GetSize( ),

			.Pos		= HwBkp::i( )->GetPos( ),

			.Callback   = Callback,
		};

		AddressAdded[ Address ] = HwBkp::i( );
	}

	return HwBkp::i( )->GetAnySuccess( );
}

void HwBkpMgr::RemoveBkpAddressInAllThreads( const uintptr_t Address )
{
	const auto itAddress			= AddressAdded.find( Address );

	if ( itAddress == AddressAdded.end( ) )
		return;

	HwBkp* Hwbkp = itAddress->second;

	if ( Hwbkp )
	{
		Hwbkp->SetRemove( );

		for ( auto ThreadId : ThreadIdList )
		{
			auto* const hThread = MgrThreads::GetThreadList( )[ ThreadId ];

			Hwbkp->ApplyHwbkpDebugConfig( hThread, ThreadId );
		}

		auto const ItBreakpointInfo = VExInternal::GetBreakpointList( ).find( Address );

		if ( ItBreakpointInfo != VExInternal::GetBreakpointList( ).end( ) )
		{
			VExInternal::GetBreakpointList( ).erase( ItBreakpointInfo );

			auto const ItException = VExInternal::GetAssocExceptionList( ).find( Address );

			if ( ItException != VExInternal::GetAssocExceptionList( ).end( ) )
			{
				ItException->second.clear( );

				ItException->second.swap( ItException->second );

				VExInternal::GetAssocExceptionList( ).erase( ItException );
			}
		}

		AddressAdded.erase( itAddress );
	}
}