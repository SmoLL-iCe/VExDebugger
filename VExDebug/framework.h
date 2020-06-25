#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <TlHelp32.h>
#include <vector>
#include <map>
#define r_cast reinterpret_cast

//#ifdef _WIN64
//#pragma comment(lib, "VExDebug\\libs\\x64\\winapi_wrapper.lib")
//#else
//#pragma comment(lib, "VExDebug\\libs\\x86\\winapi_wrapper.lib")
//#endif