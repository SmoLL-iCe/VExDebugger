#include "WinWrap.h"
#include "Logs.h"
#include "ntos.h"
#include "../Headers/Header.h"



bool is_valid_range( void* p_address, void* mod_base, const size_t size )
{
	return ( reinterpret_cast<uintptr_t>( p_address ) > reinterpret_cast<uintptr_t>( mod_base ) && 
		reinterpret_cast<uintptr_t>( p_address ) < ( reinterpret_cast<uintptr_t>( mod_base ) + size ) );
}

DWORD dwMZ_PE00 = 0x5A4D + 0x00004550;
uint32_t get_sys_call_64( void* mod_base, const char* obf_func_name )
{
	auto* const u8_mod				= static_cast<uint8_t*>( mod_base );

	if ( !mod_base )
		return 0;

	auto* const p_img_dos			= static_cast<PIMAGE_DOS_HEADER>( mod_base );

	if ( !p_img_dos || !p_img_dos->e_lfanew )
		return 0;

	auto* const p_img_nt			= reinterpret_cast<PIMAGE_NT_HEADERS>( u8_mod + p_img_dos->e_lfanew );

	if ( !p_img_nt )
		return 0;

	const auto dw_flag				= p_img_dos->e_magic + p_img_nt->Signature;

	if ( dwMZ_PE00 != dw_flag )
		return 0;

	const auto bin_size				= p_img_nt->OptionalHeader.SizeOfImage;

	auto* const p_img_exp_dir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>( u8_mod +
		p_img_nt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress );

	if ( !p_img_exp_dir || !is_valid_range( p_img_exp_dir, mod_base, bin_size ) )
		return 0;

	auto* const u32_address			= reinterpret_cast<uint32_t*>( u8_mod + p_img_exp_dir->AddressOfFunctions		);

	auto* const u32_name			= reinterpret_cast<uint32_t*>( u8_mod + p_img_exp_dir->AddressOfNames			);

	auto* const u32_ordinal			= reinterpret_cast<uint16_t*>( u8_mod + p_img_exp_dir->AddressOfNameOrdinals	);

	if ( 
		!is_valid_range( u32_address,	mod_base, bin_size ) || 
		!is_valid_range( u32_name,		mod_base, bin_size ) || 
		!is_valid_range( u32_ordinal,	mod_base, bin_size ) )
		return 0;

	uint8_t p_buff[32] = { 0 };
	/* 
	 *  4C 8B D1              - mov r10,rcx
	 *  B8 XX000000           - mov eax,000000XX
	 */
	const uint8_t p_sig[4] = { 0x4C, 0x8B, 0xD1, 0xB8 };

	for ( DWORD i = 0; i < p_img_exp_dir->NumberOfFunctions; i++ )
	{
		auto* const p_address		= reinterpret_cast<uint8_t*>( u8_mod + u32_address[ u32_ordinal[ i ] ] );

		auto* const func_name		= reinterpret_cast<char*>( u8_mod + u32_name[ i ] );

		if ( 
			!is_valid_range( func_name, mod_base, bin_size ) || 
			!is_valid_range( p_address, mod_base, bin_size ) )
			continue;
		
		if ( !strcmp( func_name, obf_func_name ) )
		{
			memcpy(&p_buff, p_address, sizeof p_buff);
			for (auto x = 0; x < static_cast<int>( sizeof p_sig ); x++)
			{

				printf( "\t   %p\t%s\n", p_address, func_name );
				//if ( p_buff[ x ] != p_sig[ x ] )
				//	break;

				//if ( x == sizeof( p_sig ) - 1 )
				//	//
				//	return *reinterpret_cast<uint32_t*>( &p_buff[ 4 ] );
			}
		}
	}
	return 0;
}

uint32_t GetNtSyscall( HMODULE pMod, const char* ntName )
{
	auto pPoint = (uint8_t*)GetProcAddress( pMod, ntName );

	if ( !pPoint )
	{
		log_file( "[-] - Fail Get Export offset: %s", ntName );
		return false;
	}

	log_file( "[+] - pPoint: %X", pPoint );

	uint32_t SysId = 0;

	auto Point = pPoint + 0xF;

	for ( size_t i = 0; i < 0x30; i++ )
	{
		if ( Point[ i ] == 0xB8 && Point[ i + 5 ] == 0xBA )
		{
			SysId = ( *reinterpret_cast<uint32_t*>( &Point[ i + 1 ] ) ) - 1;
			break;
		}
	}

	return SysId;
}


bool WinWrap::Init( )
{
	get_sys_call_64( LoadLibrary( L"ntdll.dll" ), "adfsdf" );

	return true;
}

ACCESS_MASK WinWrap::IsValidHandle( HANDLE Handle )
{
	const auto bRet = ( Handle != nullptr && Handle != INVALID_HANDLE_VALUE );

	if ( bRet )
	{
		OBJECT_BASIC_INFORMATION ObjInfo = { 0 };

		DWORD RetLen = 0;

		if ( NtQueryObject( Handle, ObjectBasicInformation, &ObjInfo, sizeof( OBJECT_BASIC_INFORMATION ), &RetLen ) == 0 )
			return ObjInfo.GrantedAccess;

	}
	return 0;
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

	NtOpenThread( &hThread, DesiredAccess, &ObjAtt, &cPid );

	return hThread;
}

bool WinWrap::GetContextThread( HANDLE hThread, PCONTEXT pContext )
{
	//auto status = reinterpret_cast<decltype( NtGetContextThread )*>(  NtGetContextThreadAsm )( h_thread, p_context );
	auto status = NtGetContextThread( hThread, pContext );
	return ( status == 0 );
}

ULONG WinWrap::GetErrorStatus( )
{
	return 0;
}

bool WinWrap::SetContextThread( HANDLE hThread, PCONTEXT pContext )
{
	//auto status = reinterpret_cast<decltype( NtSetContextThread )*>( NtSetContextThreadAsm )( h_thread, p_context );
	auto status = NtSetContextThread( hThread, pContext );

	return ( status == 0 );
}

uint32_t WinWrap::SuspendThread( HANDLE hThread )
{
	ULONG suspend_count = -1;
	NtSuspendThread( hThread, &suspend_count );
	return suspend_count;
}

uint32_t WinWrap::ResumeThread( HANDLE hThread )
{
	ULONG suspend_count = -1;
	NtResumeThread( hThread, &suspend_count );
	return suspend_count;
}