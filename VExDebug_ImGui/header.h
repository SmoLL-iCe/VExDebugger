#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#define r_cast reinterpret_cast
#ifdef _WIN64
#pragma comment(lib, "..\\import\\libs\\x64\\VExDebug.lib")
#pragma comment(lib, "..\\import\\libs\\x64\\winapi_wrapper.lib")
#else
#pragma comment(lib, "..\\import\\libs\\x86\\VExDebug.lib")
#pragma comment(lib, "..\\import\\libs\\x86\\winapi_wrapper.lib")
#endif
#include "../import/include/VExDebug.h"
