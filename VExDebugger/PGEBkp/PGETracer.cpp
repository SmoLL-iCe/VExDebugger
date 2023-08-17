#include "../Headers/Header.h"
#include "PGETracer.h"
#include "PGEMgr.h"
#include <algorithm>
#include "../Tools/ntos.h"
#include "../Tools/Utils.h"
#include "../Headers/VExInternal.h"
#include "../Tools/Logs.h"
#include "../Tools/WinWrap.h"

#ifdef USE_SWBREAKPOINT
bool PatchMem( void* dst, void* src, std::size_t size )
{
	DWORD dwOld = 0;

	SIZE_T sSize = size;

	void* Address = dst;

	auto Result = WinWrap::ProtectMemory( Address, sSize, PAGE_EXECUTE_READWRITE, &dwOld );

	if ( Result )
	{
		memcpy( dst, src, size );

		WinWrap::ProtectMemory( Address, sSize, dwOld, &dwOld );
	}

	return Result;
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

bool PGETracer::ResolverMultiplesSwBrkpt( EXCEPTION_RECORD* pException )
{
	if ( EXCEPTION_BREAKPOINT == pException->ExceptionCode )
	{
		const auto ExceptAddress  = reinterpret_cast<uintptr_t>( pException->ExceptionAddress );

		const auto CurrentID      = GetCurrentThreadId( );

		auto ThreadIt             = std::find_if( 

			PGEMgr::GetThreadHandlingList( ).begin( ), PGEMgr::GetThreadHandlingList( ).end( ),

			[ CurrentID, ExceptAddress ]( auto& ThreadIt )
			{
				auto& [ThreadId, Step] = ThreadIt;

				return ( CurrentID != ThreadId && Step.AddressToHit == ExceptAddress );
			} 
		);

		if ( PGEMgr::GetThreadHandlingList( ).end( ) != ThreadIt )
		{
			return true;
		}
	}

	return false;
}

#endif

bool PGETracer::ManagerCall2( EXCEPTION_POINTERS* pExceptionInfo, StepBkp& Step, std::vector<PageGuardException>::iterator PGEit )
{
	if ( Step.NextExceptionCode == STILL_ACTIVE )
	{
		printf( "Skip ExceptionAddress 0x%p, PG: %d, SS: %d\n", pExceptionInfo->ExceptionRecord->ExceptionAddress,
			pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE,
			pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP );
		Step.NextExceptionCode = 0;
		return true;
	}

	auto&             PGE              = ( *PGEit );

	PCONTEXT          pContext         = pExceptionInfo->ContextRecord;

	PEXCEPTION_RECORD pException       = pExceptionInfo->ExceptionRecord;

	const auto        CurrentAddress   = pContext->REG( ip );

	const auto        IsRange          = PGE.InRange( CurrentAddress );

	if ( IsRange )
		return true;
	
	auto FoundPGEit = std::find_if( //check if this point is already included any existing page

		PGEMgr::GetPageExceptionsList( ).begin( ),

		PGEMgr::GetPageExceptionsList( ).end( ),

		[ CurrentAddress ]( PageGuardException& PGE )
		{
			return PGE.InRange( CurrentAddress );
		} );

	if ( FoundPGEit != PGEMgr::GetPageExceptionsList( ).end( ) )
	{
		Step.AllocBase = ( *FoundPGEit ).AllocBase; // update to existing page base

		return true;
	}


	// go to add this page
	MEMORY_BASIC_INFORMATION	mbi		= {};

	SIZE_T						rSize	= 0;
				
	if ( !WinWrap::QueryMemory( reinterpret_cast<void*>( CurrentAddress ), MemoryBasicInformation, &mbi, sizeof( mbi ), &rSize ) )
	{
		log_file( "[-] Failed query status 0x%X, address 0x%p in %s\n", WinWrap::GetErrorStatus( ),
			reinterpret_cast<void*>( CurrentAddress ),
			__FUNCTION__ );

		return false;
	}

	DWORD SetProtection     = mbi.Protect;

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

	if ( PGE.PGTriggersList.empty( ) ) // if is not empty, it's mean that is the initial page breakpoint
	{
		PGEMgr::GetPageExceptionsList( ).erase( PGEit );
	}

	PGEMgr::GetPageExceptionsList( ).push_back( PageInfo ); // add new page for going on tracing

	DWORD dwOld             = 0;

	if ( !WinWrap::ProtectMemory( mbi.BaseAddress, mbi.RegionSize, SetProtection, &dwOld ) )
	{
		log_file( "[-] Failed protect status 0x%X, address 0x%p, size 0x%lX in %s\n", WinWrap::GetErrorStatus( ),
			mbi.BaseAddress, 
			*reinterpret_cast<uint32_t*>( &mbi.RegionSize ), 
			__FUNCTION__ );
		return false;
	}

	Step.AllocBase          = PageInfo.AllocBase; // update new page base	

	return true;
}

bool PGETracer::ManagerCall( EXCEPTION_POINTERS* pExceptionInfo, StepBkp& Step, std::vector<PageGuardException>::iterator PGEit )
{
	auto&             PGE         = ( *PGEit );

	PCONTEXT          pContext    = pExceptionInfo->ContextRecord;

	PEXCEPTION_RECORD pException  = pExceptionInfo->ExceptionRecord;

#ifdef USE_SWBREAKPOINT

	if ( EXCEPTION_BREAKPOINT == Step.NextExceptionCode )
	{
		if ( pException->ExceptionCode == EXCEPTION_BREAKPOINT )
		{
			PatchMem( pException->ExceptionAddress,
				Step.OriginalByte );

			Step.NextExceptionCode  = 0;

			Step.AddressToHit       = 0;

			Step.OriginalByte       = 0;
		}
		else
			return true;
	}

#else



#endif

	auto Result                   = Step.Trigger.Callback( pException, pContext );

	auto CurrentAddress           = pContext->REG( ip );

	auto IsRange                  = PGE.InRange( CurrentAddress );

	switch ( Result )
	{
	case CBReturn::StopTrace:
		{
			Step.NextExceptionCode = 0;

#ifndef USE_SWBREAKPOINT

			if ( PGE.PGTriggersList.empty( ) ) // if is not empty, it's mean that is the initial page breakpoint
			{
				PGEMgr::GetPageExceptionsList( ).erase( PGEit );
			}
#endif

			return false;
		}
	case CBReturn::StepInto:
		{
			if ( !IsRange )
			{
				//Step.NextExceptionCode = EXCEPTION_SINGLE_STEP;
				//SET_TRAP_FLAG( pContext );
			}

			break;
		}
	case CBReturn::StepOver:
		{
		    auto CallSize = Utils::IsCallInstruction( pContext->REG( ip ) );
			
#ifdef USE_SWBREAKPOINT

			if ( CallSize == 0 )
			{
				if ( PageGuardTriggerType::Execute != Step.Trigger.Type || !PGE.InRange( pContext->REG( ip ) ) )
				{
					Step.NextExceptionCode = EXCEPTION_SINGLE_STEP;
					//SET_TRAP_FLAG( pContext );
				}

			}
			else
			{
				auto NextInstrutionAddr = CurrentAddress + CallSize;

				Step.NextExceptionCode  = EXCEPTION_BREAKPOINT;

				Step.AddressToHit       = NextInstrutionAddr;

				Step.OriginalByte       = *reinterpret_cast<std::uint8_t*>( NextInstrutionAddr );

				PatchMem( NextInstrutionAddr, std::uint8_t( 0xCC ) );
			}
#else

			if ( CallSize > 0 )
			{
				// to skip call
				Step.NextExceptionCode = STILL_ACTIVE;
			}

#endif
			break;
		}
	default:
		break;
	}

	return true;
};