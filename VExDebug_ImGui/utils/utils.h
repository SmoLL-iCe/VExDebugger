#pragma once
namespace utils
{
    std::wstring get_env( const wchar_t* env_var );
    bool create_file_text( std::wstring local_file, std::string data );
    bool is_valided_hex( std::string& s );
    std::streampos get_file_size( std::wstring file_path );
};

