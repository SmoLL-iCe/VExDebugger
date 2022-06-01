#pragma once
#include <Windows.h>
#ifdef __cplusplus
extern "C" {
#endif
SIZE_T __stdcall wrap_VirtualQuery( LPVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength );
LPVOID __stdcall wrap_VirtualAlloc( LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect );
BOOL __stdcall wrap_VirtualProtect( LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect );
BOOL __stdcall wrap_VirtualFree( LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType );
#ifdef __cplusplus
}
#endif