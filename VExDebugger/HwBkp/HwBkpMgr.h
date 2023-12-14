#pragma once
#include <windows.h>
#include "../../import/include/VExDebugger.h"
#include "HwBkp.h"

namespace HwBkpMgr 
{ 
	bool SetBkpAddressInAllThreads( const uintptr_t Address, const BkpTrigger Type, const BkpSize Size, TCallback Callback = {} );
	bool RemoveBkpAddressInAllThreads( const uintptr_t Address );
	void UpdateInfo( );
	std::map<uintptr_t, HwBkp*>& GetHwBrkpList( );
}