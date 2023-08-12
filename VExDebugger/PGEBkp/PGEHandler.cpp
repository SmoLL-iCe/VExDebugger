#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "../Config/Config.h"
#include "PGEHandler.h"
#include "MgrPGE.h"
#include "../Headers/LogsException.hpp"
#include <algorithm>
#include "../Tools/ntos.h"

bool PageGuardException::RestorePageGuardProtection( )
{
	DWORD dwOld = 0;

	auto Status = NtProtectVirtualMemory( (HANDLE)( -1 ),
		reinterpret_cast<void**>( &this->AllocBase ),
		reinterpret_cast<SIZE_T*>( &this->AllocSize ), this->SetProtection, &dwOld );

	auto Result = Status == 0;
	if ( !Result )
	{
		// log error
	}
	return Result;
}

bool TracerCallBack( EXCEPTION_POINTERS* pExceptionInfo, std::map<std::uint32_t, StepBkp>::iterator ThreadStepIt )
{
	auto& [_, Step] = ( *ThreadStepIt );

	PCONTEXT          pContext = pExceptionInfo->ContextRecord;

	PEXCEPTION_RECORD pException = pExceptionInfo->ExceptionRecord;

	auto const        IsTF = IS_TRAP_FLAG( pContext );

	auto const IsSingleStep   = pException->ExceptionCode == STATUS_SINGLE_STEP;
	auto const IsSoftBkp      = pException->ExceptionCode == EXCEPTION_BREAKPOINT;
	auto const IsPG           = pException->ExceptionCode == EXCEPTION_GUARD_PAGE;



	bool trapFlagSet = ( pContext->EFlags & ( 1 << 8 ) ) != 0;
	printf( "IsTF: %d, IsTF: %d, IsSoftBkp: %d, IsPG: %d, Exec: %d\n", IsTF, trapFlagSet, IsSoftBkp, IsPG, PageGuardTriggerType::Execute == Step.Trigger.Type );
	printf( "Dr6: 0x%llX, EFlags: 0x%X\n", pContext->Dr6, pContext->EFlags );


	//auto pDr = &pContext->Dr0;

	//if ( IsTF )
	//{
	//	if ( !IS_ENABLE_DR7_INDEX( pContext, BpInfo.Pos ) || ( pDr[ BpInfo.Pos ] != BpInfo.Pos ) )
	//	{
	//		return false; //it's not mine
	//	}
	//}

	auto Result = Step.Trigger.Callback( pException, pContext );

	switch ( Result )
	{
	case CBReturn::StopTrace:
		{

			SET_RESUME_FLAG( pContext );

			break;
		}
	case CBReturn::StepInto:
		{
			//if ( !IsTF )
			//{
			//	//int index = GET_HW_TRIGGERED_INDEX( pContext );

			//	//pDr[ index ] = BpInfo.Pos; // change address to index
			//}

			//if ( Step.IsExecTrigger )
			//{

			//}

			if ( PageGuardTriggerType::Execute == Step.Trigger.Type )
			{
				//MessageBoxA( 0, "restore", "cap", 0 );

				//if ( IsPG )
				//{
				//	auto PageBase = Step.AllocBase;

				//	auto PGEit = std::find_if( MgrPGE::GetPageExceptionsList( ).begin( ), MgrPGE::GetPageExceptionsList( ).end( ),
				//		[ PageBase ]( PageGuardException& PGE )
				//		{
				//			return ( PageBase == PGE.AllocBase );
				//		} );

				//	if ( PGEit == MgrPGE::GetPageExceptionsList( ).end( ) )
				//		return false;

				//	auto& PGE = ( *PGEit );

				//	PGE.RestorePageGuardProtection( );
				//}else

					SET_TRAP_FLAG( pContext );
			}


			//

			break;
		}
	case CBReturn::StepOver:
		{
			//auto CallSize = Utils::IsCallInstruction(  pContext->REG( ip ) );

			//if ( !IsTF )
			//{
			//	int index = GET_HW_TRIGGERED_INDEX( pContext );

			//	pDr[ index ] = BpInfo.Pos; // change address to index
			//}

			//if ( CallSize )
			//{
			//	pDr[ BpInfo.Pos ] = pContext->REG( ip ) + CallSize;

			//	SET_DR7_INDEX_TYPE( pContext, BpInfo.Pos, 0 ); // Exec

			//	SET_DR7_INDEX_SIZE( pContext, BpInfo.Pos, 0 );

			//}else
			//SET_TRAP_FLAG( pContext );

			break;
		}
	default:
		break;
	}
	
	return true;
};

//bool IsMyPGTrapFlag( PEXCEPTION_RECORD ExceptionRecord )
//{
//	if ( ExceptionRecord->ExceptionCode != STATUS_SINGLE_STEP ) //We will also catch STATUS_SINGLE_STEP, meaning we just had a PAGE_GUARD violation
//		return false;
//
//	//bool hasAnyReset = false;
//
//	for ( auto& PGE : MgrPGE::GetPageExceptionsList( ) )
//	{
//		auto TIDit = std::find( PGE.Threads.begin( ), PGE.Threads.end( ), GetCurrentThreadId() );
//
//		if ( TIDit != PGE.Threads.end( ) ) // it's a way to know that this thread is the right thread and not a bait thread with tf
//		{
//			//hasAnyReset = true;
//
//			PGE.RestorePageGuardProtection( );
//
//			PGE.Threads.erase( TIDit );
//
//			//PGE.ReSet = false;
//			return true;
//		}
//	}
//
//	return false;
//}


bool IsMyPGTrapFlag( EXCEPTION_POINTERS* pExceptionInfo, std::map<std::uint32_t, StepBkp>::iterator ThreadStepIt )
{
	auto pContext   = pExceptionInfo->ContextRecord;
	auto pException = pExceptionInfo->ExceptionRecord;

	auto& [_, Step] = ( *ThreadStepIt );

	auto  PageBase  = Step.AllocBase;

	auto PGEit = std::find_if( MgrPGE::GetPageExceptionsList( ).begin( ), MgrPGE::GetPageExceptionsList( ).end( ),
		[ PageBase ]( PageGuardException& PGE )
		{
			return ( PageBase == PGE.AllocBase );
		} );

	if ( PGEit == MgrPGE::GetPageExceptionsList( ).end( ) )
		return false;


	printf( "IsSingle Address: %p\n", pException->ExceptionAddress );

	auto& PGE = ( *PGEit );

	PGE.RestorePageGuardProtection( );

	if ( !Step.Trigger.Callback )
		MgrPGE::GetThreadHandlingList( ).erase( ThreadStepIt );

	return true;
}

bool IsThreadInHandling( EXCEPTION_POINTERS* pExceptionInfo )
{
	auto ThreadIt = MgrPGE::GetThreadHandlingList( ).find( GetCurrentThreadId( ) );

	if ( ThreadIt == MgrPGE::GetThreadHandlingList( ).end( ) )
		return false;


	auto pContext   = pExceptionInfo->ContextRecord;
	auto pException = pExceptionInfo->ExceptionRecord;

	auto& [_, Step] = *ThreadIt;

	auto const IsSingleStep = pException->ExceptionCode == STATUS_SINGLE_STEP;

	auto const IsPG = pException->ExceptionCode == EXCEPTION_GUARD_PAGE;

	if ( IsSingleStep )
	{
		return IsMyPGTrapFlag( pExceptionInfo, ThreadIt );
	}
	

	if ( !IsPG || !Step.Trigger.Callback )
		return false;

	return TracerCallBack( pExceptionInfo, ThreadIt );
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
			return ( CurrentAddress >= PGE.AllocBase && CurrentAddress <= ( PGE.AllocBase + PGE.AllocSize ) );
		} );

	if ( PGEit == MgrPGE::GetPageExceptionsList( ).end( ) )
	{
		// IT'S NOT MINE PAGE
		LeaveCriticalSection( MgrPGE::GetCs( ) );
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// yes, in one of my pages

	auto& PGE                           = ( *PGEit );

	//PGE.ReSet							= true;

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
			Trigger.Callback( ExceptionRecord, ContextRecord );
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
		.AllocBase = PGE.AllocBase,
		.Trigger = SetTrigger,
	};
	

	//bool SetInReset = true;




	//if ( SetInReset )



	//PGE.Threads.push_back( GetCurrentThreadId( ) );

	// if page guard was hitted, the page guard was removed
	// set tf for next instruction call the handler again and restore the page guard again

	SET_TRAP_FLAG( ContextRecord );

	LeaveCriticalSection( MgrPGE::GetCs( ) );
	return EXCEPTION_CONTINUE_EXECUTION; //Continue to next instruction
}

