#pragma once
#include "../../import/include/VExDebugger.h"

namespace VExInternal
{
	std::map<uintptr_t, ExceptionInfoList>& GetAssocExceptionList( );

	std::map<uintptr_t, BkpInfo>& GetBreakpointList( );
}