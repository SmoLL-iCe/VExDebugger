#pragma once
#include "../../import/include/VExDebugger.h"

#define SET_TRAP_FLAG(ctx)			ctx->EFlags |= (1 << 8)

#define UNSET_TRAP_FLAG(ctx)		ctx->EFlags &= ~(1 << 8)

#define SET_RESUME_FLAG(ctx)		ctx->EFlags |= 0x10000u // RF

#define IS_TRAP_FLAG(ctx)			( ctx->Dr6 & 0x4000 ) // check is TF

#define UNSET_HWBKP_POS( ctx, pos ) ( &ctx->Dr0)[pos] = 0; \
ctx->Dr7 &= ~( 1 << pos );

#define SET_TRAP_FLAG_STEP_OUT(ctx) ctx->EFlags &= ~0x100;

constexpr auto	STATUS_WX86_SINGLE_STEP = 0x4000001E; // CE check this old flag too

#define IS_SINGLE_STEP(pExcept) ( EXCEPTION_SINGLE_STEP == pExcept->ExceptionRecord->ExceptionCode || \
STATUS_WX86_SINGLE_STEP == pExcept->ExceptionRecord->ExceptionCode )

namespace VExInternal
{
	std::map<uintptr_t, ExceptionInfoList>& GetAssocExceptionList( );

	std::map<uintptr_t, BkpInfo>& GetBreakpointList( );
}
