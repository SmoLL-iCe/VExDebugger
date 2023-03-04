#pragma once

#include <Windows.h>
#include <iostream>
#include <vector>

namespace DoHook 
{
	std::uint8_t* GetNextPage( void* pAddress );
	std::vector<uint8_t> MakeJmp( void* SrcAddress, void* DstAddress );
	size_t GetFuncSize( void* pModule, void* pFunc );
	bool SetInlineHook( void* TargetAddress, void* pDetourFunc, void** pOriginalFunc, int RestoreSize = 5 );
}