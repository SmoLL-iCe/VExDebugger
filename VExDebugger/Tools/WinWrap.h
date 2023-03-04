#pragma once
#include <Windows.h>
#include <cstdint>
#include "ntos.h"
#include <string>
#include <map>

struct FuncInfo
{
	uint32_t SyscallId		= 0;
	uint8_t* OriginalPtr	= nullptr;
	uint8_t* UsePtr			= nullptr;
};

namespace WinWrap 
{
	bool Init( );

	ULONG GetErrorStatus( );

	std::map<std::string, FuncInfo*>& GetWindowsFuncInfo( );

	HMODULE GetModuleBase( const WCHAR* FileName );

	ACCESS_MASK IsValidHandle( HANDLE Handle );

	HANDLE OpenThread( ACCESS_MASK DesiredAccess, uintptr_t ThreadId );

	NTSTATUS Continue( PCONTEXT ContextRecord, BOOLEAN TestAlert );

	bool GetContextThread( HANDLE hThread, PCONTEXT pContext );

	bool SetContextThread( HANDLE hThread, PCONTEXT pContext );

	uint32_t SuspendThread( HANDLE hThread );

	uint32_t ResumeThread( HANDLE hThread );

	uint32_t QuerySystemInformation( SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength );

	bool ProtectMemory( PVOID BaseAddress, SIZE_T RegionSize, ULONG NewProtect, PULONG OldProtect );

	void* AllocMemory( PVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect );

	bool QueryMemory( PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength );
};
