#include <iostream>
#include <windows.h>
#include <VExDebugger.h>
#include <iomanip>

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

uint8_t* AllocMemPage = 0;
uint8_t* RelativePointPtr = 0;

void func( )
{
	VExDebugger::Init( HandlerType::VectoredExceptionHandler, false, false );

	VExDebugger::StartMonitorAddress( RelativePointPtr + 0x100, HwbkpType::ReadWrite, HwbkpSize::Size_2 );
	VExDebugger::StartMonitorAddress( RelativePointPtr + 0x120, HwbkpType::ReadWrite, HwbkpSize::Size_1 );
	VExDebugger::StartMonitorAddress( RelativePointPtr + 0x140, HwbkpType::ReadWrite, HwbkpSize::Size_4 );
	VExDebugger::StartMonitorAddress( RelativePointPtr, HwbkpType::ReadWrite, HwbkpSize::Size_1 );
}
#define READ_TEST
#define WRITE_TEST

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

	CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( func ), nullptr, 0, nullptr );

	Sleep( 1000 );

	printf( "read point: %llX\n", *(uint64_t*)( RelativePointPtr ) );

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
				auto pFuncReadHit		= reinterpret_cast<uint8_t( __fastcall* )( uint8_t* )>( pPtr );
				auto pFuncWriteHit		= reinterpret_cast<uint8_t( __fastcall* )( uint8_t*, uint8_t )>( pPtr + 0x4 );
				auto rCount				= random( 1, 200 );
#if defined( READ_TEST )
				for ( size_t i = 0; i < rCount; i++ )
					pFuncReadHit( RelativePointPtr );
#endif

#if defined( WRITE_TEST )
				rCount = random( 1, 20 );
				for ( size_t i = 0; i < rCount; i++ )
					pFuncWriteHit( RelativePointPtr, random( 0, 255) );
#endif
			}

			VirtualFree( AnotherMemPage, 0, MEM_RELEASE );
		}
	}

#endif

	for ( const auto& [ Address, BpInfo ] : VExDebugger::GetBreakpointList( ) )
	{
		if ( !Address )
			continue;

		if ( BpInfo.Type != BkpType::Hardware )             // only support hardware breakpoint
			continue;

		auto ItExceptionList  = VExDebugger::GetAssocExceptionList( ).find( Address );

		if ( ItExceptionList == VExDebugger::GetAssocExceptionList( ).end( ) )
			continue;

		auto& ExceptionList     = ItExceptionList->second;

		for ( const auto& [ExceptionAddress, ExceptionInfo] : ExceptionList )
        {
            std::cout << 
				"Count " << std::setfill(' ') << std::setw(8) << std::dec << ExceptionInfo.Details.Count <<
				" Address: 0x" << std::hex << std::uppercase << ExceptionAddress << "\n";

        }

	}
    return getchar( );
}

#pragma optimize( "", on )