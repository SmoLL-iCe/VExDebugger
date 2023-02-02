#pragma once
#include <iostream>
#include <vector>
#include <string>

namespace Utils
{
	void OpenConsole( const std::string& title );
	const char* SystemStrToCharStr( System::String^ text );
	std::string StrReplace( std::string str, const std::string& from, const std::string& to );
	bool IsValidHex( std::string& s );
	char* FormatString( const char* fmt, ... );
	System::String^ CharStrToSystemStr( char* text );
	void MsgBox( System::IntPtr handle, const char* text, const char* caption, int typ );
}