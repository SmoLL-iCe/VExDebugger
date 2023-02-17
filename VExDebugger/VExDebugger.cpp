#include "Headers/Header.h"
#include "Headers/VExInternal.h"
#include "VEH/VEH.h"
#include "Tools/WinWrap.h"
#include "Tools/Logs.h"
#include "Config/Config.h"
#include "HwBkp/MgrHwBkp.h"
#include "HwBkp/HwBkpHandler.h"

bool					isCsInitialized				= false;

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

void*		p_VExDebugger			= nullptr;

void*		OriginalHandlerFilter	= nullptr;

long __stdcall InitialHandler( EXCEPTION_POINTERS* pExceptionInfo )
{
	if ( !pExceptionInfo || !pExceptionInfo->ExceptionRecord || !pExceptionInfo->ContextRecord )
	{ // maybe trap

		if ( OriginalHandlerFilter )
			return reinterpret_cast<long (__stdcall*)( EXCEPTION_POINTERS* )>( OriginalHandlerFilter )( pExceptionInfo );
		
		return EXCEPTION_EXECUTE_HANDLER;
	}

	EnterCriticalSection( &HandlerCS );

	//DisplayContextLogs( pContext, pExceptionRec ); // tests

	auto Result = MgrHwBkp::Handler( pExceptionInfo );
	
	if ( OriginalHandlerFilter && EXCEPTION_EXECUTE_HANDLER == Result )
	{
		LeaveCriticalSection( &HandlerCS );

		return reinterpret_cast<decltype( InitialHandler )*>( OriginalHandlerFilter )( pExceptionInfo );
	}

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

	nLog::Init( );
	bool Result = false;

	switch ( Type )
	{
	case HandlerType::VectoredExceptionHandler:
		p_VExDebugger = RtlAddVectoredExceptionHandler( 1, InitialHandler );
		Result = ( p_VExDebugger != nullptr );
		break;
	case HandlerType::UnhandledExceptionFilter:
		p_VExDebugger = SetUnhandledExceptionFilter( InitialHandler );
		Result = ( p_VExDebugger != nullptr );
		break;
	case HandlerType::VectoredExceptionHandlerIntercept:
		return VEH_Internal::InterceptVEHHandler( InitialHandler, OriginalHandlerFilter );
	default:
		break;
	}

	return Result;
}

