#pragma once
#include <windows.h>

namespace HwBkpMgr
{
	long __stdcall ExceptionHandler( EXCEPTION_POINTERS* pExceptionInfo );
	long __stdcall ContinueHandler( EXCEPTION_POINTERS* pExceptionInfo );
}