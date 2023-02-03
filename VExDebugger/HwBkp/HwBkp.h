#pragma once
#include <windows.h>

class HwBkp
{
private:

	uintptr_t Address			= 0;

	HwbkpType Type				= HwbkpType::Execute;

	HwbkpSize Size				= HwbkpSize::Size_1;

	int DbgRegAvailable			= 0; //max 4

	bool Add					= true;

	bool AnySuccess				= false;

public:

	HwBkp( uintptr_t Address, HwbkpSize Size, HwbkpType Type, bool Add = true );

	static HwBkp* i( );

	void SetRemove( );

	bool ApplyHwbkpDebugConfig( HANDLE hThread, uint32_t ThreadId, bool useExisting = false );

	int GetPos( ) const;

	bool GetAnySuccess( ) const;

	uintptr_t GetAddress( ) const;
};