#include "nt_impt.h"
#include <stdint.h>
#include "../ntos.h"

SIZE_T __stdcall wrap_VirtualQuery( LPVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength)
{
	SIZE_T r_size;
	const NTSTATUS status = NtQueryVirtualMemory( NtCurrentProcess( ), lpAddress, 0, lpBuffer, sizeof( MEMORY_BASIC_INFORMATION ), &r_size );
	return  ( status == 0 ) ?  r_size : 0;
}

LPVOID __stdcall wrap_VirtualAlloc( LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect )
{
	PVOID address = lpAddress;
	SIZE_T Size = dwSize;
	const NTSTATUS status = NtAllocateVirtualMemory( NtCurrentProcess( ), &address, 0, &Size, flAllocationType, flProtect );
	return  ( status == 0 ) ? address : NULL;
}

BOOL __stdcall wrap_VirtualProtect( LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect )
{
	PVOID address = lpAddress;
	SIZE_T Size = dwSize;
	const NTSTATUS status = NtProtectVirtualMemory( NtCurrentProcess( ), &address, &Size, flNewProtect, lpflOldProtect );
	return  ( status == 0 );
}

BOOL __stdcall wrap_VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType )
{
	PVOID address = lpAddress;
	SIZE_T Size = dwSize;
	const NTSTATUS status = NtFreeVirtualMemory( NtCurrentProcess( ), &address, &Size, dwFreeType );
	return  ( status == 0 );
}