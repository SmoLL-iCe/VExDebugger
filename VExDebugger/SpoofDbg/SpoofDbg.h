#pragma once 

#include "../Headers/Header.h"

namespace SpoofDbg 
{ 
	bool HookNtGetContextThread( );
	bool HookNtContinue( );
}