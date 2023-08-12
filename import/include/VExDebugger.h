#pragma once
#include <Windows.h>
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <winnt.h>

enum class CBReturn
{
	StopTrace,
	StepInto,
	StepOver,
};

using TCallback                     = std::function<CBReturn( PEXCEPTION_RECORD, PCONTEXT )>;

enum class BkpTrigger
{
	Execute,
	ReadWrite,
	Write
};

enum class BkpSize
{
	Size_1,
	Size_2,
	Size_8,
	Size_4
};

enum class HandlerType
{
	VectoredExceptionHandler,
	UnhandledExceptionFilter,
	VectoredExceptionHandlerIntercept,
	KiUserExceptionDispatcherHook,
};

enum class BkpMethod
{
	Hardware,
	PageExceptions,
};

struct BkpInfo
{
	BkpMethod	Method		= BkpMethod::Hardware;

	BkpTrigger	Trigger		= BkpTrigger::Execute;

	BkpSize		Size		= BkpSize::Size_1;

	int         Pos         = 0;

	TCallback   Callback    = {};
};
	
struct CatchedDetails
{
	size_t		Count		= 0;

	size_t		ThreadId	= 0;

	CONTEXT		Ctx			= {};
};

struct ExceptionInfo
{
	CatchedDetails Details{};
};

using ExceptionInfoList				= std::map<uintptr_t, ExceptionInfo>;

using TBreakpointList				= std::map<uintptr_t, BkpInfo>;

using TAssocExceptionList			= std::map<uintptr_t, ExceptionInfoList>;

namespace VExDebugger
{
	bool Init( HandlerType Type = HandlerType::VectoredExceptionHandler, bool Logs = false );

	void CallAssocExceptionList( const std::function<void( TAssocExceptionList& )>& lpEnumFunc );

	void CallBreakpointList( const std::function<void( TBreakpointList& )>& lpEnumFunc );

	bool StartMonitorAddress( const uintptr_t Address, const BkpTrigger Trigger, const BkpSize Size );

	bool SetTracerAddress( const uintptr_t Address, const BkpTrigger Trigger, const BkpSize Size, TCallback Callback );
	
	void RemoveMonitorAddress( uintptr_t Address );

	template <typename T>
	inline bool StartMonitorAddress( T Address, BkpTrigger Trigger, BkpSize Size )
	{
		return StartMonitorAddress( (uintptr_t)(Address), Trigger, Size );
	}

	template <typename T>
	inline bool SetTracerAddress( T Address, BkpTrigger Trigger, BkpSize Size, TCallback Callback )
	{
		return SetTracerAddress( (uintptr_t)( Address ), Trigger, Size, Callback );
	}
}