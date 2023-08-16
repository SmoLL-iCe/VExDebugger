#pragma once
#include <Windows.h>
#include <map>
#include "PGE.hpp"

namespace PGEMgr
{ 
	CRITICAL_SECTION * GetCs();
	std::vector<PageGuardException>& GetPageExceptionsList();
	std::map<std::uint32_t, StepBkp>& GetThreadHandlingList();
	bool AddPageExceptions( uintptr_t Address, BkpTrigger TriggerType, BkpSize bSize, TCallback Callback = {} );
	bool RemovePageExceptions( uintptr_t Address, BkpTrigger TriggerType );
}