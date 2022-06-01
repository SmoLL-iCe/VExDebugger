#include "VEH.h"
#define AAAAA
#include "../utils/ntos.h"
#include <iostream>
#include "veh_structs.hpp"

DWORD GetVEHOffset( )
{
    /*
    ntdll.RtlAddVectoredExceptionHandler+E1 - 8B C8                 - mov ecx,eax
    ntdll.RtlAddVectoredExceptionHandler+E3 - 33 C7                 - xor eax,edi
    ntdll.RtlAddVectoredExceptionHandler+E5 - 83 E1 1F              - and ecx,1F
    ntdll.RtlAddVectoredExceptionHandler+E8 - D3 C8                 - ror eax,cl
    ntdll.RtlAddVectoredExceptionHandler+EA - 89 46 10              - mov [esi+10],eax
    ntdll.RtlAddVectoredExceptionHandler+ED - 6B 45 08 0C           - imul eax,[ebp+08],0C
    ntdll.RtlAddVectoredExceptionHandler+F1 - 53                    - push ebx
    ntdll.RtlAddVectoredExceptionHandler+F2 - 05 34C35677           - add eax,ntdll.dll+12C334
    ntdll.RtlAddVectoredExceptionHandler+F7 - 89 45 FC              - mov [ebp-04],eax
    ntdll.RtlAddVectoredExceptionHandler+FA - 8D 78 04              - lea edi,[eax+04]
    ntdll.RtlAddVectoredExceptionHandler+FD - E8 AF5B0200           - call ntdll.RtlReleaseSRWLockExclusive+61
    ntdll.RtlAddVectoredExceptionHandler+102- 8B 5D FC              - mov ebx,[ebp-04]
    ntdll.RtlAddVectoredExceptionHandler+105- FF 33                 - push [ebx]
    ntdll.RtlAddVectoredExceptionHandler+107- E8 54560200           - call ntdll.RtlAcquireSRWLockExclusive
    ntdll.RtlAddVectoredExceptionHandler+10C- 39 3F                 - cmp [edi],edi
    ntdll.RtlAddVectoredExceptionHandler+10E- 75 13                 - jne ntdll.RtlAddVectoredExceptionHandler+123


    */

    //ntdll.RtlAddVectoredExceptionHandler+BF - 81 C3 3C93F577        - add ebx,ntdll.dll+12933C

    //return 0x12C334;
    return 0x12933C;
    //ntdll.RtlAddVectoredExceptionHandler+F2 - 05 34C35677           - add eax,ntdll.dll+12C334 // win11

}

void VEH_internal::HookVEHHandlers( void* VectoredHandler, void*& orig_VectoredHandler )
{
    uintptr_t ntdll = (uintptr_t)LoadLibrary( L"ntdll.dll" );

    uintptr_t veh_addr = ntdll + GetVEHOffset( );

    //handler_list = (VECTORED_HANDLER_LIST*)veh_addr;

    PVECTORED_HANDLER_LIST vectored_handler_list_ = (VECTORED_HANDLER_LIST*)veh_addr;// grab list somehow;
    {
        PLIST_ENTRY forward_link = vectored_handler_list_->ExecuteHandlerList.Flink;

        for ( auto* link = forward_link; link != &vectored_handler_list_->ExecuteHandlerList; link = link->Flink )
        {
            auto* handler_entry = reinterpret_cast<PVECTORED_HANDLER_ENTRY>( link );
            void* decoded_pointer = ( handler_entry->Old.Refs < sizeof( ULONG ) ) ? RtlDecodePointer( handler_entry->Old.Handler ) : RtlDecodePointer( handler_entry->New.Handler );

            orig_VectoredHandler = decoded_pointer;

            if ( handler_entry->Old.Refs < sizeof( ULONG ) )
                *reinterpret_cast<void**>( &handler_entry->Old.Handler ) = RtlEncodePointer( VectoredHandler );
            else 
                *reinterpret_cast<void**>( &handler_entry->New.Handler ) = RtlEncodePointer( VectoredHandler );

            printf( "Swiped: 0x%p\n", decoded_pointer );

            break;

        }
    }
}