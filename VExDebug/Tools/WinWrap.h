#pragma once

#include <Windows.h>
#include <cstdint>

namespace WinWrap 
{
	bool Init( );

	ULONG GetErrorStatus( );

	ACCESS_MASK IsValidHandle( HANDLE Handle );

	HANDLE OpenThread( ACCESS_MASK DesiredAccess, uintptr_t ThreadId );

	bool GetContextThread( HANDLE hThread, PCONTEXT pContext );

	bool SetContextThread( HANDLE hThread, PCONTEXT pContext );

	uint32_t SuspendThread( HANDLE hThread );

	uint32_t ResumeThread( HANDLE hThread );
};
