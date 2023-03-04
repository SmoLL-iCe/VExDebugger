#pragma once
namespace MgrThreads
{
	struct ThreadIdem
	{
		uint32_t Id = 0;
		HANDLE Handle = nullptr;
	};

	bool UpdateThreads( );
	std::map<uint32_t, HANDLE>& GetThreadList( );
}
