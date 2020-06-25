#include "../header.h"
#include "utils.h"
#include <sstream>
#include <fstream>


std::string str_replace( std::string str, const std::string& from, const std::string& to )
{
	size_t start_pos = 0;
	while ( ( start_pos = str.find( from, start_pos ) ) != std::string::npos )
	{
		str.replace( start_pos, from.length( ), to );
		start_pos += to.length( ); 
	}
	return str;
}

bool utils::is_valided_hex( std::string& s )
{
	s = str_replace( s, " ", "" );
	s = str_replace( s, "0x", "" );
	return
		s.size( ) >= 2 &&
		s.find_first_not_of( "0123456789abcdefABCDEF", 1 ) == std::string::npos;
}

std::wstring utils::get_env( const wchar_t* env_var )
{
    size_t r_size;
    auto err = _wgetenv_s( &r_size, nullptr, 0, env_var );
    if ( err || !r_size ) return {};
    const auto local_env = reinterpret_cast<wchar_t*>( LocalAlloc( 0x40, r_size * sizeof( wchar_t ) ) );
    if ( !local_env ) return {};
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
		if ( pos == std::wstring::npos ) pos = str.length( );
		std::wstring token = str.substr( prev, pos - prev );
		if ( !token.empty( ) ) tokens.push_back( token );
		prev = pos + delim.length( );
	} while ( pos < str.length( ) && prev < str.length( ) );
	return tokens;
}

bool path_exists( std::wstring str_path )
{
	auto f_att = GetFileAttributes( str_path.c_str( ) );
	return ( f_att != INVALID_FILE_ATTRIBUTES && ( f_att & FILE_ATTRIBUTE_DIRECTORY ) );
}

bool utils::create_file_text( std::wstring local_file, std::string data )
{
	std::ofstream out_file( local_file );
	if ( !out_file.is_open( ) )
		return false;
	out_file.write( data.data( ) , data.size( ) );
	out_file.close( );
	return true;
}

std::streampos utils::get_file_size( std::wstring file_path )
{

	std::streampos fsize = 0;
	std::ifstream file( file_path, std::ios::binary );

	fsize = file.tellg( );
	file.seekg( 0, std::ios::end );
	fsize = file.tellg( ) - fsize;
	file.close( );

	return fsize;
}
