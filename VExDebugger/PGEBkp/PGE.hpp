#pragma once
#include <algorithm>
#include <vector>
#include "../../import/include/VExDebugger.h"

#define USE_SWBREAKPOINT // it faster than pg

enum PageGuardTriggerType : std::uint32_t
{
	Execute = 8,
	Read = 0,
	Write = 1,
	ReadWrite = 99,
};

struct PageGuardTrigger
{
	PageGuardTriggerType	    Type				= PageGuardTriggerType::Execute;
	std::uintptr_t				Offset				= 0;
	std::uintptr_t				Size				= 0;
	TCallback                   Callback            = {};
};

struct StepBkp
{
	std::uintptr_t	            AllocBase           = 0;
	PageGuardTrigger            Trigger             = {};
#ifdef USE_SWBREAKPOINT
	std::uintptr_t	            AddressToHit        = 0;
	std::uint8_t                OriginalByte        = 0;
#endif
	std::uint32_t               NextExceptionCode   = 0;
};

struct PageGuardException
{
	std::uintptr_t	AllocBase			= 0;
	std::uintptr_t	AllocSize			= 0;
	std::uint32_t	OldProtection		= 0;
	std::uint32_t	SetProtection		= 0;
	std::vector<PageGuardTrigger> PGTriggersList;

	bool RestorePageGuardProtection( ) const;
	bool InRange( std::uintptr_t Address ) const;
};
