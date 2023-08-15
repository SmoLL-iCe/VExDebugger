#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "MgrPGE.h"
#include "../Tools/WinWrap.h"
#include "../Tools/Logs.h"
#include "PGE.hpp"

bool                    isPGECsInitialized = false;

CRITICAL_SECTION        PGEHandlerCS = { };

void InitCS( )
{
	if ( !isPGECsInitialized )
	{
		InitializeCriticalSection( &PGEHandlerCS );

		isPGECsInitialized = true;
	}
}

std::vector<PageGuardException> PGExceptionsList{};

std::vector<PageGuardException>& MgrPGE::GetPageExceptionsList( )
{
	return PGExceptionsList;
}
//std::vector<uint32_t> Threads = {};
std::map<std::uint32_t, StepBkp> ThreadsSteps{};

std::map<std::uint32_t, StepBkp>& MgrPGE::GetThreadHandlingList( )
{
	return ThreadsSteps;
}

CRITICAL_SECTION* MgrPGE::GetCs( )
{
	return &PGEHandlerCS;
}

PageGuardTriggerType ConvertToPGTrigger( BkpTrigger Trigger )
{
	switch ( Trigger )
	{
	case BkpTrigger::Execute:
		return PageGuardTriggerType::Execute;
	case BkpTrigger::ReadWrite:
		return PageGuardTriggerType::ReadWrite;
	case BkpTrigger::Write:
		return PageGuardTriggerType::Write;
	default:
		break;
	}
	return static_cast<PageGuardTriggerType>( -1 );
}

size_t ConvertToSize( BkpSize s )
{
	switch ( s )
	{
	case BkpSize::Size_1:
		return 1;
	case BkpSize::Size_2:
		return 2;
	case BkpSize::Size_8:
		return 8;
	case BkpSize::Size_4:
		return 4;
	default:
		break;
	}
	return static_cast<size_t>( s );
}

bool MgrPGE::AddPageExceptions( uintptr_t Address, BkpTrigger TriggerType, BkpSize bSize, TCallback Callback )
{
	InitCS( );

	EnterCriticalSection( MgrPGE::GetCs( ) );

	if ( BkpTrigger::Execute == TriggerType )
		bSize = BkpSize::Size_1;

	auto Size = ConvertToSize( bSize );

	auto PGEit = std::find_if( 
		
		PGExceptionsList.begin( ), PGExceptionsList.end( ),

		[ Address ]( PageGuardException& PGE ) {

			return  PGE.InRange( Address );
		} );

	MEMORY_BASIC_INFORMATION	mbi		= {};
	SIZE_T						rSize	= 0;
	auto Status = NtQueryVirtualMemory( (HANDLE)-1, reinterpret_cast<void*>( Address ), MemoryBasicInformation, &mbi, sizeof( mbi ), &rSize );

	if ( Status != 0 )
	{
		// log error
		LeaveCriticalSection( MgrPGE::GetCs( ) );
		return false;
	}

	DWORD SetProtection = 0;

	if ( PGEit == PGExceptionsList.end( ) )
	{
		if ( Size > mbi.RegionSize )
			Size = mbi.RegionSize; // for while it supports one page

		SetProtection			= mbi.Protect;

		if ( ( mbi.Protect & PAGE_GUARD ) == 0 )
		{
			SetProtection		= mbi.Protect | PAGE_GUARD;
		}

		printf( "BaseAddress: 0x%p, RegionSize: 0x%llX\n", mbi.BaseAddress, mbi.RegionSize );

		PageGuardException PageInfo  = {

			.AllocBase			= reinterpret_cast<uintptr_t>( mbi.BaseAddress ),

			.AllocSize			= mbi.RegionSize,

			.OldProtection		= mbi.Protect,

			.SetProtection		= SetProtection,
		};

		PageInfo.PGTriggersList.push_back( {

			.Type				= ConvertToPGTrigger( TriggerType ),

			.Offset				= Address - PageInfo.AllocBase,

			.Size				= Size,

			.Callback           = Callback,
		} );

		PGExceptionsList.push_back( PageInfo );
	}
	else
	{
		auto& Info              = ( *PGEit );

		auto PGTit              = std::find_if(

			Info.PGTriggersList.begin( ),

			Info.PGTriggersList.end( ),

			[ Address, TriggerType, Info ]( PageGuardTrigger& Trigger )
			{
				return ( Address >= ( Info.AllocBase + Trigger.Offset ) &&
					Address <= ( ( Info.AllocBase + Trigger.Offset ) + Trigger.Size ) && ConvertToPGTrigger( TriggerType ) == Trigger.Type );
			}
		);

		if ( PGTit != Info.PGTriggersList.end( ) )
		{
			LeaveCriticalSection( MgrPGE::GetCs( ) );
			return false;
		}

		SetProtection			= Info.SetProtection;

		if ( ( mbi.Protect & PAGE_GUARD ) == 0 )
		{
			SetProtection		= mbi.Protect | PAGE_GUARD;
		}

		Info.PGTriggersList.push_back( {

			.Type				= ConvertToPGTrigger( TriggerType ),

			.Offset				= Address - Info.AllocBase,

			.Size				= Size,

			.Callback           = Callback,
		} );

		Info.SetProtection      = SetProtection;
	}

	VExInternal::GetBreakpointList( )[ Address ] = {

		.Method		= BkpMethod::PageExceptions,

		.Trigger	= TriggerType,

		.Size		= bSize,

		.Pos		= -1,

		//.Callback   = Callback,
	};

	DWORD dwOld = 0;
	Status = NtProtectVirtualMemory( (HANDLE)-1, &mbi.BaseAddress, &mbi.RegionSize, SetProtection, &dwOld );

	if ( Status != 0 )
	{
		// log error
	}

	LeaveCriticalSection( MgrPGE::GetCs( ) );

	return Status == 0;
}

bool MgrPGE::RemovePageExceptions( uintptr_t Address, BkpTrigger TriggerType )
{
	InitCS( );

	EnterCriticalSection( MgrPGE::GetCs( ) );

	auto BkpIt = std::find_if( 

		VExInternal::GetBreakpointList( ).begin( ), 

		VExInternal::GetBreakpointList( ).end( ),

		[ Address ]( auto& Pair ) {

			return ( Address == Pair.first );
		}
	);

	if ( BkpIt == VExInternal::GetBreakpointList( ).end( ) )
	{
		LeaveCriticalSection( MgrPGE::GetCs( ) );
		return false;
	}

	auto PGEit = std::find_if( PGExceptionsList.begin( ), PGExceptionsList.end( ), 

		[ Address ]( PageGuardException& PGE ) {

			return PGE.InRange( Address );
		} );

	if ( PGEit == PGExceptionsList.end( ) )
	{
		LeaveCriticalSection( MgrPGE::GetCs( ) );
		return false;
	}

	auto& Info              = ( *PGEit );

	auto PGTit              = std::find_if(

		Info.PGTriggersList.begin( ),

		Info.PGTriggersList.end( ),

		[ Address, TriggerType, Info ]( PageGuardTrigger& Trigger )
		{
			return ( Address == ( Info.AllocBase + Trigger.Offset ) && ConvertToPGTrigger( TriggerType ) == Trigger.Type );
		}
	);

	if ( PGTit == Info.PGTriggersList.end( ) )
	{
		LeaveCriticalSection( MgrPGE::GetCs( ) );
		return false;
	}

	if ( Info.PGTriggersList.size( ) > 1 )
	{
		Info.PGTriggersList.erase( PGTit );
	}
	else
	{
		printf( "Remover todos e essa pagina\n" );
		DWORD dwOld   = 0;

		auto pAddress = reinterpret_cast<void*>( Info.AllocBase );

		auto Status   = NtProtectVirtualMemory( (HANDLE)-1, &pAddress, &Info.AllocSize, Info.OldProtection, &dwOld );

		if ( Status == 0 )
		{
			// log error
		}

		Info.PGTriggersList.clear( );

		PGExceptionsList.erase( PGEit );
	}

	VExInternal::GetBreakpointList( ).erase( BkpIt );

	LeaveCriticalSection( MgrPGE::GetCs( ) );

	return true;
}

