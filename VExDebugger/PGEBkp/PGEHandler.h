#pragma once
#include <windows.h>

namespace MgrPGE
{
	long __stdcall CheckPageGuardExceptions( EXCEPTION_POINTERS* pExceptionInfo );
}