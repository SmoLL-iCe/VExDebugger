#include "Config.h"

Config* g_Config = nullptr;
Config* Config::i( ) 
{
	if ( !g_Config )
		g_Config = new Config( );

	return g_Config;
}