
#include "framework.h"

#ifdef __cplusplus
extern "C" {
#endif
        __declspec( dllexport )
        bool Init( HandlerType Type, bool SpoofHwbkp, bool Logs);

        __declspec( dllexport )
        bool StartMonitorAddress( uintptr_t Address, HwbkpType Type, HwbkpSize Size );

    
#ifdef __cplusplus 
}
#endif

bool Init( HandlerType Type, bool SpoofHwbkp, bool Logs )
{
    return VExDebugger::Init( Type, SpoofHwbkp, Logs );
}

bool StartMonitorAddress( uintptr_t Address, HwbkpType Type, HwbkpSize Size )
{
    return VExDebugger::StartMonitorAddress( Address, Type, Size );
}

void Set( )
{
    //VExDebugger::StartMonitorAddress( result_convert, static_cast<HwbkpType>( type_current + 1 ), static_cast<HwbkpSize>( size_current ) );
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

