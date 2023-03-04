#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "HwBkpHandler.h"
#include "../Config/Config.h"
#include "MgrHwBkp.h"
#include "../Headers/LogsException.hpp"

CONTEXT gLastCtx = { 0 };

long __stdcall MgrHwBkp::ExceptionHandler( EXCEPTION_POINTERS* pExceptionInfo )
{
	if ( MgrHwBkp::GetHwBrkpList( ).empty( ) )
		return EXCEPTION_EXECUTE_HANDLER;

	auto* const		pContext				= pExceptionInfo->ContextRecord;

	auto* const		pExceptionRec			= pExceptionInfo->ExceptionRecord;

	auto const		ExceptionAddress		= reinterpret_cast<uintptr_t>( pExceptionRec->ExceptionAddress );

#ifndef _WIN64
	auto const		SpoofDrCtx				= [&]( ) {

		if ( Config::i( )->m_SpoofHwbkp )
		{
			for ( size_t i = 0; i < 6; i++ )
			{
				auto const pDrCtx		= &pContext->Dr0;

				auto const pSaveDrCtx	= &gLastCtx.Dr0;

				pSaveDrCtx[ i ]			= pDrCtx[ i ];

				pDrCtx[ i ]				= 0;
			}
		}
	};
#endif // _WIN32

	if ( !IS_SINGLE_STEP( pExceptionInfo ) )
	{
#ifndef _WIN64
		SpoofDrCtx( );
#endif // _WIN32
		return EXCEPTION_EXECUTE_HANDLER;
	}

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

#ifndef _WIN64
			SpoofDrCtx( );
#endif // _WIN32

			return EXCEPTION_CONTINUE_EXECUTION;
		}

#ifndef _WIN64
	SpoofDrCtx( );
#endif // _WIN32

	return EXCEPTION_EXECUTE_HANDLER;
}

long __stdcall MgrHwBkp::ContinueHandler( EXCEPTION_POINTERS* pExceptionInfo )
{
	if ( MgrHwBkp::GetHwBrkpList( ).empty( ) )
		return EXCEPTION_EXECUTE_HANDLER;

#ifndef _WIN64
	if ( Config::i( )->m_SpoofHwbkp )
	{
		auto pContext = pExceptionInfo->ContextRecord;

		if ( pContext->Dr7 != gLastCtx.Dr7 )
		{
			for ( size_t i = 0; i < 6; i++ )
			{
				( &pContext->Dr0 )[ i ] =
					( &gLastCtx.Dr0 )[ i ];
			}
		}
	}
#endif // _WIN32

	return EXCEPTION_CONTINUE_EXECUTION;
}