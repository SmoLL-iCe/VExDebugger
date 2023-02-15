#include <iostream>
#include <windows.h>
#include <VExDebugger.h>
#include <iomanip>
#include <TlHelp32.h>
#include <psapi.h>
#include "color.hpp"

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

uint8_t* AllocMemPage = 0;
uint8_t* RelativePointPtr = 0;

void func( )
{
	VExDebugger::Init( HandlerType::VectoredExceptionHandler, false, true );

	VExDebugger::StartMonitorAddress( RelativePointPtr + 0x100, BkpTrigger::ReadWrite, BkpSize::Size_2 );

	VExDebugger::StartMonitorAddress( RelativePointPtr + 0x200, BkpTrigger::ReadWrite, BkpSize::Size_1 );

	VExDebugger::StartMonitorAddress( RelativePointPtr + 0x300, BkpTrigger::Execute, BkpSize::Size_1 );

	VExDebugger::StartMonitorAddress( RelativePointPtr, BkpTrigger::ReadWrite, BkpSize::Size_1 );

}
//#define READ_TEST
//#define WRITE_TEST

#define READ_MOD_TEST
#define WRITE_MOD_TEST

void test_mod( )
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

void test_page( )
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

#pragma optimize( "", off )
int main( )
{
    std::cout << "Hello World!\n";

	auto RandomOffset = random( 0x10, 0x200 );

	AllocMemPage = reinterpret_cast<uint8_t*>( VirtualAlloc( 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );

	if ( !AllocMemPage )
	{
		std::cout << "insufficient mem res\n";
		return getchar( );
	}

	printf( "AllocMemPage: 0x%p\n", AllocMemPage );

	RelativePointPtr = AllocMemPage + RandomOffset;

	printf( "RelativePointPtr: 0x%p\n", RelativePointPtr );

	*reinterpret_cast<uint64_t*>( RelativePointPtr ) = 0x9090C300001337B8;

	Sleep( 1000 );

	*(uint32_t*)( RelativePointPtr + 0x300 ) = 0x90C3018A;

	auto pFuncExecHit = reinterpret_cast<uint8_t( __fastcall* )( uint8_t* )>( RelativePointPtr + 0x300 );

	CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( func ), nullptr, 0, nullptr );

	Sleep( 2000 );

	//CONTEXT ctx{};

	//ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	//ctx.ContextFlags = CONTEXT_ALL;

	//GetThreadContext( GetCurrentThread( ), &ctx );

	//printf( "read point: %llX\n", *(uint64_t*)( RelativePointPtr ) );

	for ( size_t i = 0; i < 3; i++ )
	{

		uintptr_t val = 12;
		auto t = pFuncExecHit( (uint8_t*)&val );

		printf( "ret: %02X\n", t );
	}

	test_mod( );

	//test_page( );

	VExDebugger::CallBreakpointList( []( TBreakpointList BreakpointList ) -> void {

		for ( const auto& [Address, BpInfo] : BreakpointList )
		{
			if ( !Address )
				continue;

			if ( BpInfo.Method != BkpMethod::Hardware )             // only support hardware breakpoint
				continue;

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



    return getchar( );
}

#pragma optimize( "", on )