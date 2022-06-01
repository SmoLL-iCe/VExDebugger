//#include "framework.h"
#include "../import/include/VExDebug.h"

void open_console(const std::string& title)
{
    FILE* stream = nullptr;
    AllocConsole( );
    freopen_s( &stream, "CONIN$",  "r", stdin  );
    freopen_s( &stream, "CONOUT$", "w", stdout );
    freopen_s( &stream, "CONOUT$", "w", stderr );
    SetConsoleTitleA( title.c_str( ) );
}

BOOL __stdcall DllMain( HMODULE h_mod, DWORD ul_reason_for_call, LPVOID reversed )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		open_console("");
		//VExDebug::init();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    default: ;
    }
    return TRUE;
}

