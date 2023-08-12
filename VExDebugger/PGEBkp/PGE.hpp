#pragma once
#include <algorithm>
#include <vector>

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
};

struct PageGuardException
{
	std::uintptr_t	AllocBase			= 0;
	std::uintptr_t	AllocSize			= 0;
	std::uint32_t	OldProtection		= 0;
	std::uint32_t	SetProtection		= 0;
	//bool		ReSet				= false;
	std::vector<PageGuardTrigger> PGTriggersList;

	bool RestorePageGuardProtection( );
};
