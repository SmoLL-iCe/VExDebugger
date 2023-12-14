#include <iostream>
#include <windows.h>
#include <VExDebugger.h>
#include <iomanip>
#include <TlHelp32.h>
#include <psapi.h>
#include "color.hpp"
#include "../../VExDebugger/Tools/ntos.h"
#include "shell.hpp"

#define MM "MD"
#ifdef _MT
#undef MM
#define MM "MT"
#endif

#define ARCH "86"
#ifdef _WIN64
#undef ARCH
#define ARCH "64"
#endif

#define RD ""
#ifdef _DEBUG
#undef RD
#define RD "d"
#endif

#define IMPORT_LIB "x" ARCH "\\VExDebugger" MM RD ".lib"

#pragma comment(lib, IMPORT_LIB)


#define READ_TEST
#define WRITE_TEST

#define READ_MOD_TEST
#define WRITE_MOD_TEST

//#define LOAD_MODULE

int random( int min, int max )
{
	static bool first = true;
	if ( first )
	{
		srand( (uint32_t)time( nullptr ) );
		first = false;
	}
	return min + rand( ) % ( ( max + 1 ) - min );
}

MODULEENTRY32 GetModuleInfo( uintptr_t Address ) 
{
	DWORD ProcessID = GetCurrentProcessId( );

	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, ProcessID );

	if ( hSnapshot == INVALID_HANDLE_VALUE ) 
		return {};
	
	MODULEENTRY32 module{};

	module.dwSize = sizeof( MODULEENTRY32 );

	if ( Module32First( hSnapshot, &module ) ) 
	{
		do 
		{
			const auto BaseAddress	= reinterpret_cast<uintptr_t>( module.modBaseAddr );

			const auto EndBase		= BaseAddress + module.modBaseSize;
			
			if ( Address >= BaseAddress && Address <= EndBase )
			{
				CloseHandle( hSnapshot );

				return module;
			}

		} while ( Module32Next( hSnapshot, &module ) );
	}

	CloseHandle( hSnapshot );

	return { };
}

PIMAGE_SECTION_HEADER GetSectionByName( void* module, const char* name )
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)module;

	if ( dosHeader->e_magic == IMAGE_DOS_SIGNATURE )
	{
		PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)( (uintptr_t)module + dosHeader->e_lfanew );

		if ( ntHeader->Signature == LOWORD( IMAGE_NT_SIGNATURE ) )
		{
			PIMAGE_SECTION_HEADER secHdr = IMAGE_FIRST_SECTION( ntHeader );

			UINT sections = ntHeader->FileHeader.NumberOfSections;

			for ( UINT i = 0; i < sections; i++, secHdr++ )
			{
				if ( strncmp( name, (LPCSTR)secHdr->Name, IMAGE_SIZEOF_SHORT_NAME ) == 0 )
					return secHdr;
				
			}
		}
	}
	return nullptr;
}

std::vector<uint8_t*> GetCodeCaves( )
{
	std::vector<uint8_t*> listPtr{};

	DWORD ProcessID = GetCurrentProcessId( );

	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, ProcessID );

	if ( hSnapshot == INVALID_HANDLE_VALUE ) 
		return {};
	
	MODULEENTRY32 module{};

	module.dwSize = sizeof( MODULEENTRY32 );

	if ( Module32First( hSnapshot, &module ) ) 
	{
		do 
		{
			if ( module.modBaseAddr != (void*)GetModuleHandle( nullptr ) )
			{
				const auto BaseAddress	= reinterpret_cast<uintptr_t>( module.modBaseAddr );

				const auto pSection		= GetSectionByName( module.modBaseAddr, ".text" );

				if ( pSection )
				{
					int Total = 0;

					auto InitSecAddress = (uint8_t*)BaseAddress		+ pSection->VirtualAddress;

					for ( size_t i = 0; i < pSection->Misc.VirtualSize - 4; i++ )
					{
						if ( Total > 5 )
							break;

						if ( *reinterpret_cast<uint64_t*>( &InitSecAddress[ i ] ) == 0xCCCCCCCCCCCCCCCC )
						{
							++Total;
							listPtr.push_back( &InitSecAddress[ i ] );

							i += 0x50;

						}
					}

				}
			}

		} while ( Module32Next( hSnapshot, &module ) );
	}

	CloseHandle( hSnapshot );

	return listPtr;
}

uint8_t* AllocMemPage		= 0;
uint8_t* RelativePointPtr	= 0;
auto pFuncExecHit			= reinterpret_cast<uint8_t( __fastcall* )( uint8_t* )>( 0 );

void CreatePageTests( )
{
	auto RandomOffset = random( 0x10, 0x200 );

	AllocMemPage = reinterpret_cast<uint8_t*>( VirtualAlloc( 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );

	if ( !AllocMemPage )
	{
		std::cout << "insufficient mem res\n";
		return;
	}

	RelativePointPtr = AllocMemPage + RandomOffset;

	*reinterpret_cast<uint64_t*>( RelativePointPtr ) = 0x9090C300001337B8;

	*(uint32_t*)( RelativePointPtr + 0x300 ) = 0x90C3018A;

	pFuncExecHit = decltype( pFuncExecHit )( RelativePointPtr + 0x300 );

	printf( "AllocMemPage: 0x%p\n", AllocMemPage );

	printf( "RelativePointPtr: 0x%p\n", RelativePointPtr );

	printf( "ExecHit: 0x%p\n", pFuncExecHit );

}

void SetBreakPointsFunc( )
{
	VExDebugger::Init( HandlerType::VectoredExceptionHandler, true );

	//VExDebugger::StartMonitorAddress( RelativePointPtr + 0x100, BkpMethod::Hardware, BkpTrigger::Write, BkpSize::Size_2 );

	//auto Result = PGEMgr::AddPageExceptions( uintptr_t( RelativePointPtr + 0x100 ), BkpMethod::Hardware, BkpTrigger::ReadWrite, BkpSize::Size_4 );

	VExDebugger::StartMonitorAddress( RelativePointPtr + 0x200, BkpMethod::Hardware, BkpTrigger::ReadWrite, BkpSize::Size_1 );

	//VExDebugger::StartMonitorAddress( RelativePointPtr + 0x300, BkpMethod::Hardware, BkpTrigger::Execute, BkpSize::Size_1 );

	//VExDebugger::StartMonitorAddress( RelativePointPtr, BkpMethod::Hardware, BkpTrigger::ReadWrite, BkpSize::Size_1 );


	//VExDebugger::SetTracerAddress( 
	//	RelativePointPtr + 0x300, 
	//	BkpTrigger::Execute, 
	//	BkpSize::Size_1,
	//[ ]( PEXCEPTION_RECORD pExceptRec, PCONTEXT pContext ) -> CBReturn 
	//	{
	//		static int count = 0;
	//		auto Module = GetModuleInfo( (uintptr_t)pExceptRec->ExceptionAddress );

	//		if ( !Module.modBaseAddr )
	//		{
	//			std::cout << count << " ExceptionAddress: 0x" << std::hex << std::uppercase << (uintptr_t)pExceptRec->ExceptionAddress << "\n";
	//		}
	//		else
	//		{
	//			auto Offset = (uintptr_t)pExceptRec->ExceptionAddress - reinterpret_cast<uintptr_t>( Module.modBaseAddr );

	//			std::cout << count << " ExceptionAddress: ";

	//			std::wcout << Module.szModule;

	//			std::cout << "+" << std::hex << std::uppercase << Offset << "\n";
	//		}
	//		++count;


	//		return ( count >= 8 ) ? CBReturn::StopTrace : CBReturn::StepOver;

	//	}
	//);

}

void TestMod( )
{
#if defined( READ_MOD_TEST ) || defined( WRITE_MOD_TEST )


	// read/write hits test
	{

		auto CodeCaves = GetCodeCaves( );

		std::cout << "CodeCavesSize: " << CodeCaves.size( ) << std::endl;

		auto InstruRead = 0x90C3018A;
		auto InstruWrite = 0x90C31188;

		for ( auto & Address : CodeCaves )
		{
			DWORD p = 0;
			if ( !VirtualProtect( Address, sizeof( Address ), PAGE_EXECUTE_READWRITE, &p ) )
				continue;

			*reinterpret_cast<uint32_t*>( Address )		= InstruRead;

			*reinterpret_cast<uint32_t*>( Address + 4 ) = InstruWrite;

			auto pFuncReadHit	= reinterpret_cast<uint8_t( __fastcall* )( uint8_t* )>( Address );

			auto pFuncWriteHit	= reinterpret_cast<uint8_t( __fastcall* )( uint8_t*, uint8_t )>( Address + 0x4 );

			auto rCount = random( 1, 200 );

			//printf( "Count Read %d\n", rCount );
#if defined( READ_MOD_TEST )
			for ( size_t i = 0; i < rCount; i++ )
				pFuncReadHit( RelativePointPtr );
#endif

#if defined( WRITE_MOD_TEST )
			rCount = random( 1, 20 );
			//printf( "Count Write %d\n", rCount );
			for ( size_t i = 0; i < rCount; i++ )
				pFuncWriteHit( RelativePointPtr + ( 0x100 * random( 0, 3 ) ), random( 0, 255 ) );
#endif

			VirtualProtect( Address, sizeof( Address ), p, &p );
		}
	}

#endif
}

void TestPage( )
{
#if defined( READ_TEST ) || defined( WRITE_TEST )


	// read/write hits test
	{
		auto AnotherMemPage = reinterpret_cast<uint8_t*>( VirtualAlloc( 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );

		if ( AnotherMemPage )
		{
			auto InstruRead = 0x90C3018A;
			auto InstruWrite = 0x90C31188;

			for ( size_t i = 0; i < 10; i++ )
			{
				auto pPtr = AnotherMemPage + 0x10 * i;

				*reinterpret_cast<uint32_t*>( pPtr ) = InstruRead;
				*reinterpret_cast<uint32_t*>( pPtr + 4 ) = InstruWrite;
				auto pFuncReadHit = reinterpret_cast<uint8_t( __fastcall* )( uint8_t* )>( pPtr );
				auto pFuncWriteHit = reinterpret_cast<uint8_t( __fastcall* )( uint8_t*, uint8_t )>( pPtr + 0x4 );
				auto rCount = random( 1, 200 );
#if defined( READ_TEST )
				for ( size_t i = 0; i < rCount; i++ )
					pFuncReadHit( RelativePointPtr );
#endif

#if defined( WRITE_TEST )
				rCount = random( 1, 20 );
				for ( size_t i = 0; i < rCount; i++ )
					pFuncWriteHit( RelativePointPtr, random( 0, 255 ) );
#endif
			}

			VirtualFree( AnotherMemPage, 0, MEM_RELEASE );
		}
	}

#endif
}

void TestExec( )
{
	uintptr_t val = 12;
	for ( size_t i = 0; i < 1; i++ )
	{

		auto t = pFuncExecHit( (uint8_t*)&val );

		printf( "ret: %02X\n", t );
	}
}

//VOID
//NTAPI
//RtlRaiseException(
//	_In_ PEXCEPTION_RECORD ExceptionRecord );
//
//
//VOID
//NTAPI
//RtlRaiseStatus(
//	_In_ NTSTATUS Status );
//
//
//NTSTATUS
//NTAPI
//NtRaiseException(
//	_In_ PEXCEPTION_RECORD ExceptionRecord,
//	_In_ PCONTEXT ContextRecord,
//	_In_ BOOLEAN FirstChance );

//void __stdcall mRaiseException(
//	DWORD dwExceptionCode,
//	DWORD dwExceptionFlags,
//	DWORD nNumberOfArguments,
//	const ULONG_PTR* lpArguments )
//{
//	DWORD v4; // eax
//	EXCEPTION_RECORD ExceptionRecord{}; 
//
//	HIDWORD( ExceptionRecord.ExceptionRecord ) = 0;
//	ExceptionRecord.ExceptionCode = dwExceptionCode;
//	*(QWORD*)&ExceptionRecord.ExceptionFlags = dwExceptionFlags & 1;
//	ExceptionRecord.ExceptionAddress = RaiseException;
//	if ( lpArguments )
//	{
//		v4 = 15;
//		if ( nNumberOfArguments <= 0xF )
//			v4 = nNumberOfArguments;
//		ExceptionRecord.NumberParameters = v4;
//		memcpy( ExceptionRecord.ExceptionInformation, lpArguments, 8i64 * v4 );
//	}
//	else
//	{
//		ExceptionRecord.NumberParameters = 0;
//	}
//	RtlRaiseException( &ExceptionRecord );
//}

const char* drs[]
{
	"Dr0",
	"Dr1",
	"Dr2",
	"Dr3",
	"Dr6",
	"Dr7",
};

LONG NTAPI FHandler( EXCEPTION_POINTERS* ExceptionInfo )
{
	//printf( "ExceptionCode 0x%lX\n", ExceptionInfo->ExceptionRecord->ExceptionCode );
	for ( size_t i = 0; i < 6; i++ )
	{
		auto pDrCtx		= ( &ExceptionInfo->ContextRecord->Dr0 );

		auto Ptr		= pDrCtx[ i ];

		if ( Ptr )
		{
			std::cout << color::start( "Light Red" );

			printf( "Handler, Hardware Breakpoint Detected %s, 0x%p\n", drs[ i ], Ptr );

			std::cout << color::end( );
		}

	}

	return EXCEPTION_CONTINUE_EXECUTION;
}

LONG NTAPI ContinueFHandler( EXCEPTION_POINTERS* ExceptionInfo )
{
	if ( EXCEPTION_SINGLE_STEP )
	{
	}

	for ( size_t i = 0; i < 6; i++ )
	{
		auto pDrCtx = ( &ExceptionInfo->ContextRecord->Dr0 );

		auto Ptr = pDrCtx[ i ];

		if ( Ptr )
		{
			std::cout << color::start( "Light Red" );

			printf( "Continue, Hardware Breakpoint Detected %s, 0x%p\n", drs[ i ], Ptr );


			std::cout << color::end( );
		}
	}


	return EXCEPTION_CONTINUE_EXECUTION;
}

#pragma optimize( "", off )
void the_dividi( int v )
{
	printf( "%d\n", 50 / v );
}

void stackOverflow( ) {
	stackOverflow( );
}
#pragma optimize( "", on )

void TryDetectHardwareBkp( )
{
	while ( true )
	{


		printf( "RelativePointPtr %d\n", *reinterpret_cast<uint32_t*>( RelativePointPtr + 0x100 ) );

		std::cout << "dividi\n";
		Sleep( 5000 );

		//RaiseException( EXCEPTION_BREAKPOINT, 123, 0, nullptr );

		//Sleep( 1000 );
		//RaiseException( EXCEPTION_ACCESS_VIOLATION, 0x1337, 0, nullptr );

		//Sleep( 1000 );
		//RaiseException( EXCEPTION_GUARD_PAGE, 0x1337, 0, nullptr );

		Sleep( 1000 );
		CONTEXT ctx{};

		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		auto s = GetThreadContext( (HANDLE)-2, &ctx );

		printf( "GetThreadContext %d\n", s );
		for ( size_t i = 0; i < 6; i++ )
		{
			auto pDrCtx = ( &ctx.Dr0 );

			auto Ptr = pDrCtx[ i ];

			if ( Ptr )
			{
				std::cout << color::start( "Light Red" );

				printf( "GetThreadContext, Hardware Breakpoint Detected %s, 0x%p\n", drs[ i ], Ptr );

				std::cout << color::end( );
			}
		}
	
	}

}

void testDetectHardwareBkp( )
{
	std::cout << "testDetectHardwareBkp\n";

	AddVectoredContinueHandler( 1, ContinueFHandler );

	AddVectoredExceptionHandler( 1, FHandler );

	CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( TryDetectHardwareBkp ), nullptr, 0, nullptr );

	Sleep( 1000 );

}

#pragma optimize( "", off )

int xxmain( )
{
    std::cout << "Hello World!\n";

	CreatePageTests( );

	//system( "pause" );

	//testDetectHardwareBkp( );

#ifdef LOAD_MODULE
	auto h = LoadLibrary( L"ImVExDebugger_x" ARCH ".dll" );
	//auto h = LoadLibrary( L"dotNetForms_x" ARCH ".dll" );
	printf( "H: %p\n", h );
#else
	CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( SetBreakPointsFunc ), nullptr, 0, nullptr );
#endif

	system( "pause" );

	Sleep( 1000 );

	// TryDetectHardwareBkp( );

	//TestExec( );

	//printf( "out\n");

	// TestMod( );

	//TestPage( );

#ifndef LOAD_MODULE


	RelativePointPtr[ 0x100 ] = 8;

	while ( true )
	{
		Sleep( 1000 );

		VExDebugger::CallBreakpointList( []( TBreakpointList BreakpointList ) -> void {

			for ( const auto& [Address, BpInfo] : BreakpointList )
			{
				if ( !Address )
					continue;

				//if ( BpInfo.Method != BkpMethod::Hardware )             // only support hardware breakpoint
				//	continue;

				bool Skip = false;

				VExDebugger::CallAssocExceptionList( [&]( TAssocExceptionList AssocExceptionList ) -> void {

					auto ItExceptionList = AssocExceptionList.find( Address );

					if ( ItExceptionList == AssocExceptionList.end( ) )
						return;
				

					auto& ExceptionList = ItExceptionList->second;

					std::cout << "\nIndex: " << ( BpInfo.Pos + 1 ) << ", Address: 0x" <<
						std::hex << std::uppercase << Address << "\n";


					for ( const auto& [ExceptionAddress, ExceptionInfo] : ExceptionList )
					{

						std::cout <<
							"\tCount " << color::start( "Light Blue" ) <<
							std::setfill( ' ' ) << std::setw( 8 ) << std::dec << ExceptionInfo.Details.Count <<
							color::end( );


						if ( BpInfo.Trigger != BkpTrigger::Execute )
						{
							std::cout << color::start( "Light Green" );

							auto Module = GetModuleInfo( ExceptionAddress );

							if ( !Module.modBaseAddr )
							{
								std::cout << " ExceptionAddress: 0x" << std::hex << std::uppercase << ExceptionAddress << "\n";
							}
							else
							{
								auto Offset = ExceptionAddress - reinterpret_cast<uintptr_t>( Module.modBaseAddr );

								std::cout << " ExceptionAddress: ";

								std::wcout << Module.szModule;

								std::cout << "+0x" << std::hex << std::uppercase << Offset << "\n";
							}

							std::cout << color::end( );
						}
						else
						{

							std::cout << " ThreadId: " << std::dec << color::start( "Light Green" )
								<< ExceptionAddress << color::end( ) << "\n";
						}
					}

				} );
			}
		} );
	}
#endif


    return getchar( );
}



static int ThreadsCount = 0;
static int ThreadsCountChange = 0;

int testPGE( )
{
	auto Tid        = GetCurrentThreadId( );

	auto StrTID     = std::to_string( Tid );

	for ( size_t i = 0; i < sizeof( shell_code ); i++ )
	{
		if ( *reinterpret_cast<uint64_t*>( &shell_code[ i ] ) == 0xCCCCCCCCCCCCCCCC ) // change calls to test tracer StepOver
		{
			*reinterpret_cast<uint64_t*>( &shell_code[ i ] ) = uint64_t( &Sleep );

			i += 7;
		}
	}

	uint8_t* AllocMem = nullptr;

	AllocMem = reinterpret_cast<uint8_t*>( VirtualAlloc( nullptr, 0x2000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );

	if ( !AllocMem )
	{
		printf( "[Thread: %d] Enough resources\n", Tid );
		return getchar( );
	}

	printf( "[Thread: %d] AllocMem: 0x%p\n", Tid, AllocMem );

	auto NewPoint = reinterpret_cast<uintptr_t>( AllocMem ) + ( 0x1000ull - 0x30 );

	memcpy( reinterpret_cast<void*>( NewPoint ), shell_code, sizeof( shell_code ) );

	DWORD p = 0;

	if ( !VirtualProtect( reinterpret_cast<void*>( AllocMem + 0x1000 ), 0x1000, PAGE_EXECUTE_READ, &p ) )
	{
		printf( "[Thread: %d] Fail VP\n", Tid );
		return getchar( );
	}

	printf( "[Thread: %d] NewPoint: 0x%p\n", Tid, (void*)NewPoint );

	if ( ThreadsCount == 0 )
		VExDebugger::Init( HandlerType::VectoredExceptionHandler, true );

	auto Result = VExDebugger::SetTracerAddress(

		NewPoint, 
		BkpMethod::PageExceptions,
		BkpTrigger::Execute, 
		BkpSize::Size_4,

		[ ]( PEXCEPTION_RECORD pExceptionRec, PCONTEXT pContext ) -> CBReturn
		{
			static std::map<uint32_t, uint32_t> ThreadsCallbackCount = {};

			auto ThreadID = GetCurrentThreadId( );

			printf( "[Thread: %d] Called callback = 0x%llX, count: %d\n", ThreadID, pContext->Rip, ++ThreadsCallbackCount[ ThreadID ] );

			if ( ThreadsCount != ThreadsCountChange )
			{
				printf( "[Thread: %d] ThreadCount Change\n", ThreadID );
				ThreadsCountChange = ThreadsCount;
				Sleep( 400 );
			}

			//if ( ThreadsCallbackCount[ ThreadID ] == 9 || 
			//	ThreadsCallbackCount[ ThreadID ] == 10 || 
			//	ThreadsCallbackCount[ ThreadID ] == 11 || 
			//	ThreadsCallbackCount[ ThreadID ] == 50 )
			//{
			//	printf( "[Thread: %d] Espere\n", ThreadID );
			//	getchar( );
			//}

			if ( ThreadsCallbackCount[ ThreadID ] >= 59 )
			{
				return CBReturn::StopTrace;
			}

			return CBReturn::StepOver;
			return CBReturn::StepInto;
		}
	);
	printf( "[Thread: %d] AddPageExceptions Result: %d\n", Tid, Result );


	++ThreadsCount;

	//PGEMgr::AddPageExceptions( NewPoint, BkpTrigger::Write, BkpSize::Size_4 );

    //PGEMgr::RemovePageExceptions( NewPoint, BkpTrigger::Write );


	//{ // Execute
	//	MessageBoxA( 0, "Execute start", "Execute", 0 );
	//	auto func = reinterpret_cast<uint32_t( __fastcall* )( int )>( Point + 0x10 );
	//	auto v = func( 123 );
	//	printf( "v: %X\n", v );
	//	MessageBoxA( 0, "Execute end", "Execute", 0 );
	//}

	{ // Execute 2
		//MessageBoxA( 0, "Execute start", ( StrTID +  " Execute" ).c_str(), 0 );
		auto func = reinterpret_cast<uint32_t( __fastcall* )( int )>( NewPoint );
		auto v = func( 123 );
		printf( "[Thread: %d] v: %X\n", Tid, v );
		//MessageBoxA( 0, "Execute end", ( StrTID + " Execute" ).c_str( ), 0 );
	}

	//PGEMgr::RemovePageExceptions( NewPoint, BkpTrigger::Execute );


	//{ // Execute 2
	//	MessageBoxA( 0, "Execute start", ( StrTID +  " Execute" ).c_str(), 0 );
	//	auto func = reinterpret_cast<uint32_t( __fastcall* )( int )>( NewPoint );
	//	auto v = func( 123 );
	//	printf( "[Thread: %d] v: %X\n", Tid, v );
	//	MessageBoxA( 0, "Execute end", ( StrTID + " Execute" ).c_str( ), 0 );
	//}

	//{ // Read
	//	MessageBoxA( 0, "Read start", ( StrTID + " Read" ).c_str( ), 0 );
	//	auto val = *(uint8_t*)( NewPoint +1 );
	//	printf( "[Thread: %d] val: %X\n", Tid, val );
	//	MessageBoxA( 0, "Read end", ( StrTID + " Read" ).c_str( ), 0 );
	//}

	//{ // Write
	//	MessageBoxA( 0, "Write start", ( StrTID +  " Write" ).c_str(), 0 );
	//	*(uint8_t*)( NewPoint ) = 123;
	//	printf( "[Thread: %d] Write\n", Tid );
	//	MessageBoxA( 0, "Write end", ( StrTID +  " Write" ).c_str(), 0 );
	//}

	printf( "PageExceptionsList count=%lld, ThreadHandlingList count=%lld\n", PGEMgr::GetPageExceptionsList( ).size(), PGEMgr::GetThreadHandlingList( ).size() );

	getchar( );

	VirtualFree( AllocMem, 0, MEM_RELEASE );

	return getchar( );
}

void thread2( )
{
	printf( "Thread2 [%d]", GetCurrentThreadId( ) );

	while ( ThreadsCount && ( ThreadsCount == ThreadsCountChange ) )
	{
		Sleep( 1 );
	}

	printf( "Thread2 to testPGE\n" );

	testPGE( );
}

void thread3( )
{
	printf( "Thread3 [%d]", GetCurrentThreadId( ) );

	while ( ThreadsCount && ( ThreadsCount == ThreadsCountChange ) )
	{
		//Sleep( 1 );
	}

	printf( "Thread3 to testPGE\n" );

	testPGE( );
}

int main( )
{
	// test for multi-thread running in the same location

	printf( "Thread1 [%d]", GetCurrentThreadId() );

	//CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( thread2 ), nullptr, 0, nullptr );
	//CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( thread3 ), nullptr, 0, nullptr );
	
	testPGE( );


	return getchar( );
}

#pragma optimize( "", on )