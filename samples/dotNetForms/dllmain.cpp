#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <iostream>
#include <cstdio>
#include "FrmMain.h"
#include "Utils/Utils.h"


BOOL __stdcall DllMain( HMODULE h_module, DWORD  ul_reason_for_call, LPVOID reserved )
{
	if ( ul_reason_for_call == DLL_PROCESS_ATTACH )
	{
		CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( VExDebuggerform::InitForm ), nullptr, 0, nullptr );
	}
	return TRUE;
}
