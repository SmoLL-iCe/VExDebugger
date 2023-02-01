#include <windows.h>
#include <iostream>
//#define NOINTERFACE

#define log_file printf

int main()
{

#ifdef NOINTERFACE



#else
#ifdef _WIN64
	auto* mod = LoadLibrary( L"VExDebug_ImGui_x64.dll" );
#else

	auto* mod = LoadLibrary( L"VExDebug_ImGui_x86.dll" );
#endif
#endif

	return getchar();
}
