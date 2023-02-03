#include "../Headers/Header.h"
#include "MgrHwBkp.h"
#include "HwBkp.h"
#include "../Tools/WinWrap.h"
#include "../Tools/Logs.h"
#include "Threads/ManagerThreads.h"

std::map<uintptr_t, HwBkp*> AddressAdded = { };

//std::map<int, uintptr_t> AddressAssocExceptionList =
//{
//	{ 0, 0 },
//	{ 2, 0 },
//	{ 4, 0 },
//	{ 8, 0 }
//};


std::list<uint32_t> ThreadIdList = { };

void MgrHwBkp::UpdateInfo( )
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

bool MgrHwBkp::SetBkpAddressInAllThreads( const uintptr_t Address, const HwbkpType Type, const HwbkpSize Size )
{
	UpdateInfo( );

	if ( ThreadIdList.empty( ) )
		return false;

	const auto itAddress = AddressAdded.find( Address );

	if ( itAddress != AddressAdded.end( ) )
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

	// if any thread was setted
	if ( HwBkp::i( )->GetAnySuccess( ) )
	{
		VExDebugger::GetBreakpointList( )[ Address ] = {

			.Type	= BkpType::Hardware,

			.Pos	= HwBkp::i( )->GetPos( ),
		};

		AddressAdded[ Address ] = HwBkp::i( );
	}

	return HwBkp::i( )->GetAnySuccess( );
}

void MgrHwBkp::RemoveBkpAddressInAllThreads( const uintptr_t Address )
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

		auto const ItBreakpointInfo = VExDebugger::GetBreakpointList( ).find( Address );

		if ( ItBreakpointInfo != VExDebugger::GetBreakpointList( ).end( ) )
		{
			VExDebugger::GetBreakpointList( ).erase( ItBreakpointInfo );

			auto const ItException = VExDebugger::GetAssocExceptionList( ).find( Address );

			if ( ItException != VExDebugger::GetAssocExceptionList( ).end( ) )
			{
				ItException->second.clear( );

				ItException->second.swap( ItException->second );

				VExDebugger::GetAssocExceptionList( ).erase( ItException );
			}
		}

		AddressAdded.erase( itAddress );
	}
}