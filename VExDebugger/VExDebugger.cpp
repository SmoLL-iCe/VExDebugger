#include "Headers/Header.h"
#include "Headers/VExInternal.h"
#include "VEH/VEH.h"
#include "Tools/WinWrap.h"
#include "Tools/Logs.h"
#include "Config/Config.h"
#include "HwBkp/MgrHwBkp.h"
#include "HwBkp/HwBkpHandler.h"
#include "SpoofDbg/SpoofDbg.h"
#include "Headers/LogsException.hpp"

bool                    isCsInitialized				= false;

CRITICAL_SECTION        HandlerCS					= { };

TAssocExceptionList     AddressAssocExceptionList	= { };

TBreakpointList         BreakpointList				= { };

void*                   pLvlExcptFilter				= nullptr;

void*                   pExcpHandlerEntry			= nullptr;

void*                   pCtnHandlerEntry			= nullptr;

void*                   OriginalHandlerFilter		= nullptr;

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
	if ( !isCsInitialized )
		return;
	
	EnterCriticalSection( &HandlerCS );

	if ( lpEnumFunc )
		lpEnumFunc( AddressAssocExceptionList );

	LeaveCriticalSection( &HandlerCS );
}

void VExDebugger::CallBreakpointList( const std::function<void( TBreakpointList& )>& lpEnumFunc )
{
	if ( !isCsInitialized )
		return;

	EnterCriticalSection( &HandlerCS );

	if ( lpEnumFunc )
		lpEnumFunc( BreakpointList );

	LeaveCriticalSection( &HandlerCS );
}

long __stdcall InitialExceptionHandler( EXCEPTION_POINTERS* pExceptionInfo )
{
	if ( !pExceptionInfo || !pExceptionInfo->ExceptionRecord || !pExceptionInfo->ContextRecord )
	{ // maybe trap

		if ( OriginalHandlerFilter )
			return reinterpret_cast<long (__stdcall*)( EXCEPTION_POINTERS* )>( OriginalHandlerFilter )( pExceptionInfo );
		
		return EXCEPTION_EXECUTE_HANDLER;
	}

	EnterCriticalSection( &HandlerCS );

	//DisplayContextLogs( pExceptionInfo->ContextRecord, pExceptionInfo->ExceptionRecord ); // tests

	auto Result = MgrHwBkp::ExceptionHandler( pExceptionInfo );

	if ( pLvlExcptFilter && EXCEPTION_CONTINUE_EXECUTION == Result )
	{
		LeaveCriticalSection( &HandlerCS );

		WinWrap::Continue( pExceptionInfo->ContextRecord, FALSE );

		return Result;
	}
	
	if ( OriginalHandlerFilter && EXCEPTION_EXECUTE_HANDLER == Result )
	{
		LeaveCriticalSection( &HandlerCS );

		return reinterpret_cast<decltype( InitialExceptionHandler )*>( OriginalHandlerFilter )( pExceptionInfo );
	}

	LeaveCriticalSection( &HandlerCS );

	return Result;
}

long __stdcall InitialContinueHandler( EXCEPTION_POINTERS* pExceptionInfo )
{
	if ( !pExceptionInfo || !pExceptionInfo->ExceptionRecord || !pExceptionInfo->ContextRecord )
	{ // maybe trap
		return EXCEPTION_EXECUTE_HANDLER;
	}

	EnterCriticalSection( &HandlerCS );

	// Using the RtlAddVectoredContinueHandler
	// Continue handler is a callback that is called after any exception has been continued

	auto Result = MgrHwBkp::ContinueHandler( pExceptionInfo );

	LeaveCriticalSection( &HandlerCS );

	return Result;
}

bool VExDebugger::StartMonitorAddress( const uintptr_t Address, const BkpTrigger Trigger, const BkpSize Size )
{
	if ( !isCsInitialized )
		return false;

	EnterCriticalSection( &HandlerCS );

	auto r = MgrHwBkp::SetBkpAddressInAllThreads( Address, Trigger, Size );

	LeaveCriticalSection( &HandlerCS );

	return r;
}

bool VExDebugger::SetTracerAddress( const uintptr_t Address, const BkpTrigger Trigger, const BkpSize Size, TCallback Callback )
{
	if ( !isCsInitialized )
		return false;

	EnterCriticalSection( &HandlerCS );

	auto r = MgrHwBkp::SetBkpAddressInAllThreads( Address, Trigger, Size, Callback );

	LeaveCriticalSection( &HandlerCS );

	return r;
}

void VExDebugger::RemoveMonitorAddress( const uintptr_t Address )
{
	if ( !isCsInitialized )
		return;

	EnterCriticalSection( &HandlerCS );

	MgrHwBkp::RemoveBkpAddressInAllThreads( Address );

	LeaveCriticalSection( &HandlerCS );
}

bool VExDebugger::Init( HandlerType Type, bool Logs )
{
	if ( !isCsInitialized )
	{
		InitializeCriticalSection( &HandlerCS );

		isCsInitialized = true;
	}

	Config::i( )->m_HandlerType = Type;

	Config::i( )->m_SpoofHwbkp	= false;

	Config::i( )->m_Logs		= Logs;

	if ( !WinWrap::Init( ) )
		return false;

	//SpoofDbg::HookNtGetContextThread( );
	//SpoofDbg::HookNtContinue( );

	nLog::Init( );


	bool Result = false;

	switch ( Type )
	{
	case HandlerType::VectoredExceptionHandler:

		// HandleEntry is the point of item in the LdrpVectorHandlerList
		// Provides the handler info

		pExcpHandlerEntry	= RtlAddVectoredExceptionHandler( 1, InitialExceptionHandler );

		//pCtnHandlerEntry	= RtlAddVectoredContinueHandler( 1, InitialContinueHandler );

		Result				= ( pExcpHandlerEntry != nullptr );

		break;
	case HandlerType::UnhandledExceptionFilter:

		// If any VEH handle the exception, SEH will catch

		Config::i( )->m_SpoofHwbkp = false;

		pLvlExcptFilter		= SetUnhandledExceptionFilter( InitialExceptionHandler );

		Result				= ( pLvlExcptFilter != nullptr );


		break;
	case HandlerType::VectoredExceptionHandlerIntercept:

		return VEH_Internal::InterceptVEHHandler( InitialExceptionHandler, OriginalHandlerFilter );

	default:
		break;
	}
	return Result;
}

