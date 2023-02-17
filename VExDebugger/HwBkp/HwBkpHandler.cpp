#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "HwBkpHandler.h"
#include "../Config/Config.h"
#include "MgrHwBkp.h"
#include "../Headers/LogsException.hpp"

//https://en.wikipedia.org/wiki/X86_debug_register

long __stdcall MgrHwBkp::Handler( EXCEPTION_POINTERS* pExceptionInfo )
{
	if ( MgrHwBkp::GetHwBrkpList( ).empty( ) || !IS_SINGLE_STEP( pExceptionInfo ) )
		return EXCEPTION_EXECUTE_HANDLER;

	auto* const		pContext				= pExceptionInfo->ContextRecord;

	auto* const		pExceptionRec			= pExceptionInfo->ExceptionRecord;

	auto const		ExceptionAddress		= reinterpret_cast<uintptr_t>( pExceptionRec->ExceptionAddress );

	for ( const auto & [ Address, BpInfo ] : VExInternal::GetBreakpointList() )
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
			auto & ExceptionList	= VExInternal::GetAssocExceptionList( )[ Address ];

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

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	
	return EXCEPTION_EXECUTE_HANDLER;
}