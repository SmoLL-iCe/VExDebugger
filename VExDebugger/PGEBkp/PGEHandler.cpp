#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "../Config/Config.h"
#include "PGEHandler.h"
#include "PGEMgr.h"
#include <algorithm>
#include "PGETracer.h"

bool IsThreadInHandling( EXCEPTION_POINTERS* pExceptionInfo )
{
	auto pContext           = pExceptionInfo->ContextRecord;

	auto pException         = pExceptionInfo->ExceptionRecord;

#ifdef USE_SWBREAKPOINT

	if ( PGETracer::ResolverMultiplesSwBrkpt( pException ) )
	{
		return true;
	}

#endif

	auto ThreadIt = PGEMgr::GetThreadHandlingList( ).find( GetCurrentThreadId( ) );

	if ( ThreadIt == PGEMgr::GetThreadHandlingList( ).end( ) )
		return false;

	auto Result             = true;

	auto& [_, Step]         = *ThreadIt;

	auto const IsSingleStep = pException->ExceptionCode == EXCEPTION_SINGLE_STEP;

	auto const IsTracing    = ( Step.Trigger.Callback != nullptr );

	auto const IsPG         = pException->ExceptionCode == EXCEPTION_GUARD_PAGE;
	
	if ( IsTracing )
	{
		auto PageBase = Step.AllocBase;

		auto PGEit    = std::find_if(

			PGEMgr::GetPageExceptionsList( ).begin( ), PGEMgr::GetPageExceptionsList( ).end( ),

			[ PageBase ]( PageGuardException& PGE )
			{
				return ( PageBase == PGE.AllocBase );
			} );

		if ( PGEit == PGEMgr::GetPageExceptionsList( ).end( ) )
			return false;

		auto Continue = PGETracer::ManagerCall( pExceptionInfo, Step, PGEit );

		if ( !Continue )
		{
			PGEMgr::GetThreadHandlingList( ).erase( ThreadIt );
		}

		return true;
	}


	if ( !IsSingleStep )
		return false;
	
	auto PageBase       = Step.AllocBase;

	auto PGEit          = std::find_if( 

		PGEMgr::GetPageExceptionsList( ).begin( ), PGEMgr::GetPageExceptionsList( ).end( ),

		[ PageBase ]( PageGuardException& PGE )
		{
			return ( PageBase == PGE.AllocBase );
		} );

	if ( PGEit == PGEMgr::GetPageExceptionsList( ).end( ) )
		return false;


	PGEMgr::GetThreadHandlingList( ).erase( ThreadIt );
		
	( *PGEit ).RestorePageGuardProtection( );

	return true;
}

long __stdcall PGEMgr::CheckPageGuardExceptions( EXCEPTION_POINTERS* pExceptionInfo )
{
	//printf( "ExceptionAddress 0x%p, PG: %d, SS: %d\n", pExceptionInfo->ExceptionRecord->ExceptionAddress,
	//	pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE,
	//	pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP );

	EnterCriticalSection( PGEMgr::GetCs( ) );

	if ( IsThreadInHandling( pExceptionInfo ) )
	{
		LeaveCriticalSection( PGEMgr::GetCs( ) );

		return EXCEPTION_CONTINUE_EXECUTION;
	}

	auto ContextRecord		= pExceptionInfo->ContextRecord;

	auto ExceptionRecord	= pExceptionInfo->ExceptionRecord;

	auto ExceptionCode		= pExceptionInfo->ExceptionRecord->ExceptionCode;

	auto ExceptionAddress	= reinterpret_cast<uintptr_t>( pExceptionInfo->ExceptionRecord->ExceptionAddress );

	if ( 
		ExceptionRecord->ExceptionCode != EXCEPTION_GUARD_PAGE && //We will catch PAGE_GUARD Violation
		ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION ) // tests
	{

		LeaveCriticalSection( PGEMgr::GetCs( ) );

		return EXCEPTION_EXECUTE_HANDLER;
	}

	if ( !ExceptionRecord->ExceptionInformation ||
		ExceptionRecord->NumberParameters != 2 ||
		!ExceptionRecord->ExceptionInformation[ 1 ] )
	{
		// it's not mine exception
		LeaveCriticalSection( PGEMgr::GetCs( ) );

		return EXCEPTION_EXECUTE_HANDLER;
		//return EXCEPTION_CONTINUE_SEARCH;
	}

	auto const		ExceptionInfoTrigger			= ExceptionRecord->ExceptionInformation[ 0 ];

	auto const		ExceptionInfoAddress			= ExceptionRecord->ExceptionInformation[ 1 ];

	auto const		ExecInstruction					= ExceptionInfoTrigger == 8;

	auto const		CurrentAddress					= ( ExecInstruction ) ? ExceptionAddress : ExceptionInfoAddress;

	auto PGEit = std::find_if( 

		PGEMgr::GetPageExceptionsList( ).begin( ), PGEMgr::GetPageExceptionsList( ).end( ),

		[ CurrentAddress ]( PageGuardException& PGE )
		{
			return PGE.InRange( CurrentAddress );
		} 
	);

	if ( PGEit == PGEMgr::GetPageExceptionsList( ).end( ) )
	{
		// IT'S NOT MINE PAGE
		LeaveCriticalSection( PGEMgr::GetCs( ) );

		return EXCEPTION_EXECUTE_HANDLER;
	}

	// yes, in one of my pages

	auto& PGE                           = ( *PGEit );

	auto TriggedIt = std::find_if( 
		
		PGE.PGTriggersList.begin( ), PGE.PGTriggersList.end( ), 

		[ CurrentAddress, ExceptionInfoTrigger, PGE ]( PageGuardTrigger& PGT )
		{ 
			return (
				CurrentAddress >= ( PGE.AllocBase + PGT.Offset ) &&
				CurrentAddress < ( ( PGE.AllocBase + PGT.Offset ) + PGT.Size ) &&
				ExceptionInfoTrigger == PGT.Type );
		} 
	);

	PageGuardTrigger SetTrigger = {};

	if ( TriggedIt != PGE.PGTriggersList.end( ) )
	{
		auto& Trigger = ( *TriggedIt );

		const auto Address = PGE.AllocBase + Trigger.Offset;

		printf( "CurrentAddress=0x%llX, Size: %lld, ExceptionInfoTrigger=%lld\n", CurrentAddress, Trigger.Size, ExceptionInfoTrigger );
		// change _IP ?

		switch ( Trigger.Type )
		{
		case PageGuardTriggerType::Execute:
		{
			//MessageBoxA( 0, "EXECUTE IsMyTriggerPoint", "8", 0 );
			break;
		}
		case PageGuardTriggerType::Read:
		{
			MessageBoxA( 0, "READ IsMyTriggerPoint", "0", 0 );
			break;
		}
		case PageGuardTriggerType::Write:
		{
			MessageBoxA( 0, "WRITE IsMyTriggerPoint", "1", 0 );
			break;
		}
		default:
			break;

		}

		SetTrigger = Trigger;
	}

	PGEMgr::GetThreadHandlingList( )[ GetCurrentThreadId( ) ] = {

		.AllocBase    = PGE.AllocBase,

		.Trigger      = SetTrigger,
	};

	if ( SetTrigger.Callback != nullptr )
	{
		PGE.RestorePageGuardProtection( );

		LeaveCriticalSection( PGEMgr::GetCs( ) );
		return EXCEPTION_CONTINUE_EXECUTION; //Continue to next instruction
	}

	// if page guard was hitted, the page guard was removed
	// set tf for next instruction call the handler again and restore the page guard again

	SET_TRAP_FLAG( ContextRecord );

	LeaveCriticalSection( PGEMgr::GetCs( ) );

	return EXCEPTION_CONTINUE_EXECUTION; //Continue to next instruction
}

