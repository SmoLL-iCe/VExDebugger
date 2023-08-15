#include "../Headers/Header.h"
#include "PGETracer.h"
#include "MgrPGE.h"
#include <algorithm>
#include "../Tools/ntos.h"
#include "../Tools/Utils.h"
#include "../Headers/VExInternal.h"

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

bool PGETracer::ResolverMultiplesSwBrkpt( EXCEPTION_RECORD* pException )
{
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

	return false;
}

#endif

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

			Step.NextExceptionCode = 0;

			Step.AddressToHit = 0;

			Step.OriginalByte = 0;
		}
		else
			return true;
	}
#else
	if ( Step.NextExceptionCode == STILL_ACTIVE )
	{
		Step.NextExceptionCode = 0;
		return true;
	}
#endif

	auto Result                   = Step.Trigger.Callback( pException, pContext );

	auto CurrentAddress           = pContext->REG( ip );

	auto IsRange                  = PGE.InRange( CurrentAddress );

	if ( !IsRange )
	{
		auto FoundPGEit = std::find_if( //check if this point is already included any existing page

			MgrPGE::GetPageExceptionsList( ).begin( ),

			MgrPGE::GetPageExceptionsList( ).end( ),

			[ CurrentAddress ]( PageGuardException& PGE )
			{
				return PGE.InRange( CurrentAddress );
			} );

		if ( FoundPGEit != MgrPGE::GetPageExceptionsList( ).end( ) )
		{
			IsRange        = true;

			Step.AllocBase = ( *FoundPGEit ).AllocBase; // update to existing page base
		}
	}

	switch ( Result )
	{
	case CBReturn::StopTrace:
		{
			Step.NextExceptionCode = 0;
			return false;
		}
	case CBReturn::StepInto:
		{
			if ( !IsRange )
			{
				Step.NextExceptionCode = EXCEPTION_SINGLE_STEP;
				SET_TRAP_FLAG( pContext );
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
					SET_TRAP_FLAG( pContext );
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

			if ( !IsRange )
			{
				// go to add this page
				MEMORY_BASIC_INFORMATION	mbi		= {};

				SIZE_T						rSize	= 0;

				auto Status = NtQueryVirtualMemory( (HANDLE)-1, reinterpret_cast<void*>( CurrentAddress ), MemoryBasicInformation, &mbi, sizeof( mbi ), &rSize );

				if ( Status != 0 )
				{
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
					MgrPGE::GetPageExceptionsList( ).erase( PGEit );
				}

				MgrPGE::GetPageExceptionsList( ).push_back( PageInfo ); // add new page for going on tracing

				DWORD dwOld             = 0;

				Status                  = NtProtectVirtualMemory( (HANDLE)-1, &mbi.BaseAddress, &mbi.RegionSize, SetProtection, &dwOld );

				if ( Status != 0 )
				{
					// log error
					return false;
				}

				Step.AllocBase          = PageInfo.AllocBase; // update new page base
			}
#endif
			break;
		}
	default:
		break;
	}

	return true;
};