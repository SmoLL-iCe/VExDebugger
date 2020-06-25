#include "header.h"
#include "imgui.h"
#include "interface/interface.h"
#include "utils/utils.h"

int main()
{
	std::cout << "initialized-\n";
	VExDebug::init( );
    gui::init( );
    gui::set_frame( 0 );
	std::cout << "close\n";
	return getchar( );
}

void open_console( const std::string& title )
{
	FILE* stream = nullptr;
	AllocConsole( );
	freopen_s( &stream, "CONIN$",  "r", stdin  );
	freopen_s( &stream, "CONOUT$", "w", stdout );
	freopen_s( &stream, "CONOUT$", "w", stderr );
	SetConsoleTitleA( title.c_str( ) );
}

BOOL __stdcall DllMain( HMODULE h_module, DWORD  ul_reason_for_call, LPVOID reserved )
{
	UNREFERENCED_PARAMETER( h_module ); UNREFERENCED_PARAMETER( reserved );
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		open_console( "" );
		main( );
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	default:;
	}
	return TRUE;
}
