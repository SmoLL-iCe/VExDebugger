
#include "WinWrap.h"
#include "Logs.h"
#include "../Headers/Header.h"

#define GET_FUNC(name) i##name.OriginalPtr	= GetExportAddress( ntdll, #name ); \
	if ( !i##name.OriginalPtr ) { \
		log_file( "[-] Failed to get export function [%s]\n", #name ); \
		return false; \
	}else{ \
		i##name.UsePtr = i##name.OriginalPtr; \
		i##name.SyscallId = getSysCallId( i##name.OriginalPtr ); \
		if ( i##name.SyscallId ) \
			i##name.UsePtr = (uint8_t*)SysCallAsm; \
		else \
			log_file( "[-] Failed to get syscall id [%s]\n", #name ); \
		ListWindowFunc[ #name ] = &i##name; \
	}

#ifdef _WIN64
using WinSysCall = NTSTATUS( __stdcall* )( ... );
#define THE_CALL( ptr, ...) r_cast<WinSysCall>( ptr )( __VA_ARGS__ )
#else
using WinSysCall = NTSTATUS( __cdecl* )( ... );
#define THE_CALL( ptr, ...) r_cast<WinSysCall>( ptr )( 0, __VA_ARGS__ )
#endif

BYTE* GetExportAddress( HMODULE hModule, const char* exportName )
{
	auto pModule = (BYTE*)hModule;
	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
	IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)( pModule + dosHeader->e_lfanew );

	IMAGE_DATA_DIRECTORY* exportDirectory = &ntHeaders->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];
	IMAGE_EXPORT_DIRECTORY* exportDirectoryData = (IMAGE_EXPORT_DIRECTORY*)( pModule + exportDirectory->VirtualAddress );

	DWORD* nameTable = (DWORD*)( pModule + exportDirectoryData->AddressOfNames );
	WORD* ordinalTable = (WORD*)( pModule + exportDirectoryData->AddressOfNameOrdinals );
	DWORD* functionTable = (DWORD*)( pModule + exportDirectoryData->AddressOfFunctions );

	for ( DWORD i = 0; i < exportDirectoryData->NumberOfNames; i++ ) {
		const char* name = (const char*)( pModule + nameTable[ i ] );

		if ( _stricmp( name, exportName ) == 0 ) {
			WORD ordinal = ordinalTable[ i ];
			DWORD functionAddress = functionTable[ ordinal ];
			auto exportAddress = pModule + functionAddress;
			return exportAddress;
		}
	}

	return nullptr;
}

uint32_t getSysCallId( void* Func )
{
	auto pFunc = reinterpret_cast<uint8_t*>( Func );

	if ( !pFunc )
		return 0;

#ifdef _WIN64

	if ( *reinterpret_cast<uint32_t*>( &pFunc[ 0 ] ) == 0xB8D18B4C )
	{
		return *reinterpret_cast<uint32_t*>( &pFunc[ 4 ] );
	}
	else
	{
		auto Point = pFunc - 0x30;

		for ( size_t i = 0; i < 0x30; i++ )
		{
			if ( *reinterpret_cast<uint32_t*>( &Point[ i ] ) == 0xB8D18B4C )
				return ( *reinterpret_cast<uint32_t*>( &Point[ i + 1 ] ) ) + 1;
			
		}
	}

#else

	if ( pFunc[ 0 ] == 0xB8 && pFunc[ 5 ] == 0xBA )
	{
		return *reinterpret_cast<uint32_t*>( &pFunc[ 1 ] );
	}
	else
	{
		auto Point = pFunc - 0x20;

		for ( size_t i = 0; i < 0x20; i++ )
		{
			if ( Point[ i ] == 0xB8 && Point[ i + 5 ] == 0xBA )
				return ( *reinterpret_cast<uint32_t*>( &Point[ i + 1 ] ) ) + 1;
		}
	}

#endif
	return 0;
}

uintptr_t NtGetCurrentTEB( )
{
#ifdef _M_IX86
	return (uintptr_t)__readfsdword( 0x18 );
#elif _M_AMD64
	return (uintptr_t)__readgsqword( 0x30 );
#endif
}

void SetLastErrorValue( DWORD code )
{
	auto TEB = NtGetCurrentTEB( );

// LastErrorValue Teb Offsets
#ifdef _M_IX86
	* reinterpret_cast<uint32_t*>( TEB + 0x34 ) = code;
#elif _M_AMD64
	* reinterpret_cast<uint32_t*>( TEB + 0x68 ) = code;
#endif

// ExceptionCode Teb Offsets
//#ifdef _M_IX86
//	* reinterpret_cast<uint32_t*>( TEB + 0x1a4 ) = code;
//#elif _M_AMD64
//	* reinterpret_cast<uint32_t*>( TEB + 0x2c0 ) = code;
//#endif
}

HMODULE WinWrap::GetModuleBase( const WCHAR* FileName )
{
	UNICODE_STRING ModName{};

	RtlInitUnicodeString( &ModName, FileName );

	void* ModHandle = nullptr;

	LdrGetDllHandle( NULL, NULL, &ModName, &ModHandle );

	return (HMODULE)ModHandle;
}

#ifdef _WIN64

EXTERN_C
uint8_t* pAnySyscall = nullptr;

EXTERN_C
void SysCallAsm( );

#else

uint8_t* pWow64Transition = nullptr;

__declspec( naked ) int SysCallAsm( )
{
	_asm {
		mov eax, fs:[0x34]
		jmp dword ptr[ pWow64Transition ]
	}
}

//34h LastErrorValue offset 32 bits
//1a4h ExceptionCode offset 32 bits
#endif

FuncInfo iNtAllocateVirtualMemory	= {};
FuncInfo iNtProtectVirtualMemory	= {};
FuncInfo iNtQueryVirtualMemory		= {};
FuncInfo iNtQuerySystemInformation	= {};
FuncInfo iNtGetContextThread		= {};
FuncInfo iNtSetContextThread		= {};
FuncInfo iNtOpenThread				= {};
FuncInfo iNtSuspendThread			= {};
FuncInfo iNtResumeThread			= {};
FuncInfo iNtQueryObject				= {};
FuncInfo iNtContinue				= {};
NTSTATUS LastStatus					= 0;

std::map<std::string, FuncInfo*> ListWindowFunc{};
std::map<std::string, FuncInfo*>& WinWrap::GetWindowsFuncInfo( )
{
	return ListWindowFunc;
}

#define REREF( name ) i##name = ListWindowFunc[ #name ]

bool WinWrap::Init( )
{
	auto ntdll = GetModuleBase( L"ntdll.dll" );

	GET_FUNC( NtAllocateVirtualMemory )
	GET_FUNC( NtProtectVirtualMemory )
	GET_FUNC( NtQueryVirtualMemory )
	GET_FUNC( NtQuerySystemInformation )
	GET_FUNC( NtGetContextThread )
	GET_FUNC( NtSetContextThread )
	GET_FUNC( NtOpenThread )
	GET_FUNC( NtSuspendThread )
	GET_FUNC( NtResumeThread )
	GET_FUNC( NtQueryObject )
	GET_FUNC( NtContinue )

#ifdef _WIN64

	auto pAnyFunc = GetExportAddress( ntdll, "NtDeleteFile" );

	if ( !pAnyFunc )
	{
		log_file( "[-] Failed to get export function NtDeleteFile\n" );
		return false;
	}

	for ( size_t i = 0; i < 0x100; i++ )
	{
		if ( *reinterpret_cast<uint16_t*>( &pAnyFunc[ i ] ) == 0x050F )
		{
			pAnySyscall = &pAnyFunc[ i ];
			break;
		}
	}

	if ( !pAnySyscall )
	{
		log_file( "[-] Failed to get AnyInstrution Syscall\n" );
		return false;
	}

#else

	auto Wow64Transition = GetExportAddress( ntdll, "Wow64Transition" );

	if ( !Wow64Transition )
	{
		log_file( "[-] Failed to get export function Wow64Transition\n" );
		return false;
	}

	pWow64Transition = *reinterpret_cast<uint8_t**>( Wow64Transition );

#endif
	
	return true;
}

ACCESS_MASK WinWrap::IsValidHandle( HANDLE Handle )
{
	const auto bRet = ( Handle != nullptr && Handle != INVALID_HANDLE_VALUE );

	if ( bRet )
	{
		OBJECT_BASIC_INFORMATION ObjInfo = { 0 };

		DWORD RetLen = 0;

		SetLastErrorValue( iNtQueryObject.SyscallId );

		LastStatus = THE_CALL( iNtQueryObject.UsePtr, Handle, ObjectBasicInformation, &ObjInfo, sizeof( OBJECT_BASIC_INFORMATION ), &RetLen );

		if ( LastStatus == 0 )
			return ObjInfo.GrantedAccess;

		SetLastErrorValue( 0 );
	}

	return 0;
}

NTSTATUS WinWrap::Continue( PCONTEXT ContextRecord, BOOLEAN TestAlert )
{
	SetLastErrorValue( iNtContinue.SyscallId );

	LastStatus = THE_CALL( iNtContinue.UsePtr, ContextRecord, TestAlert );

	SetLastErrorValue( 0 );

	return LastStatus;
}

HANDLE WinWrap::OpenThread( ACCESS_MASK DesiredAccess, uintptr_t ThreadId )
{
	HANDLE hThread = nullptr;

	CLIENT_ID cPid = { 
		nullptr, 
		r_cast<HANDLE>( ThreadId ) 
	};

	OBJECT_ATTRIBUTES ObjAtt = { 
		sizeof OBJECT_ATTRIBUTES 
	};

	SetLastErrorValue( iNtOpenThread.SyscallId );

	LastStatus = THE_CALL( iNtOpenThread.UsePtr, &hThread, DesiredAccess, &ObjAtt, &cPid );

	SetLastErrorValue( 0 );

	return hThread;
}

bool WinWrap::GetContextThread( HANDLE hThread, PCONTEXT pContext )
{
	SetLastErrorValue( iNtGetContextThread.SyscallId );

	LastStatus = THE_CALL( iNtGetContextThread.UsePtr, hThread, pContext );

	SetLastErrorValue( 0 );

	return NT_SUCCESS( LastStatus );
}

ULONG WinWrap::GetErrorStatus( )
{
	return LastStatus;
}

bool WinWrap::SetContextThread( HANDLE hThread, PCONTEXT pContext )
{
	SetLastErrorValue( iNtSetContextThread.SyscallId );

	LastStatus = THE_CALL( iNtSetContextThread.UsePtr, hThread, pContext );

	SetLastErrorValue( 0 );

	return NT_SUCCESS( LastStatus );
}

uint32_t WinWrap::SuspendThread( HANDLE hThread )
{
	ULONG SuspendCount = -1;

	SetLastErrorValue( iNtSuspendThread.SyscallId );

	LastStatus = THE_CALL( iNtSuspendThread.UsePtr, hThread, &SuspendCount );

	SetLastErrorValue( 0 );

	return SuspendCount;
}

uint32_t WinWrap::ResumeThread( HANDLE hThread )
{
	ULONG SuspendCount = -1;

	SetLastErrorValue( iNtResumeThread.SyscallId );

	LastStatus = THE_CALL( iNtResumeThread.UsePtr, hThread, &SuspendCount );

	SetLastErrorValue( 0 );

	return SuspendCount;
}

bool WinWrap::QueryMemory( PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength )
{
	SetLastErrorValue( iNtQueryVirtualMemory.SyscallId );

	LastStatus = THE_CALL( iNtQueryVirtualMemory.UsePtr,
			(HANDLE)-1, BaseAddress, MemoryInformationClass, MemoryInformation,
			MemoryInformationLength, ReturnLength );

	SetLastErrorValue( 0 );

	return NT_SUCCESS( LastStatus );
}

bool WinWrap::ProtectMemory( PVOID BaseAddress, SIZE_T RegionSize, ULONG NewProtect, PULONG OldProtect )
{
	SetLastErrorValue( iNtProtectVirtualMemory.SyscallId );

	LastStatus = THE_CALL( iNtProtectVirtualMemory.UsePtr, (HANDLE)-1, &BaseAddress, &RegionSize, NewProtect, OldProtect );

	SetLastErrorValue( 0 );

	return NT_SUCCESS( LastStatus );
}

void* WinWrap::AllocMemory( PVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect )
{
	SetLastErrorValue( iNtAllocateVirtualMemory.SyscallId );

	PVOID	BaseAddress = lpAddress;

	auto	RegionSize	= dwSize;

	LastStatus = THE_CALL( iNtAllocateVirtualMemory.UsePtr, (HANDLE)-1, &BaseAddress, ULONG_PTR(0), &RegionSize, flAllocationType, flProtect );

	SetLastErrorValue( 0 );

	return BaseAddress;
}

uint32_t WinWrap::QuerySystemInformation( SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength )
{
	SetLastErrorValue( iNtQuerySystemInformation.SyscallId );

	LastStatus = THE_CALL( iNtQuerySystemInformation.UsePtr,
		SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength );

	SetLastErrorValue( 0 );

	return LastStatus;
}