#pragma once
#include "../../import/include/VExDebugger.h"

class Config
{
public:
	static Config* i( );

	bool m_Logs					= false;

	bool m_SpoofHwbkp			= false;

	HandlerType m_HandlerType	= HandlerType::VectoredExceptionHandler;

};