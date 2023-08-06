#pragma once
#include "../../import/include/VExDebugger.h"


#ifndef _WIN64
#define REG(v) E##v
#else
#define REG(v) R##v
#endif



namespace VExInternal
{
	std::map<uintptr_t, ExceptionInfoList>& GetAssocExceptionList( );

	std::map<uintptr_t, BkpInfo>& GetBreakpointList( );
}
