#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#define r_cast reinterpret_cast

#define MM "MD"
#ifdef _MT
#undef MM
#define MM "MT"
#endif

#define ARCH "86"
#ifdef _WIN64
#undef ARCH
#define ARCH "64"
#endif

#define RD ""
#ifdef _DEBUG
#undef RD
#define RD "d"
#endif

#define IMPORT_LIB "x" ARCH "\\VExDebugger" MM RD ".lib"

#pragma comment(lib, IMPORT_LIB)

#include <VExDebugger.h>



