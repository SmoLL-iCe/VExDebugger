#pragma once
#include <iostream>

namespace Utils
{
	int _IsCallInstruction( uint8_t* PointInstruction );

	template <typename T = void*>
	inline int IsCallInstruction( T PointInstruction )
	{
		return _IsCallInstruction( reinterpret_cast<uint8_t*>( PointInstruction ) );
	}
}

