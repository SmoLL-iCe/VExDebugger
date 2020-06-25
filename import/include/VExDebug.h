#pragma once
#include <Windows.h>
#include <iostream>
#include <map>

using exp_address_count = std::map<void*, uint32_t>;
enum class hw_brk_type
{
	hw_brk_execute,
	hw_brk_readwrite,
	hw_brk_write
};

enum class hw_brk_size
{
	hw_brk_size_1,
	hw_brk_size_2,
	hw_brk_size_8,
	hw_brk_size_4
};

namespace VExDebug
{

	void init();
	std::map<int, exp_address_count>& get_exp_assoc_address( );
	std::map<int, uintptr_t>& get_address_assoc_exp( );
	bool start_monitor_address(uintptr_t address, hw_brk_type type, hw_brk_size size);
	void remove_monitor_address(uintptr_t  address);
	void print_exceptions();
}