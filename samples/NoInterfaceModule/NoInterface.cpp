
#include "framework.h"



void Set( )
{
    //VExDebugger::StartMonitorAddress( result_convert, static_cast<BkpTrigger>( type_current + 1 ), static_cast<BkpSize>( size_current ) );
}

void Initialize( )
{
    
    CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( Set ), nullptr, 0, nullptr );
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //Initialize( );
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

