#pragma once
namespace threads
{
	struct thread_idem
	{
		uint32_t id = 0;
		HANDLE handle = nullptr;
	};

	bool update_threads();
	std::map<uint32_t, HANDLE> get_thread_list();
}
