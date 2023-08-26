#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "PGEMgr.h"
#include "../Tools/WinWrap.h"
#include "../Tools/Logs.h"
#include "PGE.hpp"
//
//bool                    isPGECsInitialized = false;
//
//CRITICAL_SECTION        PGEHandlerCS = { };
//
//void InitCS( )
//{
//	if ( !isPGECsInitialized )
//	{
//		InitializeCriticalSection( &PGEHandlerCS );
//
//		isPGECsInitialized = true;
//	}
//}

bool PageGuardException::RestorePageGuardProtection( ) const
{
	DWORD dwOld = 0;

	auto b = WinWrap::ProtectMemory(
		reinterpret_cast<void*>( this->AllocBase ), this->AllocSize, this->SetProtection, &dwOld );

	if ( !b )
	{
		log_file( "[-] Failed protect status 0x%X, address 0x%p, size 0x%p in %s\n", WinWrap::GetErrorStatus( ),
			reinterpret_cast<void*>( this->AllocBase ),
			reinterpret_cast<void*>( this->AllocSize ),
			__FUNCTION__ );
	}
	return b;
}

bool PageGuardException::InRange( std::uintptr_t Address ) const
{
	return ( Address >= this->AllocBase && Address < ( this->AllocBase + this->AllocSize ) );
}

std::vector<PageGuardException> PGExceptionsList{};

std::vector<PageGuardException>& PGEMgr::GetPageExceptionsList( )
{
	return PGExceptionsList;
}

std::map<std::uint32_t, StepBkp> ThreadsSteps{};

std::map<std::uint32_t, StepBkp>& PGEMgr::GetThreadHandlingList( )
{
	return ThreadsSteps;
}
//
//CRITICAL_SECTION* PGEMgr::GetCs( )
//{
//	return &PGEHandlerCS;
//}

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

bool PGEMgr::AddPageExceptions( uintptr_t Address, BkpTrigger TriggerType, BkpSize bSize, TCallback Callback )
{
	//InitCS( );

	//TCallback** fnPointer = Callback.target<TCallback*>( );

	//printf( "Callback=%p, %p\n", fnPointer, (uintptr_t**)( &Callback ) );

	//getchar( );

	//EnterCriticalSection( PGEMgr::GetCs( ) );

	if ( BkpTrigger::Execute == TriggerType && Callback == nullptr )
		bSize = BkpSize::Size_1;

	auto Size = ConvertToSize( bSize );

	auto PGEit = std::find_if( 
		
		PGExceptionsList.begin( ), PGExceptionsList.end( ),

		[ Address ]( PageGuardException& PGE ) {

			return  PGE.InRange( Address );
		} );

	MEMORY_BASIC_INFORMATION	mbi		= {};

	SIZE_T						rSize	= 0;

	auto Result  = WinWrap::QueryMemory( reinterpret_cast<void*>( Address ), MemoryBasicInformation, &mbi, sizeof( mbi ), &rSize );

	if ( !Result )
	{
		log_file( "[-] Failed query status 0x%X, address 0x%p in %s\n", WinWrap::GetErrorStatus( ),
			reinterpret_cast<void*>( Address ),
			__FUNCTION__ );

		//LeaveCriticalSection( PGEMgr::GetCs( ) );
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
			//LeaveCriticalSection( PGEMgr::GetCs( ) );
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

		.Method		            = BkpMethod::PageExceptions,

		.Trigger	            = TriggerType,

		.Size		            = bSize,

		.Pos		            = -1,

		.Callback               = Callback,
	};

	DWORD dwOld = 0;

	Result = WinWrap::ProtectMemory( mbi.BaseAddress, mbi.RegionSize, SetProtection, &dwOld );

	if ( !Result )
	{
		log_file( "[-] Failed protect status 0x%X, address 0x%p, size 0x%lX in %s\n", WinWrap::GetErrorStatus( ),
			mbi.BaseAddress,
			*reinterpret_cast<uint32_t*>( &mbi.RegionSize ),
			__FUNCTION__ );
	}

	//LeaveCriticalSection( PGEMgr::GetCs( ) );

	return Result;
}

bool PGEMgr::RemovePageExceptions( uintptr_t Address, BkpTrigger TriggerType )
{
	//InitCS( );

	//EnterCriticalSection( PGEMgr::GetCs( ) );

	auto BkpIt = std::find_if( 

		VExInternal::GetBreakpointList( ).begin( ), 

		VExInternal::GetBreakpointList( ).end( ),

		[ Address ]( auto& Pair ) {

			return ( Address == Pair.first );
		}
	);

	if ( BkpIt == VExInternal::GetBreakpointList( ).end( ) )
	{
		//LeaveCriticalSection( PGEMgr::GetCs( ) );
		return false;
	}

	auto PGEit = std::find_if( PGExceptionsList.begin( ), PGExceptionsList.end( ), 

		[ Address ]( PageGuardException& PGE ) {

			return PGE.InRange( Address );
		} );

	if ( PGEit == PGExceptionsList.end( ) )
	{
		//LeaveCriticalSection( PGEMgr::GetCs( ) );
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
		//LeaveCriticalSection( PGEMgr::GetCs( ) );
		return false;
	}

	if ( Info.PGTriggersList.size( ) > 1 )
	{
		Info.PGTriggersList.erase( PGTit );
	}
	else
	{
		DWORD dwOld   = 0;

		auto pAddress = reinterpret_cast<void*>( Info.AllocBase );

		auto Result   = WinWrap::ProtectMemory( pAddress, Info.AllocSize, Info.OldProtection, &dwOld );

		if ( !Result )
		{
			log_file( "[-] Failed protect status 0x%X, address 0x%p, size 0x%lX in %s\n", WinWrap::GetErrorStatus( ),
				pAddress,
				*reinterpret_cast<uint32_t*>( &Info.AllocSize ),
				__FUNCTION__ );
		}

		Info.PGTriggersList.clear( );

		PGExceptionsList.erase( PGEit );
	}

	VExInternal::GetBreakpointList( ).erase( BkpIt );

	//LeaveCriticalSection( PGEMgr::GetCs( ) );

	return true;
}

