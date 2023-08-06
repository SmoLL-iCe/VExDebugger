#include "../Headers/Header.h"
#include "../Headers/VExInternal.h"
#include "HwBkpHandler.h"
#include "../Config/Config.h"
#include "MgrHwBkp.h"
#include "../Headers/LogsException.hpp"
#include <algorithm>
#include "../Tools/Utils.cpp"

#define SET_TRAP_FLAG(ctx)			            ctx->EFlags |= (1 << 8)

#define UNSET_TRAP_FLAG(ctx)		            ctx->EFlags &= ~(1 << 8)

#define SET_RESUME_FLAG(ctx)		            ctx->EFlags |= 0x10000u // RF

#define IS_TRAP_FLAG(ctx)			            ( ctx->Dr6 & 0x4000 ) != 0 // check is TF

#define IS_HW_TRIGGERED(ctx)		            ( ctx->Dr6 & 0xF ) != 0

#define GET_HW_TRIGGERED_INDEX(ctx)             static_cast<int>( std::log2( ctx->Dr6 & 0xF ) )

#define IS_TRIGGERED_INDEX( ctx, index ) \
 ( IS_HW_TRIGGERED( ctx ) && GET_HW_TRIGGERED_INDEX( ctx ) == index )

constexpr auto	STATUS_WX86_SINGLE_STEP = 0x4000001E; // CE check this old flag too

#define IS_SINGLE_STEP(pExcept) ( EXCEPTION_SINGLE_STEP == pExcept->ExceptionRecord->ExceptionCode || \
STATUS_WX86_SINGLE_STEP == pExcept->ExceptionRecord->ExceptionCode )

CONTEXT gLastCtx = { 0 };

bool TracerCallBack( std::uintptr_t OriginalAddress, PCONTEXT pContext, PEXCEPTION_RECORD pExceptionRec, BkpInfo BpInfo )
{
	auto IsTF = IS_TRAP_FLAG( pContext );

	auto pDr = &pContext->Dr0;

	if ( IsTF )
	{
		if ( !IS_ENABLE_DR7_INDEX( pContext, BpInfo.Pos ) || ( pDr[ BpInfo.Pos ] != BpInfo.Pos ) )
		{
			return false; //it's not mine
		}
	}

	auto Result = BpInfo.Callback( pExceptionRec, pContext );

	switch ( Result )
	{
	case CBReturn::StopTrace:
		{
			if ( IsTF )
			{
				const auto itHwBkp = MgrHwBkp::GetHwBrkpList( ).find( OriginalAddress ); 

				if ( itHwBkp == MgrHwBkp::GetHwBrkpList( ).end( ) )
					return false;

				const auto HwBkp = itHwBkp->second;// get initial config

				HwBkp->SetDr7Config( pContext ); //restore

				pDr[ BpInfo.Pos ] = OriginalAddress;
			}

			SET_RESUME_FLAG( pContext );

			break;
		}
	case CBReturn::StepInto:
		{
			if ( !IsTF )
			{
				int index = GET_HW_TRIGGERED_INDEX( pContext );

				pDr[ index ] = BpInfo.Pos; // change address to index
			}

			SET_TRAP_FLAG( pContext );

			break;
		}
	case CBReturn::StepOver:
		{
			auto CallSize = Utils::IsCallInstruction(  pContext->REG( ip ) );

			if ( !IsTF )
			{
				int index = GET_HW_TRIGGERED_INDEX( pContext );

				pDr[ index ] = BpInfo.Pos; // change address to index
			}

			if ( CallSize )
			{
				pDr[ BpInfo.Pos ] = pContext->REG( ip ) + CallSize;

				SET_DR7_INDEX_TYPE( pContext, BpInfo.Pos, 0 ); // Exec

				SET_DR7_INDEX_SIZE( pContext, BpInfo.Pos, 0 );

			}else
			SET_TRAP_FLAG( pContext );

			break;
		}
	default:
		break;
	}
	return true;
};

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

			auto IsTF = IS_TRAP_FLAG( pContext );

			if ( BpInfo.Callback && ( IsTF || IS_TRIGGERED_INDEX( pContext, BpInfo.Pos ) ) )
			{

				if ( !TracerCallBack( Address, pContext, pExceptionRec, BpInfo ) )
					continue;

				return EXCEPTION_CONTINUE_EXECUTION;
			}			

			if ( !IS_HW_TRIGGERED( pContext ) ) 
				continue; // maybe it's tf and not mine

			if ( GET_HW_TRIGGERED_INDEX( pContext ) != BpInfo.Pos )
				continue; // maybe it's not my trigger

			const auto itHwBkp      = MgrHwBkp::GetHwBrkpList( ).find( Address );

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