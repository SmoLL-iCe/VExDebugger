#pragma once
#include <Windows.h>
#include <iostream>

namespace nLog
{
	bool Init( );
	bool file_n( const char* sText );
	bool file( const char* szFormat, ... );
}

#define log_file(fmt, ...) nLog::file( fmt ## "\n" , __VA_ARGS__)
#define log_nfile(fmt) nLog::file_n( fmt ## "\n" )
