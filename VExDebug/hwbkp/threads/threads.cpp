#include "../../framework.h"
#include "threads.h"
#include "../../winapi_wrapper/nt_structs.h"
#include "../../winapi_wrapper/winapi_wrapper.h"

PSYSTEM_PROCESS_INFORMATION enum_system_threads( )
{
	NTSTATUS return_val;
	uint32_t data_length = 0x10000;
	PSYSTEM_PROCESS_INFORMATION proc_info;
	do
	{
		proc_info		= r_cast<PSYSTEM_PROCESS_INFORMATION>( wrap::virtual_alloc_m( data_length ) );
		return_val		= wrap::query_sys_info( SystemExtendedProcessInformation, proc_info, data_length, &data_length );
		if ( return_val == STATUS_INFO_LENGTH_MISMATCH )
		{
			if ( proc_info )
				wrap::virtual_free_m( proc_info, 0 );
			data_length *= 2;
		}
	} while ( return_val == STATUS_INFO_LENGTH_MISMATCH );
	if ( return_val != STATUS_SUCCESS ) return nullptr;
	return proc_info;
}

PSYSTEM_HANDLE_INFORMATION enum_system_handles( )
{
	NTSTATUS return_val;
	uint32_t data_length = 0x10000;
	PSYSTEM_HANDLE_INFORMATION handles_info;
	do
	{
		handles_info = r_cast<PSYSTEM_HANDLE_INFORMATION>( wrap::virtual_alloc_m( data_length ) );
		return_val = wrap::query_sys_info( SystemHandleInformation, handles_info, data_length, &data_length );
		if ( return_val == STATUS_INFO_LENGTH_MISMATCH )
		{
			if ( handles_info )
				wrap::virtual_free_m( handles_info, 0 );
			data_length *= 2;
		}
	} while ( return_val == STATUS_INFO_LENGTH_MISMATCH );
	if ( return_val != STATUS_SUCCESS ) return nullptr;
	return handles_info;
}

std::map<uint32_t, HANDLE> list_thread_idem = {};
bool threads::update_threads( )
{
	if ( auto handles_info = enum_system_handles( ) )
	{
		for ( uint32_t i = 0; i < handles_info->HandleCount; i++ )
		{
			auto const handle_info = handles_info->Handles[ i ];
			if ( DWORD( handle_info.ProcessId ) == GetCurrentProcessId( ) )
			{
				auto const handle	= reinterpret_cast<HANDLE>( handle_info.Handle );
				auto const tid		= GetThreadId( handle );
				if ( !tid )
					continue;
				if ( !( handle_info.GrantedAccess & THREAD_GET_CONTEXT && handle_info.GrantedAccess & THREAD_SET_CONTEXT ) )
					continue;
				list_thread_idem[ tid ] = handle;
			}
		}
		wrap::virtual_free_m( handles_info, 0 );
	}
	if ( const auto proc_info = enum_system_threads( ) )
	{
		const uint32_t process_id = GetCurrentProcessId( );
		auto cur_proc = proc_info;
		do
		{
			cur_proc = r_cast<PSYSTEM_PROCESS_INFORMATION>( uintptr_t( cur_proc ) + cur_proc->NextEntryOffset );
			if ( *reinterpret_cast<uint32_t*>( &cur_proc->UniqueProcessId ) != process_id )
				continue;
			for ( DWORD t = 0; t < cur_proc->NumberOfThreads; ++t )
			{
				auto current_thread = &cur_proc->Threads[ t ];
				const auto tid		= *reinterpret_cast<uint32_t*>( &current_thread->ThreadInfo.ClientId.UniqueThread );
				auto add			= true;
				for ( auto& thread : list_thread_idem )
					if ( thread.first == tid )
					{
						add = false;
						break;
					}
				if ( add )
				{
					auto const h_thread		= wrap::open_thread( THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, tid );
					auto const access		= wrap::is_valid_handle( h_thread );
					if ( access && access & THREAD_GET_CONTEXT && access & THREAD_SET_CONTEXT )
						list_thread_idem[ tid ] = h_thread;
					else
						printf( "fail open thread[%d], status: 0x%X\n", tid, wrap::get_status( ) );
				}
			}

		} while ( cur_proc->NextEntryOffset );
		wrap::virtual_free_m( proc_info, 0 );
	}
	return ( !list_thread_idem.empty( ) );
}

std::map<uint32_t, HANDLE> threads::get_thread_list( )
{
	return list_thread_idem;
}
