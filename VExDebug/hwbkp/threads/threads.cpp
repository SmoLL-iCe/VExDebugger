#include "../../framework.h"
#include "threads.h"
#include "../../utils/utils.hpp"


PSYSTEM_PROCESS_INFORMATION enum_system_threads( )
{
	NTSTATUS return_val;
	auto data_length = 0x10000UL;
	PSYSTEM_PROCESS_INFORMATION proc_info;
	do
	{
		proc_info		= s_cast<PSYSTEM_PROCESS_INFORMATION>( malloc( data_length ) );
		return_val		= NtQuerySystemInformation( SystemExtendedProcessInformation, proc_info, data_length, &data_length );
		if ( return_val == STATUS_INFO_LENGTH_MISMATCH )
		{
			if ( proc_info )
				free( proc_info );
			data_length *= 2;
		}
	} while ( return_val == STATUS_INFO_LENGTH_MISMATCH );
	if ( return_val != 0 ) return nullptr;
	return proc_info;
}

PSYSTEM_HANDLE_INFORMATION enum_system_handles( )
{
	NTSTATUS return_val;
	auto data_length = 0x10000UL;
	PSYSTEM_HANDLE_INFORMATION handles_info;
	do
	{
		handles_info = s_cast<PSYSTEM_HANDLE_INFORMATION>( malloc( data_length ) );
		return_val = NtQuerySystemInformation( SystemHandleInformation, handles_info, data_length, &data_length );
		if ( return_val == STATUS_INFO_LENGTH_MISMATCH )
		{
			if ( handles_info )
				free( handles_info );
			data_length *= 2;
		}
	} while ( return_val == STATUS_INFO_LENGTH_MISMATCH );
	if ( return_val != 0 ) return nullptr;
	return handles_info;
}

std::map<uint32_t, HANDLE> list_thread_idem = {};
bool threads::update_threads( )
{
	if (auto* handles_info = enum_system_handles( ) )
	{
		for ( uint32_t i = 0; i < handles_info->NumberOfHandles; i++ )
		{
			auto const handle_info = handles_info->Handles[ i ];
			if ( s_cast<DWORD>(handle_info.UniqueProcessId) == GetCurrentProcessId( ) )
			{
				auto* const handle	= r_cast<HANDLE>( handle_info.HandleValue );
				auto const tid		= GetThreadId( handle );
				if ( !tid )
					continue;
				if ( !( handle_info.GrantedAccess & THREAD_GET_CONTEXT && handle_info.GrantedAccess & THREAD_SET_CONTEXT ) )
					continue;
				list_thread_idem[ tid ] = handle;
			}
		}
		free( handles_info );
	}
	if (auto* const proc_info = enum_system_threads( ) )
	{
		const uint32_t process_id = GetCurrentProcessId( );
		auto* cur_proc = proc_info;
		do
		{
			cur_proc = r_cast<PSYSTEM_PROCESS_INFORMATION>( r_cast<uintptr_t>( cur_proc ) + cur_proc->NextEntryOffset );
			if ( *r_cast<uint32_t*>( &cur_proc->UniqueProcessId ) != process_id )
				continue;
			for ( DWORD t = 0; t < cur_proc->NumberOfThreads; ++t )
			{
				auto* current_thread = &cur_proc->Threads[ t ];
				const auto tid		= *r_cast<uint32_t*>( &current_thread->ThreadInfo.ClientId.UniqueThread );
				auto add			= true;
				for ( auto& thread : list_thread_idem )
					if ( thread.first == tid )
					{
						add = false;
						break;
					}
				if ( add )
				{
					auto* const h_thread	= open_thread( THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, tid );
					auto const access		= is_valid_handle( h_thread );
					if ( access && access & THREAD_GET_CONTEXT && access & THREAD_SET_CONTEXT )
						list_thread_idem[ tid ] = h_thread;
					else
						printf( "fail open thread[%d]\n", tid );
				}
			}

		} while ( cur_proc->NextEntryOffset );
		free( proc_info );
	}
	return ( !list_thread_idem.empty( ) );
}

std::map<uint32_t, HANDLE> threads::get_thread_list( )
{
	return list_thread_idem;
}
