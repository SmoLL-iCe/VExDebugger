#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "../Config/Config.h"
#include "PGEHandler.h"
#include "MgrPGE.h"
#include "../Headers/LogsException.hpp"
#include <algorithm>
#include "../Tools/ntos.h"
#include "../Tools/Utils.h"

#define USE_SWBREAKPOINT

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

bool PageGuardException::InRange( std::uintptr_t Address )
{
	return ( Address >= this->AllocBase && Address < ( this->AllocBase + this->AllocSize ) );
}

//bool IsMyPGTrapFlag( EXCEPTION_POINTERS* pExceptionInfo, std::map<std::uint32_t, StepBkp>::iterator ThreadStepIt )
//{
//	auto pContext   = pExceptionInfo->ContextRecord;
//	auto pException = pExceptionInfo->ExceptionRecord;
//
//	auto& [_, Step] = ( *ThreadStepIt );
//
//	auto  PageBase  = Step.AllocBase;
//
//	auto PGEit = std::find_if( MgrPGE::GetPageExceptionsList( ).begin( ), MgrPGE::GetPageExceptionsList( ).end( ),
//		[ PageBase ]( PageGuardException& PGE )
//		{
//			return ( PageBase == PGE.AllocBase );
//		} );
//
//	if ( PGEit == MgrPGE::GetPageExceptionsList( ).end( ) )
//		return false;
//
//	auto IsTracing = ( Step.Trigger.Callback != nullptr );
//
//
//	printf( "IsSingle Address: %p, IsTracing %d\n", pException->ExceptionAddress, IsTracing );
//
//	auto& PGE = ( *PGEit );
//
//	PGE.RestorePageGuardProtection( );
//
//	if ( IsTracing )
//		MgrPGE::GetThreadHandlingList( ).erase( ThreadStepIt );
//
//	return true;
//}

#ifdef USE_SWBREAKPOINT
bool PatchMem( void* dst, void* src, std::size_t size )
{
	DWORD dwOld = 0;

	SIZE_T sSize = size;
	void* Address = dst;

	auto Status = NtProtectVirtualMemory( (HANDLE)( -1 ), &Address,
		&sSize, PAGE_EXECUTE_READWRITE, &dwOld );

	if ( Status == 0 )
	{
		memcpy( dst, src, size );

		NtProtectVirtualMemory( (HANDLE)( -1 ), &Address,
			&sSize, dwOld, &dwOld );
	}
	return Status == 0;
}

template <typename T1, typename T2>
bool PatchMem( T1 dst, T2 src, std::size_t size )
{
	return PatchMem( (void*)dst, (void*)src, size );
}


template <typename T1, typename T2>
bool PatchMem( T1 dst, T2 val )
{
	return PatchMem( (void*)dst, (void*)( &val ), sizeof( val ) );
}

bool TracerCallBack( EXCEPTION_POINTERS* pExceptionInfo, StepBkp& Step, PageGuardException& PGE )
{
	PCONTEXT          pContext = pExceptionInfo->ContextRecord;

	PEXCEPTION_RECORD pException = pExceptionInfo->ExceptionRecord;

	if ( EXCEPTION_BREAKPOINT == Step.NextExceptionCode )
	{
		if ( pException->ExceptionCode == EXCEPTION_BREAKPOINT )
		{
			printf( "Restore to original byte\n" );
			PatchMem( pException->ExceptionAddress,
				Step.OriginalByte );

			Step.NextExceptionCode = 0;

			Step.AddressToHit      = 0;

			Step.OriginalByte      = 0;
		}
		else
			return true;
	}

	auto Result = Step.Trigger.Callback( pException, pContext );

	switch ( Result )
	{
	case CBReturn::StopTrace:
		{
			Step.NextExceptionCode = 0;
			return false;
		}
	case CBReturn::StepInto:
		{
			if ( PageGuardTriggerType::Execute != Step.Trigger.Type || !PGE.InRange( pContext->REG( ip ) ) )
			{
				printf( "set tf\n" );
				Step.NextExceptionCode = EXCEPTION_SINGLE_STEP;
				SET_TRAP_FLAG( pContext );
			}

			break;
		}
	case CBReturn::StepOver:
		{
		    auto CallSize = Utils::IsCallInstruction( pContext->REG( ip ) );
			{ 
				if ( CallSize == 0 )
				{
					printf( "CallSize is zero\n" );

					if ( PageGuardTriggerType::Execute != Step.Trigger.Type || !PGE.InRange( pContext->REG( ip ) ) )
					{
						printf( "set tf\n" );
						Step.NextExceptionCode = EXCEPTION_SINGLE_STEP;
						SET_TRAP_FLAG( pContext );
					}

				}
				else
				{
					printf( "CallSize=%d\n", CallSize );
					auto NextInstrutionAddr = pContext->REG( ip ) + CallSize;
					Step.NextExceptionCode = EXCEPTION_BREAKPOINT;
					Step.AddressToHit = NextInstrutionAddr;
					Step.OriginalByte = *reinterpret_cast<std::uint8_t*>( NextInstrutionAddr );
					PatchMem( NextInstrutionAddr,
						std::uint8_t(0xCC) );
				}
			}

			break;
		}
	default:
		break;
	}

	
	return true;
};
#endif

bool IsThreadInHandling( EXCEPTION_POINTERS* pExceptionInfo )
{
	auto pContext           = pExceptionInfo->ContextRecord;

	auto pException         = pExceptionInfo->ExceptionRecord;

#ifdef USE_SWBREAKPOINT

	if ( EXCEPTION_BREAKPOINT == pException->ExceptionCode )
	{
		const auto ExceptAddress  = reinterpret_cast<uintptr_t>( pException->ExceptionAddress );

		const auto CurrentID      = GetCurrentThreadId( );

		auto ThreadIt = std::find_if( MgrPGE::GetThreadHandlingList( ).begin( ), MgrPGE::GetThreadHandlingList( ).end( ),
			[ CurrentID, ExceptAddress ]( auto& ThreadIt )
			{
				auto& [ThreadId, Step] = ThreadIt;

				return ( CurrentID != ThreadId && Step.AddressToHit == ExceptAddress );
			} 
		);

		if ( MgrPGE::GetThreadHandlingList( ).end( ) != ThreadIt )
		{
			return true;
		}
	}

#endif

	auto ThreadIt = MgrPGE::GetThreadHandlingList( ).find( GetCurrentThreadId( ) );

	if ( ThreadIt == MgrPGE::GetThreadHandlingList( ).end( ) )
		return false;

	auto Result             = true;

	auto& [_, Step]         = *ThreadIt;

	auto const IsSingleStep = pException->ExceptionCode == EXCEPTION_SINGLE_STEP;

	auto const IsTracing    = ( Step.Trigger.Callback != nullptr );

	auto const IsTF         = IS_TRAP_FLAG( pContext );

	auto const IsSoftBkp    = pException->ExceptionCode == EXCEPTION_BREAKPOINT;

	auto const IsPG         = pException->ExceptionCode == EXCEPTION_GUARD_PAGE;

	//printf( "ExceptionAddress: %p, ExceptionCode: %X, IsTF: %d, IsSoftBkp: %d, IsPG: %d, Exec: %d\n", pException->ExceptionAddress, pException->ExceptionCode, IsTF, IsSoftBkp, IsPG, PageGuardTriggerType::Execute == Step.Trigger.Type );
	//if ( pException->ExceptionCode == EXCEPTION_ACCESS_VIOLATION )
	//{
	//	MessageBoxA( 0, "a", "b", 0 );
	//}
	
	if ( IsSingleStep || ( Step.NextExceptionCode == pException->ExceptionCode ) )
	{
		auto  PageBase  = Step.AllocBase;

		auto PGEit = std::find_if( MgrPGE::GetPageExceptionsList( ).begin( ), MgrPGE::GetPageExceptionsList( ).end( ),
			[ PageBase ]( PageGuardException& PGE )
			{
				return ( PageBase == PGE.AllocBase );
			} );

		if ( PGEit == MgrPGE::GetPageExceptionsList( ).end( ) )
			return false;

		auto IsTracing = ( Step.Trigger.Callback != nullptr );

		auto& PGE = ( *PGEit );

		if ( IsTracing )
		{
			auto Continue = TracerCallBack( pExceptionInfo, Step, PGE );

			if ( !Continue )
			{
				MgrPGE::GetThreadHandlingList( ).erase( ThreadIt );
			}
		}
		else
		{
			MgrPGE::GetThreadHandlingList( ).erase( ThreadIt );
		}

		PGE.RestorePageGuardProtection( );

	}else

	if ( IsPG )
	{
		SET_TRAP_FLAG( pContext );
	}

	return Result;
}


long __stdcall MgrPGE::CheckPageGuardExceptions( EXCEPTION_POINTERS* pExceptionInfo )
{
	//printf( "MgrPGE::CheckPageGuardExceptions\n" );

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

