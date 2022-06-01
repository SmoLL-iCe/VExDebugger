#pragma once
#include <Windows.h>

namespace VEH_internal 
{
	void HookVEHHandlers( void* VectoredHandler, void*& orig_VectoredHandler );
}