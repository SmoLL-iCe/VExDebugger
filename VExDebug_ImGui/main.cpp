#include "header.h"
#ifndef _DEBUG
#include "imgui.h"
#include "interface/interface.h"
#endif // DEBUG
#include "utils/utils.h"

#ifndef _DEBUG
int main( void* address )
{
	std::cout << "initialized-\n";
    gui::init( );
    gui::set_frame( 0 );
	std::cout << "close\n";

	return getchar( );
}
#endif // DEBUG

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
		//open_console( "" );
		VExDebug::Init( HandlerType::VectoredExceptionHandler, false, true );
#ifndef _DEBUG
		CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( main ), h_module, 0, nullptr );
#endif // DEBUG
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	default:;
	}
	return TRUE;
}
