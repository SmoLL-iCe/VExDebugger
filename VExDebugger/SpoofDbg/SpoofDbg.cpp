#include "SpoofDbg.h"
#include "../Tools/WinWrap.h"
#include "DoHook.h"
#include "../HwBkp/MgrHwBkp.h"

bool	isHookedNtGetContextThread	= false;
bool	isHookedNtContinue			= false;
void*	oNtGetContextThread			= nullptr;
void*	oNtContinue					= nullptr;

NTSTATUS NTAPI hkNtGetContextThread( HANDLE ThreadHandle, PCONTEXT ThreadContext )
{
	auto Status = reinterpret_cast<decltype( hkNtGetContextThread )*>( oNtGetContextThread )( ThreadHandle, ThreadContext );

	if ( Status == 0 )
	{
	}
		
	return 0;
}

NTSTATUS NTAPI hkNtContinue( PCONTEXT ContextRecord, BOOLEAN TestAlert )
{
	if ( ContextRecord )
	{

	}
	return reinterpret_cast<decltype( hkNtContinue )*>( oNtGetContextThread )( ContextRecord, TestAlert );
}

bool SpoofDbg::HookNtGetContextThread( )
{

	if ( isHookedNtGetContextThread )
		return true;

	auto FuncInfo	= WinWrap::GetWindowsFuncInfo( )[ "NtGetContextThread" ];

	if ( !FuncInfo->SyscallId )
		return false;

	isHookedNtGetContextThread = DoHook::SetInlineHook( FuncInfo->OriginalPtr, hkNtGetContextThread, &oNtGetContextThread,
#ifdef _WIN64
		8
#else
		5
#endif
		);
	return isHookedNtGetContextThread;
}

bool SpoofDbg::HookNtContinue( )
{

	if ( isHookedNtContinue )
		return true;

	auto FuncInfo	= WinWrap::GetWindowsFuncInfo( )[ "NtContinue" ];

	if ( !FuncInfo->SyscallId )
		return false;

	isHookedNtContinue = DoHook::SetInlineHook( FuncInfo->OriginalPtr, hkNtContinue, &oNtContinue,
#ifdef _WIN64
		8
#else
		5
#endif
		);
	return isHookedNtContinue;
}