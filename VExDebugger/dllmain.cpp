#include "../import/include/VExDebugger.h"

BOOL __stdcall DllMain( HMODULE h_mod, DWORD ul_reason_for_call, LPVOID reversed )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    default: ;
    }
    return TRUE;
}

