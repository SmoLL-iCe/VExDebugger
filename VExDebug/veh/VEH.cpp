#include "VEH.h"
#include "../Tools/ntos.h"
#include <iostream>
#include "Structs.hpp"

using tVectoredHandler = long( __stdcall* )( EXCEPTION_POINTERS* );

void** pKiUserExceptionDispatcherInterceptPointer   = nullptr;

extern "C" void* pKiUserExceptionDispatcherTrampo   = nullptr;

void* oKiUserExceptionDispatcher                    = nullptr;

void* OrigVectoredHandler                           = nullptr;

uint8_t* LdrpVectorHandlerList						= nullptr;

tVectoredHandler pMyVectoredHandler					= nullptr;

extern "C" 
#ifdef _WIN64
uintptr_t __fastcall HandleKiUserExceptionDispatcher( PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord )
#else
uintptr_t __stdcall HandleKiUserExceptionDispatcher( PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord )
#endif
{
	EXCEPTION_POINTERS Exception{};

    Exception.ExceptionRecord		= ExceptionRecord;

    Exception.ContextRecord			= ContextRecord;

	auto* const pExceptionRec		= Exception.ExceptionRecord;

	auto Result						= pMyVectoredHandler( &Exception );
	
	if ( Result == EXCEPTION_CONTINUE_EXECUTION )
	{
		NtContinue( ContextRecord, 0 );
		return 1;
	}

    return Result;
}

#ifndef _WIN64
__declspec( naked )
VOID KiUserExceptionDispatcherAsm( )
{
    __asm
    {
        mov eax, [ esp + 4 ]
        mov ecx, [ esp ]
        push eax
        push ecx
        call HandleKiUserExceptionDispatcher
        cmp eax, 1
        jne _return
        jmp dword ptr[ pKiUserExceptionDispatcherTrampo ]
        _return:
        ret
    }
}
#endif // _WIN64

bool IniVEH( )
{
	auto* ntdll = GetModuleHandle( L"ntdll.dll" );
	if ( !ntdll )
	{
		return false;
	}

	auto ntdll_base		= reinterpret_cast<uintptr_t>( ntdll );

	auto Dos			= reinterpret_cast<PIMAGE_DOS_HEADER>( ntdll );

	auto Nt				= reinterpret_cast<PIMAGE_NT_HEADERS>( ntdll_base + Dos->e_lfanew );

	auto ntdll_base_end = ntdll_base + Nt->OptionalHeader.SizeOfImage;

	auto* oRtlAddVectoredExceptionHandler = reinterpret_cast<uint8_t*>( GetProcAddress( ntdll, "RtlAddVectoredExceptionHandler" ) );

#ifndef _WIN64

	auto GetVectoredHandlerList = []( uint8_t* Point, uintptr_t Base, uintptr_t EndBase ) -> uint8_t*
	{
		if ( reinterpret_cast<uintptr_t>( Point ) < Base || reinterpret_cast<uintptr_t>( Point ) > EndBase )
			return nullptr;

		uint8_t* pVectoredHandlerList = nullptr;
		for ( size_t x = 0; x < 0x200; x++ )
		{
			if ( !pVectoredHandlerList && Point[ x ] == 0x05 && Point[ x + 5 ] == 0x89 ) //add eax, LdrpVectorHandlerList // mov [ebp-04],eax
			{
				auto VectoredHandlerList = *reinterpret_cast<uint32_t*>( &Point[ x + 1 ] );
				if ( VectoredHandlerList > Base && VectoredHandlerList < EndBase )
				{
					pVectoredHandlerList = reinterpret_cast<uint8_t*>( VectoredHandlerList );

					printf( "pVectoredHandlerList %p\n", pVectoredHandlerList );
					break;
				}
				else
				{
					printf( "VectoredHandlerList fail %d\n", x );
				}
			}
		}
		return pVectoredHandlerList;
	};

	uint8_t* fPoint = nullptr;

	for ( size_t i = 0; i < 0x30; i++ )
	{
		if ( oRtlAddVectoredExceptionHandler[ i ] == 0xE8 )
			fPoint = &oRtlAddVectoredExceptionHandler[ i ] + 5 + *reinterpret_cast<uint32_t*>( &oRtlAddVectoredExceptionHandler[ i + 1 ] );

		if ( fPoint )
		{
			LdrpVectorHandlerList = GetVectoredHandlerList( fPoint, ntdll_base, ntdll_base_end );

			if ( LdrpVectorHandlerList )
				break;
		}				
	}
#else
	auto GetVectoredHandlerList = []( uint8_t* Point, uintptr_t Base, uintptr_t EndBase ) -> uint8_t*
	{
		if ( reinterpret_cast<uintptr_t>( Point ) < Base || reinterpret_cast<uintptr_t>( Point ) > EndBase )
			return nullptr;

		uint8_t* pVectoredHandlerList = nullptr;
		for ( size_t x = 0; x < 0x200; x++ )
		{
			if ( !pVectoredHandlerList && Point[ x ] == 0x83 && Point[ x + 2 ] == 0x3F && Point[ x + 3 ] == 0x48 ) //and eax, 3Fh // lea rdi, LdrpVectorHandlerList
			{
				auto VectoredHandlerList = reinterpret_cast<uintptr_t>( &Point[ x + 3 ] ) + (*reinterpret_cast<uint32_t*>( &Point[x + 3 + 3] ) + 7 );
				if ( VectoredHandlerList > Base && VectoredHandlerList < EndBase )
				{
					pVectoredHandlerList = reinterpret_cast<uint8_t*>( VectoredHandlerList );
					printf( "pVectoredHandlerList %p\n", pVectoredHandlerList );
					break;
				}
				else
				{
					printf( "VectoredHandlerList fail %d\n", x );
				}
			}
		}
		return pVectoredHandlerList;
	};

	uint8_t* fPoint = nullptr;

	for ( size_t i = 0; i < 0x30; i++ )
	{
		if ( oRtlAddVectoredExceptionHandler[ i ] == 0xE8 || oRtlAddVectoredExceptionHandler[ i ] == 0xE9 )
			fPoint = &oRtlAddVectoredExceptionHandler[ i ] + 5 + *reinterpret_cast<uint32_t*>( &oRtlAddVectoredExceptionHandler[ i + 1 ] );

		if ( fPoint )
		{
			LdrpVectorHandlerList = GetVectoredHandlerList( fPoint, ntdll_base, ntdll_base_end );

			if ( LdrpVectorHandlerList )
				break;
		}
	}
#endif // _WIN64

	oKiUserExceptionDispatcher = GetProcAddress( ntdll, "KiUserExceptionDispatcher" );

	if ( !oKiUserExceptionDispatcher )
	{
		return false;
	}

	auto* pKiUserExceptionDispatcher = reinterpret_cast<uint8_t*>( oKiUserExceptionDispatcher );
#ifndef _WIN64
	for ( size_t i = 0; i < 0x20; i++ )
	{
		if ( pKiUserExceptionDispatcher[ i ] == 0xFC && pKiUserExceptionDispatcher[ i + 1 ] == 0x8B )
		{
			pKiUserExceptionDispatcherTrampo = reinterpret_cast<void*>( &pKiUserExceptionDispatcher[ i ] );

			auto endpoint = static_cast<uint32_t>( &pKiUserExceptionDispatcher[ i ] - pKiUserExceptionDispatcher );
			for ( size_t x = 0; x < endpoint; x++ )
			{
				if ( pKiUserExceptionDispatcher[ x ] == 0x83 && pKiUserExceptionDispatcher[ x + 1 ] == 0x3D )
				{
					pKiUserExceptionDispatcherInterceptPointer = *reinterpret_cast<void***>( &pKiUserExceptionDispatcher[ x + 2 ] );
					break;
				}
			}

			break;
		}
	}
#else
	for ( size_t i = 0; i < 0x20; i++ )
	{
		if ( !pKiUserExceptionDispatcherTrampo && pKiUserExceptionDispatcher[ i ] == 0x74 && pKiUserExceptionDispatcher[ i - 3 ] == 0x48 )
		{
			pKiUserExceptionDispatcherTrampo = reinterpret_cast<void*>( pKiUserExceptionDispatcher + i + pKiUserExceptionDispatcher[ i + 1 ] + 2 );
		}
		if ( !pKiUserExceptionDispatcherInterceptPointer && 
			pKiUserExceptionDispatcher[ i ] == 0x48 && pKiUserExceptionDispatcher[ i + 1 ] == 0x8B && pKiUserExceptionDispatcher[ i + 2 ] == 0x05 )
		{
			pKiUserExceptionDispatcherInterceptPointer =
				reinterpret_cast<void**>( &pKiUserExceptionDispatcher[ i ] +
					*reinterpret_cast<uint32_t*>( &pKiUserExceptionDispatcher[ i + 3 ] ) + 7 );
		}
	}
#endif // _WIN64

	printf( "pKiUserExceptionDispatcherInterceptPointer %p\n", pKiUserExceptionDispatcherInterceptPointer );
	printf( "pKiUserExceptionDispatcherTrampo %p\n", pKiUserExceptionDispatcherTrampo );
	return ( pKiUserExceptionDispatcherInterceptPointer && pKiUserExceptionDispatcherTrampo );
	//hk::apply_detour( KiUserExceptionDispatcherAsm, oKiUserExceptionDispatcher );
}

#ifdef _WIN64
extern "C" void KiUserExceptionDispatcherAsm( );
#endif

bool VEH_Internal::HookKiUserExceptionDispatcher( void* MyVectoredHandler )
{
	pMyVectoredHandler = reinterpret_cast<decltype( pMyVectoredHandler )>( MyVectoredHandler );

	auto r = IniVEH( );

	if ( r )
	{

#ifdef _WIN64
		auto Address = (void*)pKiUserExceptionDispatcherInterceptPointer;

		SIZE_T Size = 0x1000;

		ULONG P;

		auto status = NtProtectVirtualMemory( (HANDLE)(- 1 ), &Address, &Size, PAGE_EXECUTE_READWRITE, &P );

		if ( status == 0 )
		{
			*pKiUserExceptionDispatcherInterceptPointer = HandleKiUserExceptionDispatcher;
		}
#else
			*pKiUserExceptionDispatcherInterceptPointer = KiUserExceptionDispatcherAsm;
#endif

	}

	return r;
}

bool VEH_Internal::InterceptVEHHandler( void* VectoredHandler, void*& orig_VectoredHandler )
{
	if ( !LdrpVectorHandlerList )
		return false;

	auto* const VEH_Addr			= LdrpVectorHandlerList;

	auto* const VectoredHandlerList	= reinterpret_cast<VECTORED_HANDLER_LIST*>( VEH_Addr );// grab list somehow;

	auto* const ForwardLink			= VectoredHandlerList->ExecuteHandlerList.Flink;

	for ( auto* pLink = ForwardLink; pLink != &VectoredHandlerList->ExecuteHandlerList; pLink = pLink->Flink )
	{
		auto* const HandlerEntry	= reinterpret_cast<PVECTORED_HANDLER_ENTRY>( pLink );

		void* const DecodedPointer	= ( HandlerEntry->Old.Refs < sizeof( ULONG ) ) ? RtlDecodePointer( HandlerEntry->Old.Handler ) : RtlDecodePointer( HandlerEntry->New.Handler );

		orig_VectoredHandler		= DecodedPointer;

        //if ( HandlerEntry->Old.Refs < sizeof( ULONG ) )
        //    *reinterpret_cast<void**>( &HandlerEntry->Old.Handler ) = RtlEncodePointer( VectoredHandler );
        //else 
        //    *reinterpret_cast<void**>( &HandlerEntry->New.Handler ) = RtlEncodePointer( VectoredHandler );

        printf( "DecodedPointer: 0x%p\n", DecodedPointer );

		return true;

    }

	return false;
}