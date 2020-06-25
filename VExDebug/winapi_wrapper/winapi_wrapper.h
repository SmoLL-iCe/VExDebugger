#pragma once
#include <Windows.h>
#include <iostream>
typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
typedef NTSTATUS* PNTSTATUS;
enum import_type
{
	syscall_id,
	direct_address
};

namespace wrap
{
	uint32_t ini_import(import_type load);
	NTSTATUS get_status( );
	void restore_bytes( );
	void* import_module( const wchar_t* name );
	void* get_export( void* mod, const char* name );
	bool write_process_mem(HANDLE h_process, void* p_address, void* p_buffer, size_t size, size_t* out_size);
	bool write_process_mem(HANDLE h_process, void* p_address, void* p_buffer, size_t size);
	bool read_process_mem(HANDLE h_process, void* p_address, void* p_buffer, size_t size, size_t* out_size);
	bool read_process_mem(HANDLE h_process, void* p_address, void* p_buffer, size_t size);
	void* virtual_alloc(HANDLE lh_process, size_t size, uint32_t u_alloc_type, uint32_t  u_protect_type);
	void* virtual_alloc(const HANDLE h_process_s, size_t size);
	void* virtual_alloc_m(size_t size);
	bool virtual_free(HANDLE lh_process, void* p_base_address, size_t region_size, uint32_t free_type);
	bool virtual_free(HANDLE h_process_s, void* p_base_address, size_t region_size);
	bool virtual_free_m(void* p_base_address, size_t region_size);
	bool virtual_protect(HANDLE lh_process, void** p_address, size_t number_of_bytes, DWORD new_protect, DWORD* old_protect);
	bool virtual_protect_m(void* p_address, size_t number_of_bytes, DWORD new_protect, DWORD* old_protect);
	HANDLE open_process(ACCESS_MASK dw_desired_access, uint64_t process_id);
	bool close(HANDLE to_close);
	size_t virtual_query(HANDLE lh_process, void* p_address, uint32_t mem_info, void* p_buffer, size_t dw_length);
	size_t virtual_query(HANDLE lh_process, void* p_address, PMEMORY_BASIC_INFORMATION p_buffer, size_t dw_length);
	size_t virtual_query(void* p_address, const uint32_t mem_info, void* p_buffer, size_t dw_length);
	NTSTATUS query_sys_info(int32_t SystemInformationClass, void* SystemInformation, uint32_t SystemInformationLength, uint32_t* return_len);
	bool query_sys_info(const int32_t system_information_class, void* system_information, const uint32_t system_information_length);
	HANDLE open_file(const wchar_t* object_path);
	bool dev_io_ctrl(HANDLE h_device, DWORD dw_io_control_code, LPVOID lp_in_buffer, DWORD n_in_buffer_size, LPVOID lp_out_buffer, DWORD n_out_buffer_size );
	bool map_view_sec(HANDLE h_section, void** p_base_address, size_t* view_size);
	bool un_map_view_sec(void* p_base_address);
	bool map_view_sec(HANDLE h_section, void** p_base_address, size_t* view_size);
	bool un_map_view_sec(void* p_base_address);
	ACCESS_MASK is_valid_handle(HANDLE h_handle);
	bool get_context_thread(HANDLE h_thread, PCONTEXT p_context);
	bool set_context_thread(HANDLE h_thread, PCONTEXT p_context);
	uint32_t suspend_thread(HANDLE h_thread);
	uint32_t resume_thread(HANDLE h_thread);
	bool terminate_thread(HANDLE h_thread, NTSTATUS exit_status);
	bool set_info_thread(HANDLE h_thread, THREAD_INFORMATION_CLASS ThreadInformationClass, void* thread_information, uint32_t thread_information_length);
	bool set_event(HANDLE h_event, int32_t* PreviousState );
	HANDLE create_event(ACCESS_MASK am_desired_access, int32_t EventType, BOOLEAN initial_state);
	bool wait_for_single_obj(HANDLE handle);
	void* add_veh( ULONG first, PVECTORED_EXCEPTION_HANDLER handler );
	HANDLE open_thread(ACCESS_MASK am_desired_access, uintptr_t thread_id);
	HANDLE create_thread(HANDLE h_proc, void* start_address, void* p_parameter, uint32_t * status);
	HANDLE create_thread_ex(HANDLE h_process_s, void* p_start_address, LPVOID lp_parameter, uint32_t* status);
	NTSTATUS query_sys_info(int32_t SystemInformationClass, void* SystemInformation, uint32_t SystemInformationLength, uint32_t* return_len);
	bool query_sys_info(int32_t system_information_class, void* system_information, uint32_t system_information_length);
	NTSTATUS query_info_thread(HANDLE h_thread, int32_t thread_info_class, void* thread_info, uint32_t thread_info_len, uint32_t* ret_len);
	bool query_info_thread(HANDLE h_thread, int32_t thread_info_class, void* thread_info, uint32_t thread_info_len);
	bool query_info_thread(int32_t  thread_info_class, void* thread_info, uint32_t thread_info_len, uint32_t* ret_len);
	NTSTATUS query_info_process(HANDLE h_proc, int32_t process_info_class, void* process_info, uint32_t process_info_len, uint32_t* ret_len);
	void delay(uint32_t delay);
	ACCESS_MASK is_valid_handle(HANDLE h_handle);
	NTSTATUS system_debug_control( unsigned long control_code, void* input_buffer, uint32_t input_buffer_length, void* output_buffer, uint32_t output_buffer_length, uint32_t* return_length );

}