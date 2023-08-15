#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "../Config/Config.h"
#include "PGEHandler.h"
#include "MgrPGE.h"
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

	auto ThreadIt = MgrPGE::GetThreadHandlingList( ).find( GetCurrentThreadId( ) );

	if ( ThreadIt == MgrPGE::GetThreadHandlingList( ).end( ) )
		return false;

	auto Result             = true;

	auto& [_, Step]         = *ThreadIt;

	auto const IsSingleStep = pException->ExceptionCode == EXCEPTION_SINGLE_STEP;

	auto const IsTracing    = ( Step.Trigger.Callback != nullptr );

	auto const IsPG         = pException->ExceptionCode == EXCEPTION_GUARD_PAGE;
	
	if ( IsSingleStep || ( Step.NextExceptionCode == pException->ExceptionCode ) )
	{
		auto  PageBase  = Step.AllocBase;

		auto PGEit = std::find_if( 

			MgrPGE::GetPageExceptionsList( ).begin( ), MgrPGE::GetPageExceptionsList( ).end( ),

			[ PageBase ]( PageGuardException& PGE )
			{
				return ( PageBase == PGE.AllocBase );
			} );

		if ( PGEit == MgrPGE::GetPageExceptionsList( ).end( ) )
			return false;

		if ( IsTracing )
		{
			auto Continue = PGETracer::ManagerCall( pExceptionInfo, Step, PGEit );

			if ( !Continue )
			{
				MgrPGE::GetThreadHandlingList( ).erase( ThreadIt );
			}
		}
		else
		{
			MgrPGE::GetThreadHandlingList( ).erase( ThreadIt );
		}

		( *PGEit ).RestorePageGuardProtection( );

	}else

	if ( IsPG )
	{
		SET_TRAP_FLAG( pContext );
	}

	return Result;
}


long __stdcall MgrPGE::CheckPageGuardExceptions( EXCEPTION_POINTERS* pExceptionInfo )
{
	EnterCriticalSection( MgrPGE::GetCs( ) );

	if ( IsThreadInHandling( pExceptionInfo ) )
	{
		LeaveCriticalSection( MgrPGE::GetCs( ) );
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

		LeaveCriticalSection( MgrPGE::GetCs( ) );
		return EXCEPTION_EXECUTE_HANDLER;
	}

	if ( !ExceptionRecord->ExceptionInformation ||
		ExceptionRecord->NumberParameters != 2 ||
		!ExceptionRecord->ExceptionInformation[ 1 ] )
	{
		// it's not mine exception
		LeaveCriticalSection( MgrPGE::GetCs( ) );
		return EXCEPTION_EXECUTE_HANDLER;
		//return EXCEPTION_CONTINUE_SEARCH;
	}

	auto const		ExceptionInfoTrigger			= ExceptionRecord->ExceptionInformation[ 0 ];

	auto const		ExceptionInfoAddress			= ExceptionRecord->ExceptionInformation[ 1 ];

	auto const		ExecInstruction					= ExceptionInfoTrigger == 8;

	auto const		CurrentAddress					= ( ExecInstruction ) ? ExceptionAddress : ExceptionInfoAddress;

	auto PGEit = std::find_if( MgrPGE::GetPageExceptionsList( ).begin( ), MgrPGE::GetPageExceptionsList( ).end( ),
		[ CurrentAddress ]( PageGuardException& PGE )
		{
			return PGE.InRange( CurrentAddress );
		} );

	if ( PGEit == MgrPGE::GetPageExceptionsList( ).end( ) )
	{
		// IT'S NOT MINE PAGE
		LeaveCriticalSection( MgrPGE::GetCs( ) );
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// yes, in one of my pages

	auto& PGE                           = ( *PGEit );

	auto TriggedIt = std::find_if( PGE.PGTriggersList.begin( ), PGE.PGTriggersList.end( ), 
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
			//SetInReset = false;

			MessageBoxA( 0, "EXECUTE IsMyTriggerPoint", "8", 0 );
			//Trigger.Callback( ExceptionRecord, ContextRecord );
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


	MgrPGE::GetThreadHandlingList( )[ GetCurrentThreadId( ) ] = {
		.AllocBase    = PGE.AllocBase,
		.Trigger      = SetTrigger,
	};

	// if page guard was hitted, the page guard was removed
	// set tf for next instruction call the handler again and restore the page guard again

	SET_TRAP_FLAG( ContextRecord );

	LeaveCriticalSection( MgrPGE::GetCs( ) );
	return EXCEPTION_CONTINUE_EXECUTION; //Continue to next instruction
}

