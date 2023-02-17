#include "header.h"
#include "imgui.h"
#include "Interface/Interface.h"
#include "Utils/Utils.h"

int Initialize( )
{
    Gui::Init( );

    Gui::SetWindowIndex( 0 );

	return getchar( );
}

BOOL __stdcall DllMain( HMODULE h_module, DWORD  ul_reason_for_call, LPVOID reserved )
{
	UNREFERENCED_PARAMETER( h_module ); UNREFERENCED_PARAMETER( reserved );
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		VExDebugger::Init( HandlerType::VectoredExceptionHandler, true );
		CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( Initialize ), h_module, 0, nullptr );
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	default:;
	}
	return TRUE;
}
