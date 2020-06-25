#pragma once
#include <Windows.h>
#include <string>

namespace iwrap
{
#ifndef _WIN64
	bool is64bitOS();
#endif
	std::wstring encode( const wchar_t* name );
	std::string encode( const char* name );
	NTSTATUS init_import(int typee);
	void* import_module( const wchar_t * name );
	void* get_export( void * mod, const char* name );
	
	void memcpy_protect(void* address, uint8_t* valor, int size);
	void restore_bytes();
	NTSTATUS __stdcall NtCreateThread(PHANDLE rh_thread, ACCESS_MASK desired_access, POBJECT_ATTRIBUTES obj, HANDLE h_process, PCLIENT_ID client_id, PCONTEXT p_ctx, PINITIAL_TEB init_peb, BOOLEAN suspended);
	NTSTATUS __stdcall NtCreateThreadEx(PHANDLE rh_thread, ACCESS_MASK d_access, void* obj_att, HANDLE h_process,
		void* start, void* arg, ULONG CreateFlags, size_t zero_bits, size_t stack_size, size_t max_stack_size, void* att_list);
	NTSTATUS __stdcall NtWriteVirtualMemory(HANDLE h_process, void* address, void* buffer, size_t  NumberOfBytes, size_t* NumberOfBytes_r);
	NTSTATUS __stdcall NtReadVirtualMemory(HANDLE h_process, void* address, void* buffer, size_t  NumberOfBytes, size_t* NumberOfBytes_r);
	NTSTATUS __stdcall NtProtectVirtualMemory(HANDLE h_process, void** p_address, size_t* NumberOfBytesToprotect, DWORD NewAccessprotection, PDWORD OldAccessprotection);
	NTSTATUS __stdcall NtQueryVirtualMemory(HANDLE h_process, void* address, MEMORY_INFORMATION_CLASS mem_info_class, void* mem_info, size_t  mem_l, size_t* mem_rl);
	NTSTATUS __stdcall NtAllocateVirtualMemory(HANDLE h_process, void** address, uintptr_t zero_bits, size_t* region_size, uint32_t alloc_type, uint32_t protect);
	NTSTATUS __stdcall NtQuerySystemInformation(int32_t SystemInformationClass, void* SystemInformation, uint32_t SystemInformationLength, uint32_t* return_len);
	NTSTATUS __stdcall NtQueryObject(HANDLE Handle, OBJECT_INFORMATION_CLASS obj_info_class, void* obj_info, uint32_t obj_info_len, uint32_t* ret_len);
	NTSTATUS __stdcall NtFreeVirtualMemory(HANDLE lh_process, void* p_base_address, size_t* region_size, uint32_t free_type);
	NTSTATUS __stdcall NtDeviceIoControlFile(HANDLE h_file, HANDLE event, PIO_APC_ROUTINE apc_routine, void* apc_ctx, PIO_STATUS_BLOCK IoSttsBlock, uint32_t IoCC, void* IB, uint32_t IBL, void* OB, uint32_t OBL);
	NTSTATUS __stdcall NtOpenProcess(HANDLE* h_process, ACCESS_MASK d_access, OBJECT_ATTRIBUTES* obj_att, CLIENT_ID* c_id);
	NTSTATUS __stdcall NtClose(HANDLE handle);
	NTSTATUS __stdcall NtDuplicateObject(HANDLE s_h_process, HANDLE s_handle, HANDLE t_h_process, HANDLE* h_target, ACCESS_MASK d_access, uint32_t h_att, uint32_t opts);
	NTSTATUS __stdcall NtSetInformationObject(HANDLE h_obj, OBJECT_INFORMATION_CLASS obj_info_class, void* obj_info, uint32_t Length);
	NTSTATUS __stdcall NtOpenFile(HANDLE* h_file, ACCESS_MASK d_access, OBJECT_ATTRIBUTES* obj_att, PIO_STATUS_BLOCK IoStatusBlock, uint32_t share_access, uint32_t open_opts);
	NTSTATUS __stdcall ZwMapViewOfSection(HANDLE h_section, HANDLE h_process, void** p_address, uintptr_t zero_bits, size_t commit_size, 
		LARGE_INTEGER* section_offset, size_t* view_size, SECTION_INHERIT inherit_desc, uint32_t alloc_type, uint32_t win32_protect);
	NTSTATUS __stdcall ZwUnmapViewOfSection(HANDLE h_process, void* p_address);
	NTSTATUS __stdcall ZwCreateSection(HANDLE* h_section, ACCESS_MASK d_access, OBJECT_ATTRIBUTES* obj_att, LARGE_INTEGER* max_size, uint32_t section_page_pro, uint32_t alloc_att, HANDLE h_file);
	NTSTATUS __stdcall ZwOpenSection(HANDLE* h_section, ACCESS_MASK d_access, OBJECT_ATTRIBUTES* obj_att);
	NTSTATUS __stdcall ZwMakeTemporaryObject(HANDLE handle);
	NTSTATUS __stdcall NtGetContextThread(HANDLE h_thread, PCONTEXT p_ctx);
	NTSTATUS __stdcall NtSetContextThread(HANDLE h_thread, PCONTEXT p_ctx);
	NTSTATUS __stdcall NtResumeThread(HANDLE h_thread, uint32_t* sus_count);
	NTSTATUS __stdcall NtSuspendThread(HANDLE h_thread, uint32_t* previous_sus_count);
	NTSTATUS __stdcall NtTerminateThread(HANDLE h_thread, NTSTATUS exit_stts);
	NTSTATUS __stdcall NtSetInformationThread(HANDLE h_thread, THREADINFOCLASS thread_info_class, void* thread_info, uint32_t thread_info_len);
	NTSTATUS __stdcall ZwSetEvent(HANDLE h_event, int32_t* previous_state);
	NTSTATUS __stdcall NtOpenThread(HANDLE* h_thread, ACCESS_MASK d_access, OBJECT_ATTRIBUTES* obj_att, CLIENT_ID* c_id);
	NTSTATUS __stdcall ZwCreateEvent(HANDLE* h_event, ACCESS_MASK d_access, OBJECT_ATTRIBUTES* obj_att, EVENT_TYPE e_type, BOOLEAN init_state);
	NTSTATUS __stdcall NtWaitForSingleObject(HANDLE h, BOOLEAN alertable, LARGE_INTEGER* time_out);
	NTSTATUS __stdcall NtQueryInformationThread(HANDLE h_thread, THREADINFOCLASS thread_info_class, void* thread_info, uint32_t thread_info_len, uint32_t* r_len);
	NTSTATUS __stdcall NtQueryInformationProcess(HANDLE h_process, PROCESSINFOCLASS process_info_class, void* process_info, uint32_t process_info_len, uint32_t* r_len);
	NTSTATUS __stdcall NtDelayExecution(BOOLEAN alertable, LARGE_INTEGER* delay_interval);
	PVOID	 __stdcall RtlAddVectoredExceptionHandler( ULONG First, PVECTORED_EXCEPTION_HANDLER Handler );
	NTSTATUS __stdcall NtSystemDebugControl( unsigned long control_code, void* input_buffer, uint32_t input_buffer_length, void* output_buffer, uint32_t output_buffer_length, uint32_t* return_length );
}
