#include "Utils.h"
#include <Windows.h>
#include <VExDebugger.h>
using namespace System;

using namespace System::Runtime::InteropServices;

void Utils::OpenConsole( const std::string& title )
{
	FILE* stream = nullptr;
	AllocConsole( );
	freopen_s( &stream, "CONIN$", "r", stdin );
	freopen_s( &stream, "CONOUT$", "w", stdout );
	freopen_s( &stream, "CONOUT$", "w", stderr );
	SetConsoleTitleA( title.c_str( ) );
}

const char* Utils::SystemStrToCharStr( System::String^ text )
{
	auto us_ptr = Marshal::StringToHGlobalAnsi( text );

	auto converted_text = static_cast<char*>( us_ptr.ToPointer( ) );

	Marshal::FreeHGlobal( us_ptr );

	return converted_text;
}

std::string Utils::StrReplace( std::string str, const std::string& from, const std::string& to )
{
	size_t start_pos = 0;
	while ( ( start_pos = str.find( from, start_pos ) ) != std::string::npos ) {
		str.replace( start_pos, from.length( ), to );
		start_pos += to.length( );
	}
	return str;
}

bool Utils::IsValidHex( std::string& s )
{
	s = StrReplace( s, " ", "" );
	s = StrReplace( s, "0x", "" );
	return
		s.size( ) >= 2 &&
		s.find_first_not_of( "0123456789abcdefABCDEF", 1 ) == std::string::npos;
}

char* Utils::FormatString( const char* fmt, ... )
{
	if ( !fmt ) 
		return nullptr;

	va_list va_a_list = {};

	va_start( va_a_list, fmt );

	auto ret = _vscprintf( fmt, va_a_list );

	auto const length = _vscprintf( fmt, va_a_list ) + 1;

	auto* log_buf = new char[ length ];

	_vsnprintf_s( log_buf, length, _TRUNCATE, fmt, va_a_list );

	va_end( va_a_list );

	return log_buf;
}

System::String^ Utils::CharStrToSystemStr( char* text )
{
	return gcnew System::String( text );
}

void Utils::MsgBox( System::IntPtr handle, const char* text, const char* caption, int typ )
{
	MessageBoxA( (HWND)handle.ToPointer( ), text, caption, typ );
}
