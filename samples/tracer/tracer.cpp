#include <iostream>
#include <windows.h>
#include <VExDebugger.h>
#include <iomanip>
#include <TlHelp32.h>
#include <psapi.h>
#include "../../VExDebugger/Tools/ntos.h"

#define MM "MT"

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

#ifndef _WIN64
#define REG(v) E##v
#else
#define REG(v) R##v
#endif


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
			const auto BaseAddress = reinterpret_cast<uintptr_t>( module.modBaseAddr );

			const auto EndBase = BaseAddress + module.modBaseSize;

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

#pragma optimize( "", off )
void TraceMe( )
{
	MessageBoxA( 0, "Hello", "Normal", 0 );
}

char MsgData[ ] = "hello data";
void TraceMeData( )
{
	MsgData[ 0 ] = 'H';
	MessageBoxA( 0, MsgData, "Data", 0 );
}
#pragma optimize( "", on )

void PrintAddress( std::uintptr_t Address )
{
	auto Module = GetModuleInfo( Address );

	if ( !Module.modBaseAddr )
	{
		std::cout << "Address: 0x" << std::hex << std::uppercase << Address << "\n";
	}
	else
	{
		auto Offset = Address - reinterpret_cast<uintptr_t>( Module.modBaseAddr );

		std::cout << "Address: ";

		std::wcout << Module.szModule;

		std::cout << "+" << std::hex << std::uppercase << Offset << "\n";
	}
}
int
WINAPI
hkMessageBoxA(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_ UINT uType ) {


}

static int count = 0;
void SetBreakPointsFunc( )
{
	VExDebugger::Init( HandlerType::VectoredExceptionHandler, true );

	VExDebugger::SetTracerAddress(
		TraceMe,
		BkpMethod::Hardware,
		BkpTrigger::Execute,
		BkpSize::Size_1,
		[ ]( PEXCEPTION_RECORD pExceptRec, PCONTEXT pContext ) -> CBReturn
		{

			std::cout << count << " ";
			PrintAddress( pContext->REG( ip ) );

			uint8_t* pPoint = reinterpret_cast<uint8_t*>( pContext->REG( ip ) );

			if ( pPoint[ 0 ] == 0xff && pPoint[ 1 ] == 0x15 )
			{
				static char str[ ] = "another string";
				
#ifndef _WIN64
				*reinterpret_cast<std::uintptr_t*>( pContext->Esp + 4 ) = reinterpret_cast<uintptr_t>( str );
#else
				pContext->Rdx = reinterpret_cast<uintptr_t>( str );
#endif

			}


			++count;


			return CBReturn::StopTrace;

		}
	);

//	VExDebugger::SetTracerAddress(
//		MsgData,
//		BkpMethod::Hardware,
//		BkpTrigger::ReadWrite, // data rw
//		BkpSize::Size_1,
//		[ ]( PEXCEPTION_RECORD pExceptRec, PCONTEXT pContext ) -> CBReturn
//		{
//
//			static int count = 0;
//			std::cout << count << " ";
//			PrintAddress( pContext->REG( ip ) );
//
//			uint8_t* pPoint = reinterpret_cast<uint8_t*>( pContext->REG( ip ) );
//
//			if ( pPoint[ 0 ] == 0xff && pPoint[ 1 ] == 0x15 )
//			{
//				static char str[ ] = "hacked";
//
//#ifndef _WIN64
//				*reinterpret_cast<std::uintptr_t*>( pContext->Esp + 4 ) = reinterpret_cast<uintptr_t>( str );
//#else
//				pContext->Rdx = reinterpret_cast<uintptr_t>( str );
//#endif
//
//			}
//
//
//			++count;
//
//
//			return ( count >= 8 ) ? CBReturn::StopTrace : CBReturn::StepOver;
//
//		}
//	);

}


int main()
{
    std::cout << "Hello World!\n";

	CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( SetBreakPointsFunc ), nullptr, 0, nullptr );

	Sleep( 500 );

	while ( true )
	{
		count = 0;
		TraceMe( );

	}
	//TraceMeData( );

	return getchar( );
}
