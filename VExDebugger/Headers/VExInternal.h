#pragma once
#include "../../import/include/VExDebugger.h"

#ifndef _WIN64
#define REG(v) E##v
#else
#define REG(v) R##v
#endif

#define SET_TRAP_FLAG(ctx)			            ctx->EFlags |= (1 << 8)

#define UNSET_TRAP_FLAG(ctx)		            ctx->EFlags &= ~(1 << 8)

#define SET_RESUME_FLAG(ctx)		            ctx->EFlags |= 0x10000u // RF

#define IS_TRAP_FLAG(ctx)			            ( ctx->Dr6 & 0x4000 ) != 0 // check is TF

namespace VExInternal
{
	std::map<uintptr_t, ExceptionInfoList>& GetAssocExceptionList( );

	std::map<uintptr_t, BkpInfo>& GetBreakpointList( );
}
