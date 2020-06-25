#include <windows.h>
#include <iostream>
auto base = reinterpret_cast<uint8_t*>(GetModuleHandle(nullptr)) + 0x1000;
bool imprime = false;
void __stdcall thread_test(const uintptr_t * param)
{
	//printf( "%p\n", param );
	const auto id = *param;
	while (true)
	{
		//if (imprime)
		//	printf("hi from thread %d\n", base[0]);
		++base[0];
		++base[20];
		Sleep( 500);
	}

}

void __stdcall thread_test2(const uintptr_t* param)
{
	const auto id = *param;
	while (true)
	{

		Sleep(1000);
		++base[0];
		Sleep(1000);
		++base[20];
		Sleep(1000);
	}

}
int main()
{

	//auto mod = LoadLibrary( L"..\\..\\VExDebug\\VExDebug\\libs\\x64\\VExDebug.dll" );
#ifdef _WIN64
	auto mod = LoadLibrary( L"VExDebug_form_x64.dll" );
#else
	auto mod = LoadLibrary( L"VExDebug_form_x86.dll" );
#endif
	printf( "base %p\n", base );
	DWORD p;
	VirtualProtect(base, 20, PAGE_EXECUTE_READWRITE, &p);
	static uintptr_t i = 0;
    for (i = 0; i < 15; ++i)
    {
		CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(thread_test), &i, 0, nullptr);
		Sleep(100);
    }    
	//std::cout << "Hello World!\n";
	//auto mod = LoadLibrary(L"VExDebug_form.dll");
	//system("pause");
	//while (true)
	//{
	//	if (base[10] == 25)
	//	{
	//		base[10] = 1;
	//		i = 0;
	//		imprime = true;
	//		for (i = 0; i < 5; ++i)
	//		{
	//			CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(thread_test2), &i, 0, nullptr);
	//			Sleep(100);
	//		}
	//		base[10] = 0;
	//		break;
	//	}
	//	Sleep(3000);
	//}
	return getchar();
}
