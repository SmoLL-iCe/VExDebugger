#pragma once
#include <windows.h>

namespace PGEMgr
{
	long __stdcall CheckPageGuardExceptions( EXCEPTION_POINTERS* pExceptionInfo );
}