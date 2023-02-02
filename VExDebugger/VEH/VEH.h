#pragma once
#include <Windows.h>

namespace VEH_Internal 
{
	bool InterceptVEHHandler( void* VectoredHandler, void*& orig_VectoredHandler );
	bool HookKiUserExceptionDispatcher( void* MyVectoredHandler );
}