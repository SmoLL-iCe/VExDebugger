#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

namespace Utils
{
    std::wstring GetEnv( const wchar_t* env_var );

    bool CreateFileFromText( std::wstring local_file, std::string data );

    std::string ValToHexStr( intptr_t x );

    bool IsValidHex( std::string& s );

    std::streampos GetFileSize( std::wstring file_path );

    void OpenConsole( const std::string& title );
};

