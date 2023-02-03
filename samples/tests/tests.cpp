#include <windows.h>
#include <iostream>


#define NOINTERFACE
//typedef enum _MEMORY_INFORMATION_CLASS {
//	MemoryBasicInformation,
//	MemoryWorkingSetList,
//} MEMORY_INFORMATION_CLASS;
//
////

typedef union _PSAPI_WORKING_SET_BLOCK {
	ULONG Flags;
	struct {
		ULONG Protection : 5;
		ULONG ShareCount : 3;
		ULONG Shared : 1;
		ULONG Reserved : 3;
		ULONG VirtualPage : 20;
	};
} PSAPI_WORKING_SET_BLOCK, * PPSAPI_WORKING_SET_BLOCK;

typedef struct _MEMORY_WORKING_SET_LIST
{
	ULONG NumberOfPages;
	PSAPI_WORKING_SET_BLOCK WorkingSetList[ 1 ];
} MEMORY_WORKING_SET_LIST, * PMEMORY_WORKING_SET_LIST;



bool IsDebugged( )
{
#ifndef _WIN64
	NTSTATUS status;
	PBYTE pMem = nullptr;
	DWORD dwMemSize = 0;

	do
	{
		dwMemSize += 0x1000;
		pMem = (PBYTE)_malloca( dwMemSize );
		if ( !pMem )
			return false;

		memset( pMem, 0, dwMemSize );
		status = NtQueryVirtualMemory(
			GetCurrentProcess( ),
			NULL,
			MemoryWorkingSetInformation,
			pMem,
			dwMemSize,
			NULL );
	} while ( status == STATUS_INFO_LENGTH_MISMATCH );

	PMEMORY_WORKING_SET_LIST pWorkingSet = (PMEMORY_WORKING_SET_LIST)pMem;
	for ( ULONG i = 0; i < pWorkingSet->NumberOfPages; i++ )
	{
		DWORD dwAddr = pWorkingSet->WorkingSetList[ i ].VirtualPage << 0x0C;
		DWORD dwEIP = 0;
		__asm
		{
			push eax
			call $ + 5
			pop eax
			mov dwEIP, eax
			pop eax
		}

		if ( dwAddr == ( dwEIP & 0xFFFFF000 ) )
			return ( pWorkingSet->WorkingSetList[ i ].Shared == 0 ) || ( pWorkingSet->WorkingSetList[ i ].ShareCount == 0 );
	}
#endif // _WIN64
	return false;
}

int random( int min, int max ) //range : [min, max]
{
	static bool first = true;
	if ( first )
	{
		srand( time( NULL ) ); //seeding for the first time only!
		first = false;
	}
	return min + rand( ) % ( ( max + 1 ) - min );
}


#define log_file printf

inline void DisplayContextLogs( PCONTEXT ContextRecord, PEXCEPTION_RECORD pExceptionRecord )
{
	log_file( "===> ExceptionAddress 0x%p, Code: %X\n", pExceptionRecord->ExceptionAddress, pExceptionRecord->ExceptionCode );
#ifdef _M_X64
	log_file( " r8[%I64X]\n r9[%I64X]\nr10[%I64X]\nr11[%I64X]\nr12[%I64X]\nr13[%I64X]\nr14[%I64X]\nr15[%I64X]\n"
		"rax[%I64X]\nrbx[%I64X]\nrcx[%I64X]\nrdx[%I64X]\nrbp[%I64X]\nrsi[%I64X]\nrsp[%I64X]\nrdi[%I64X]\n",
		ContextRecord->R8,
		ContextRecord->R9,
		ContextRecord->R10,
		ContextRecord->R11,
		ContextRecord->R12,
		ContextRecord->R13,
		ContextRecord->R14,
		ContextRecord->R15,
		ContextRecord->Rax,
		ContextRecord->Rbx,
		ContextRecord->Rcx,
		ContextRecord->Rdx,
		ContextRecord->Rbp,
		ContextRecord->Rsi,
		ContextRecord->Rsp,
		ContextRecord->Rdi
	);

	log_file( "\nP1Home 0x%I64X\nP2Home 0x%I64X\nP3Home 0x%I64X\nP4Home 0x%I64X\nP5Home 0x%I64X\nP6Home 0x%I64X\n",
		ContextRecord->P1Home,
		ContextRecord->P2Home,
		ContextRecord->P3Home,
		ContextRecord->P4Home,
		ContextRecord->P5Home,
		ContextRecord->P6Home
	);

	log_file( "\nDr0 0x%I64X\nDr1 0x%I64X\nDr2 0x%I64X\nDr3 0x%I64X\nDr6 0x%I64X\nDr7 0x%I64X\n",
		ContextRecord->Dr0,
		ContextRecord->Dr1,
		ContextRecord->Dr2,
		ContextRecord->Dr3,
		ContextRecord->Dr6,
		ContextRecord->Dr7
	);
	log_file( "\nContextFlags 0x%X\nVectorControl 0x%I64X\n",
		ContextRecord->ContextFlags,
		ContextRecord->VectorControl
	);

	log_file( "\nDebugControl 0x%I64X\nLastBranchToRip 0x%I64X\nLastBranchFromRip 0x%I64X\nLastExceptionToRip 0x%I64X\nLastExceptionFromRip 0x%I64X\n",
		ContextRecord->DebugControl,
		ContextRecord->LastBranchToRip,
		ContextRecord->LastBranchFromRip,
		ContextRecord->LastExceptionToRip,
		ContextRecord->LastExceptionFromRip
	);
	for ( uint32_t i = 0; i < pExceptionRecord->NumberParameters; ++i )
	{
		log_file( "ExceptionInformation[%d] == %I64X\n", i, pExceptionRecord->ExceptionInformation[ i ] );
	}
#else
	log_file( " Edi[%lX]\nEsi[%lX]\nEbx[%lX]\nEdx[%lX]\nEcx[%lX]"
		"\nEax[%lX]\nEbp[%lX]\nEip[%lX]\nEsp[%lX]\n",
		ContextRecord->Edi,
		ContextRecord->Esi,
		ContextRecord->Ebx,
		ContextRecord->Edx,
		ContextRecord->Ecx,
		ContextRecord->Eax,
		ContextRecord->Ebp,
		ContextRecord->Eip,
		ContextRecord->Esp
	);
	log_file( "\nDr0 0x%lX\nDr1 0x%lX\nDr2 0x%lX\nDr3 0x%lX\nDr6 0x%lX\nDr7 0x%lX\n",
		ContextRecord->Dr0,
		ContextRecord->Dr1,
		ContextRecord->Dr2,
		ContextRecord->Dr3,
		ContextRecord->Dr6,
		ContextRecord->Dr7
	);
	for ( uint32_t i = 0; i < pExceptionRecord->NumberParameters; ++i )
	{
		log_file( "ExceptionInformation[%d] == %X\n", i, pExceptionRecord->ExceptionInformation[ i ] );
	}
#endif
	log_file( "\nSegCs %lu \nSegDs %lu\nSegEs %lu\nSegFs %lu\nSegGs %lu\nSegSs %lu\nEFlags %lu\n",
		ContextRecord->SegCs,
		ContextRecord->SegDs,
		ContextRecord->SegEs,
		ContextRecord->SegFs,
		ContextRecord->SegGs,
		ContextRecord->SegSs,
		ContextRecord->EFlags
	);
	log_file( "\nContextFlags 0x%lX\n",
		ContextRecord->ContextFlags
	);
	log_file(
		"======================================================="
		"\n" );

}













auto base = reinterpret_cast<uint8_t*>( GetModuleHandle( nullptr ) ) + 0x1000;
bool imprime = false;
void __stdcall thread_test( const uintptr_t* param )
{
	//printf( "%p\n", param );
	const auto id = *param;
	while ( true )
	{
		//if (imprime)
		//	printf("hi from thread %d\n", base[0]);
		++base[ 0 ];
		++base[ 0x20 ];
		Sleep( 500 );
	}

}

void __stdcall thread_test2( const uintptr_t* param )
{
	const auto id = *param;
	while ( true )
	{

		Sleep( 1000 );
		++base[ 0 ];
		Sleep( 1000 );
		++base[ 20 ];
		Sleep( 1000 );
	}

}
#ifdef _WIN64
#define XIP Rip
#else
#define XIP Eip
#endif

/*

  HasDebugEvent: QWORD; //set by the dll (qword so the allignment isn't broken when used in 32-bit)
  HasHandledDebugEvent: QWORD; //set by the debugger
  ContinueMethod: QWORD;
  ProcessID: DWORD;
  ThreadID: DWORD;
  ThreadWatchMethod: QWORD;
  ThreadWatchMethodConfig: QWORD; //each bit contains an boolean option (for threadpoll, the only one implemented, bit 0 means simulate thread create contexts)
  VEHVersion: DWORD; //0xCECE####  , set by the DLL
  HeartBeat: DWORD; //value that constantly changes as long as CE is alive

  NoBreakListSize: QWORD;   //number of entries in the nobreaklist
  NoBreakList: array [0..63] of QWORD;
*/

#pragma pack(push, 1)
struct TVEHDebugSharedMem
{
	uint8_t CurrentContext[ 0x2000 ];
	uint64_t HasDebugEvent;
	uint64_t HasHandledDebugEvent;
	uint64_t ContinueMethod;
	uint32_t ProcessID;
	uint32_t ThreadID;
	uint64_t ThreadWatchMethod;
	uint64_t ThreadWatchMethodConfig;
	uint32_t VEHVersion;
	uint32_t HeartBeat;
	uint64_t NoBreakListSize;
	uint64_t NoBreakList[ 64 ];

	union
	{
		EXCEPTION_RECORD64 Exception64;
		EXCEPTION_RECORD32 Exception32;
	};
};
#pragma pack(pop)

void* allocMem = nullptr;

long __stdcall internal_handler( EXCEPTION_POINTERS* pExceptionInfo )
{

	//printf( "pExceptionInfo->ExceptionRecord->ExceptionCode %X\n", pExceptionInfo->ExceptionRecord->ExceptionCode );
	DisplayContextLogs( pExceptionInfo->ContextRecord, pExceptionInfo->ExceptionRecord );

	//if ( pExceptionInfo->ExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C )
	//{ 
	//	if ( pExceptionInfo->ExceptionRecord->NumberParameters == 2 &&
	//		pExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ] > 0 )
	//	{
	//		auto Lenght = pExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ];
	//		auto str = (char*)pExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];
	//		// Cheat engine
	//		printf( "Strings: %s\n", str );
	//	}
	//}


	return EXCEPTION_CONTINUE_EXECUTION;

	MessageBoxA( 0, "a", "a", 0 );

	//STATUS_ACCESS_VIOLATION


	MessageBoxA( 0, "a", "a", 0 );



	//if ( pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION )
	//{

	//	return EXCEPTION_CONTINUE_EXECUTION;
	//}


	//



	if ( pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION ) //We will catch PAGE_GUARD Violation
	{
		//printf( "load veh called\n" );

		//if ( pExceptionInfo->ContextRecord->XIP == (uintptr_t)og_fun ) //Make sure we are at the address we want within the page
		//{
		//	pExceptionInfo->ContextRecord->XIP = (uintptr_t)hk_fun; //Modify EIP/RIP to where we want to jump to instead of the original function
		//}

		//E->ContextRecord->EFlags|=(1<<16)

		pExceptionInfo->ContextRecord->EFlags |= 0x100; //Will trigger an STATUS_SINGLE_STEP exception right after the next instruction get executed. In short, we come right back into this exception handler 1 instruction later


		DWORD dwOld;
		VirtualProtect( allocMem, 0x1000, PAGE_EXECUTE_READWRITE, &dwOld );
		return EXCEPTION_CONTINUE_EXECUTION; //Continue to next instruction
	}

	if ( pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP ) //We will also catch STATUS_SINGLE_STEP, meaning we just had a PAGE_GUARD violation
	{
		DWORD dwOld;
		VirtualProtect( allocMem, 0x1000, PAGE_READONLY, &dwOld ); //Reapply the PAGE_GUARD flag because everytime it is triggered, it get removes

		return EXCEPTION_CONTINUE_EXECUTION; //Continue the next instruction
	}

	return EXCEPTION_CONTINUE_SEARCH;

	MessageBoxA( 0, "a", "a", 0 );

	return EXCEPTION_CONTINUE_EXECUTION;
}

//https://mark.rxmsolutions.com/category/c/
#pragma optimize( "", off )
void f1( void )
{
	volatile int a = 2 + 2;
	MessageBoxA( 0, "f1", "f1", 0 );
}

void f2( )
{
	volatile int a = 2 + 2;
	MessageBoxA( 0, "f2", "f2", 0 );
}
#pragma optimize( "", on )


void hit( uintptr_t addres )
{
	auto* mod = LoadLibrary( L"NoInterfaceModule.dll" );
	bool( __stdcall * Init )( int Type, bool SpoofHwbkp, bool Logs ) = (decltype( Init ))GetProcAddress( mod, "Init" );
	bool( __stdcall * StartMonitorAddress )( uintptr_t Address, int Type, int Size ) = (decltype( StartMonitorAddress ))GetProcAddress( mod, "StartMonitorAddress" );

	Init( 0, false, true );

	//StartMonitorAddress( (uintptr_t)f1, 0, 0 );

	StartMonitorAddress( addres + 0x20, 1, 2 );
	StartMonitorAddress( addres + 0x40, 1, 2 );
	StartMonitorAddress( addres, 0, 0 );
}

int main( )
{

	auto rval = random( 25, 500 );

	allocMem = VirtualAlloc( 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

	printf( "allocMem: %p\n", allocMem );

	auto point = (uint64_t)allocMem + rval;

	printf( "point: %p\n", point );
	*(uint64_t*)( point ) = 0x9090C300001337B8;


	DWORD dwOld;

	//VirtualProtect( allocMem, 0x1000, PAGE_EXECUTE_READ | PAGE_GUARD, &dwOld );

	//VirtualProtect( allocMem, 0x1000, PAGE_READONLY, &dwOld );

	//allocMem = VirtualAlloc( 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );


	printf( "f1 func: %p\n", f1 );


	//AddVectoredExceptionHandler( 1, internal_handler );




#ifdef NOINTERFACE

	CreateThread( nullptr, 0, (LPTHREAD_START_ROUTINE)hit, (void*)f1, 0, nullptr );

	Sleep( 2000 );

#else
#ifdef _WIN64
	auto* mod = LoadLibrary( L"VExDebug_ImGui_x64.dll" );
#else

	auto* mod = LoadLibrary( L"VExDebug_ImGui_x86.dll" );
#endif
#endif

	f1( );
	getchar( );

	//{ // Execute

	//	auto func = reinterpret_cast<uint32_t( __fastcall* )( int )>( point );

	//	auto v = func( 123 );

	//	printf( "v: %X\n", v );

	//	getchar( );
	//}

	for ( size_t i = 0; i < 5; i++ )
	{

		{ // Read

			auto val = *(uint8_t*)( point );

			printf( "val: %X\n", val );
		}

		//{ // Write

		//	*(uint8_t*)( point ) = 123;
		//}

		//RaiseException( EXCEPTION_SINGLE_STEP, 123, 0, nullptr );



		{ // Read

			auto val = *(uintptr_t*)( point );

			printf( "val: %X\n", val );
		}
	}
	return getchar( );
}
