#pragma once
#include <Windows.h>
#include <iostream>
#include <map>

using ExceptionAddressCount = std::map<void*, uint32_t>;

enum class HwbkpType
{
	Execute,
	ReadWrite,
	Write
};

enum class HwbkpSize
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

namespace VExDebug
{
	bool Init( HandlerType Type = HandlerType::VectoredExceptionHandler, bool SpoofHwbkp = false, bool Logs = false );

	std::map<int, ExceptionAddressCount>& GetExceptionAssocAddress( );

	std::map<int, uintptr_t>& GetAddressAssocException( );

	bool StartMonitorAddress( uintptr_t Address, HwbkpType Type, HwbkpSize Size );
	
	void RemoveMonitorAddress( uintptr_t Address );
	
	void PrintExceptions( );
}