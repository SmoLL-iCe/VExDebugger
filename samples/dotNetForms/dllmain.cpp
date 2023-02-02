#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <iostream>
#include <cstdio>
#include "FrmMain.h"
#include "Utils/Utils.h"

void __stdcall ThreadForm( )
{
	VExDebuggerform::FrmMain^ form = gcnew VExDebuggerform::FrmMain( );
	form->Text = "VExDebugger";
	form->ShowDialog( );
}

BOOL __stdcall DllMain( HMODULE h_module, DWORD  ul_reason_for_call, LPVOID reserved )
{
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		Utils::OpenConsole( "" );
		CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( ThreadForm ), nullptr, 0, nullptr );
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	default:;
	}
	return TRUE;
}
