#include "../header.h"
#include "Utils.h"
#include <sstream>
#include <fstream>

std::string StrReplace( std::string str, const std::string& from, const std::string& to )
{
	size_t start_pos = 0;

	while (
		( start_pos = str.find( from, start_pos ) ) != 
		std::string::npos )
	{
		str.replace( start_pos, from.length( ), to );

		start_pos += to.length( ); 
	}

	return str;
}

std::string Utils::ValToHexStr( intptr_t x )
{
	std::stringstream stream{ };

	stream << std::hex << std::uppercase << x;

	return stream.str( );
}

bool Utils::IsValidHex( std::string& s )
{
	s = StrReplace( s, " ", "" );

	s = StrReplace( s, "0x", "" );

	return
		s.size( ) >= 2 &&
		s.find_first_not_of( "0123456789abcdefABCDEF", 1 ) == std::string::npos;
}

std::wstring Utils::GetEnv( const wchar_t* env_var )
{
    size_t r_size;

    auto err = _wgetenv_s( &r_size, nullptr, 0, env_var );

    if ( err || !r_size ) 
		return {};

    const auto local_env = reinterpret_cast<wchar_t*>( LocalAlloc( 0x40, r_size * sizeof( wchar_t ) ) );

    if ( !local_env ) 
		return {};

    _wgetenv_s( &r_size, local_env, r_size, env_var );

    const std::wstring wlocal( local_env );

    LocalFree( local_env );

    return wlocal;
}

std::vector<std::wstring> split( const std::wstring& str, const std::wstring& delim )
{
	std::vector<std::wstring> tokens;

	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find( delim, prev );

		if ( pos == std::wstring::npos ) 
			pos = str.length( );

		std::wstring token = str.substr( prev, pos - prev );

		if ( !token.empty( ) ) 
			tokens.push_back( token );

		prev = pos + delim.length( );

	} while ( pos < str.length( ) && prev < str.length( ) );

	return tokens;
}

bool IsPathExists( std::wstring str_path )
{
	auto FileAtt = GetFileAttributes( str_path.c_str( ) );

	return ( FileAtt != INVALID_FILE_ATTRIBUTES && ( FileAtt & FILE_ATTRIBUTE_DIRECTORY ) );
}

bool Utils::CreateFileFromText( std::wstring local_file, std::string data )
{
	std::ofstream out_file( local_file );

	if ( !out_file.is_open( ) )
		return false;

	out_file.write( data.data( ) , data.size( ) );

	out_file.close( );

	return true;
}

std::streampos Utils::GetFileSize( std::wstring file_path )
{
	std::streampos fsize = 0;

	std::ifstream file( file_path, std::ios::binary );

	fsize = file.tellg( );

	file.seekg( 0, std::ios::end );

	fsize = file.tellg( ) - fsize;

	file.close( );

	return fsize;
}

void Utils::OpenConsole( const std::string& title )
{
	FILE* stream = nullptr;

	AllocConsole( );

	freopen_s( &stream, "CONIN$", "r", stdin );

	freopen_s( &stream, "CONOUT$", "w", stdout );

	freopen_s( &stream, "CONOUT$", "w", stderr );

	SetConsoleTitleA( title.c_str( ) );
}
