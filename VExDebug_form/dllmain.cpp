#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <iostream>
#include <cstdio>

#include "form_main.h"

void open_console( const std::string& title )
{
	FILE* stream = nullptr;
	AllocConsole( );
	freopen_s( &stream, "CONIN$", "r", stdin );
	freopen_s( &stream, "CONOUT$", "w", stdout );
	freopen_s( &stream, "CONOUT$", "w", stderr );
	SetConsoleTitleA( title.c_str( ) );
}

void __stdcall thread_form( )
{
	VExDebugform::form_main^ form = gcnew VExDebugform::form_main( );
	form->Text = "VExDebug";
	form->ShowDialog( );
}


BOOL __stdcall DllMain( HMODULE h_module, DWORD  ul_reason_for_call, LPVOID reserved )
{
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		open_console( "" );
		CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( thread_form ), nullptr, 0, nullptr );
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	default:;
	}
	return TRUE;
}
