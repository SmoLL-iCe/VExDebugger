#pragma once
#include "../../import/include/VExDebug.h";

class Config
{
public:
	static Config* i( );

	bool m_Logs					= false;

	bool m_SpoofHwbkp			= false;

	HandlerType m_HandlerType	= HandlerType::VectoredExceptionHandler;

private:

};