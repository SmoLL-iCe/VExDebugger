#pragma once
#include <windows.h>
#include "../../import/include/VExDebugger.h"
#include "HwBkp.h"

namespace MgrHwBkp 
{
	bool SetBkpAddressInAllThreads( const uintptr_t Address, const HwbkpType Type, const HwbkpSize Size );
	void RemoveBkpAddressInAllThreads( const uintptr_t Address );
	void UpdateInfo( );
}