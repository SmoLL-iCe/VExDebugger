#pragma once
#include <windows.h>

class HwBkp
{
private:

	uintptr_t Address		= 0;

	HwbkpType Type			= HwbkpType::Execute;

	HwbkpSize Size			= HwbkpSize::Size_1;

	intptr_t DbgRegAvailable		= 0; //max 4

	bool Add				= true;

public:
	HwBkp( uintptr_t Address, HwbkpSize Size, HwbkpType Type, bool Add = true );

	static HwBkp* i( );

	bool ApplyHwbkpDebugConfig( HANDLE hThread, uint32_t ThreadId, bool useExisting = false );

	bool& AddBkp( );

	uintptr_t GetAddress( ) const;
};