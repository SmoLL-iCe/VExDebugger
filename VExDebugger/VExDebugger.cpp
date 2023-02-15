#include "Headers/Header.h"
#include "Headers/VExInternal.h"
#include "VEH/VEH.h"
#include "Tools/WinWrap.h"
#include "Tools/Logs.h"
#include "Config/Config.h"
#include "Headers/LogsException.hpp"
#include "HwBkp/MgrHwBkp.h"

CRITICAL_SECTION		HandlerCS					= { };

TAssocExceptionList		AddressAssocExceptionList	= { };

TBreakpointList         BreakpointList				= { };

std::map<uintptr_t, ExceptionInfoList>& VExInternal::GetAssocExceptionList( )
{
	return AddressAssocExceptionList;
}

std::map<uintptr_t, BkpInfo>& VExInternal::GetBreakpointList( )
{
	return BreakpointList;
}

void VExDebugger::CallAssocExceptionList( const std::function<void( TAssocExceptionList& )>& lpEnumFunc )
{
	EnterCriticalSection( &HandlerCS );

	if ( lpEnumFunc )
		lpEnumFunc( AddressAssocExceptionList );

	LeaveCriticalSection( &HandlerCS );
}

void VExDebugger::CallBreakpointList( const std::function<void( TBreakpointList& )>& lpEnumFunc )
{
	EnterCriticalSection( &HandlerCS );

	if ( lpEnumFunc )
		lpEnumFunc( BreakpointList );

	LeaveCriticalSection( &HandlerCS );
}

void*		p_VExDebugger			= nullptr;

void*		OriginalHandlerFilter	= nullptr;

//https://en.wikipedia.org/wiki/X86_debug_register

#define SET_TRAP_FLAG(ctx)			ctx->EFlags |= (1 << 8)

#define UNSET_TRAP_FLAG(ctx)		ctx->EFlags &= ~(1 << 8)

#define SET_RESUME_FLAG(ctx)		ctx->EFlags |= 0x10000u // RF

#define IS_TRAP_FLAG(ctx)			( ctx->Dr6 & 0x4000 ) // check is TF

#define UNSET_HWBKP_POS( ctx, pos ) ( &ctx->Dr0)[pos] = 0; \
ctx->Dr7 &= ~( 1 << pos );

#define SET_TRAP_FLAG_STEP_OUT(ctx) ctx->EFlags &= ~0x100;

long __stdcall HandlerFilter( EXCEPTION_POINTERS* pExceptionInfo )
{
	EnterCriticalSection( &HandlerCS );
	
	constexpr auto	STATUS_WX86_SINGLE_STEP = 0x4000001E; // CE check this old flag too

	auto* const		pContext				= pExceptionInfo->ContextRecord;

	auto* const		pExceptionRec			= pExceptionInfo->ExceptionRecord;

	auto const		ExceptionAddress		= reinterpret_cast<uintptr_t>( pExceptionRec->ExceptionAddress );

	//DisplayContextLogs( pContext, pExceptionRec ); // tests

	if ( EXCEPTION_SINGLE_STEP == pExceptionRec->ExceptionCode || 
		STATUS_WX86_SINGLE_STEP == pExceptionRec->ExceptionCode )
	{
		for ( const auto & [ Address, BpInfo ] : BreakpointList )
		{
			if ( BpInfo.Method != BkpMethod::Hardware )             // only support hardware breakpoint
				continue;


			if ( ( pContext->Dr6 & ( static_cast<uintptr_t>( 1 ) << BpInfo.Pos ) ) == 0 ) // check if this position was setted
				continue;

			const auto itHwBkp		= MgrHwBkp::GetHwBrkpList( ).find( Address );

			if ( itHwBkp == MgrHwBkp::GetHwBrkpList( ).end( ) )
				continue;

			const auto HwBkp		= itHwBkp->second;

			// if it's doesnt have this address, add or update info
			auto & ExceptionList	= AddressAssocExceptionList[ Address ];

			auto& Info				= ExceptionList[ 
				( HwBkp->GetTriggerType( ) != BkpTrigger::Execute ) ? ExceptionAddress : GetCurrentThreadId( ) 
			];
		
			++Info.Details.Count;                               // inc occurrences

			Info.Details.ThreadId	= GetCurrentThreadId( );    // last thread triggered

			Info.Details.Ctx		= *pContext;                // save context
			
			if ( Config::i( )->m_Logs )
				DisplayContextLogs( pContext, pExceptionRec );  // save in txt

			if ( HwBkp->GetTriggerType( ) == BkpTrigger::Execute )
			{
				SET_RESUME_FLAG( pContext );
			}

			LeaveCriticalSection( &HandlerCS );

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	if ( OriginalHandlerFilter )
	{
		LeaveCriticalSection( &HandlerCS );

		return reinterpret_cast<decltype( HandlerFilter )*>( OriginalHandlerFilter )( pExceptionInfo );
	}

	LeaveCriticalSection( &HandlerCS );

	return EXCEPTION_EXECUTE_HANDLER;
}

bool VExDebugger::StartMonitorAddress( const uintptr_t Address, const BkpTrigger Trigger, const BkpSize Size )
{
	EnterCriticalSection( &HandlerCS );

	auto r = MgrHwBkp::SetBkpAddressInAllThreads( Address, Trigger, Size );

	LeaveCriticalSection( &HandlerCS );

	return r;
}

void VExDebugger::RemoveMonitorAddress( const uintptr_t Address )
{
	EnterCriticalSection( &HandlerCS );

	MgrHwBkp::RemoveBkpAddressInAllThreads( Address );

	LeaveCriticalSection( &HandlerCS );
}

bool VExDebugger::Init( HandlerType Type, bool SpoofHwbkp, bool Logs )
{
	if ( HandlerCS.SpinCount == 0 )
	{
		InitializeCriticalSection( &HandlerCS );
	}

	Config::i( )->m_HandlerType = Type;

	Config::i( )->m_SpoofHwbkp	= SpoofHwbkp;

	Config::i( )->m_Logs		= Logs;

	if ( !WinWrap::Init( ) )
		return false;

	nLog::Init( );

	switch ( Type )
	{
	case HandlerType::VectoredExceptionHandler:
		p_VExDebugger = RtlAddVectoredExceptionHandler( 1, HandlerFilter );
		break;
	case HandlerType::UnhandledExceptionFilter:
		p_VExDebugger = SetUnhandledExceptionFilter( HandlerFilter );
		break;
	case HandlerType::VectoredExceptionHandlerIntercept:
		return VEH_Internal::InterceptVEHHandler( HandlerFilter, OriginalHandlerFilter );
	default:
		return false;
	}

	return true;
}

