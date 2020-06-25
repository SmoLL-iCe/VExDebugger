#pragma once
#include <windows.h>
class hw_bkp
{
private:
	uintptr_t address		= 0;
	hw_brk_type type		= hw_brk_type::hw_brk_execute;
	hw_brk_size size		= hw_brk_size::hw_brk_size_1;
	intptr_t i_reg_busy		= 0; //max 4
	bool add				= true;
public:
	hw_bkp(uintptr_t address, hw_brk_size size, hw_brk_type type, bool add = true);
	static hw_bkp* i();
	bool apply_debug_control(HANDLE h_thread, bool use_existing = false);
	bool& add_bkp();
	uintptr_t get_address() const;
};