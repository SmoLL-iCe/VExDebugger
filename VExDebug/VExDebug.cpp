#include "framework.h"
#include "../import/include/VExDebug.h"
#include "hwbkp/threads/threads.h"
#include "utils/utils.hpp"
#include "hwbkp/hw_bkp.h"
#include "veh/VEH.h"

std::vector<uint32_t> threads_id = { };
std::map<int, exp_address_count> exp_assoc_address =
{
	{ 0, { } },
	{ 2, { } },
	{ 4, { } },
	{ 8, { } }
};

std::map<int, uintptr_t> address_assoc_exp =
{
	{ 0, 0 },
	{ 2, 0 },
	{ 4, 0 },
	{ 8, 0 }
};

std::map<int, exp_address_count>& VExDebug::get_exp_assoc_address( )
{
	return exp_assoc_address;
}

std::map<int, uintptr_t>& VExDebug::get_address_assoc_exp( )
{
	return address_assoc_exp;
}

void* o_internal_handler = nullptr;
long __stdcall internal_handler( EXCEPTION_POINTERS* p_exception_info )
{
	auto* const p_exception_rec = p_exception_info->ExceptionRecord;
	if ( EXCEPTION_BREAKPOINT != p_exception_rec->ExceptionCode )
	{
		auto* const p_context = p_exception_info->ContextRecord;
		//display_context(p_context, p_exception_rec);
		if ( p_exception_rec->ExceptionCode == EXCEPTION_SINGLE_STEP )
		{
			auto const bit_index = s_cast<int>( p_context->Dr6 & 0xE );
			++exp_assoc_address[ bit_index ][ p_exception_rec->ExceptionAddress ];
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	if ( o_internal_handler )
		return reinterpret_cast<decltype( internal_handler )*>( o_internal_handler )( p_exception_info );

	return EXCEPTION_EXECUTE_HANDLER;
}

void* p_VExDebug = nullptr;
std::vector<hw_bkp*> address_added = { };
void dos_update( )
{
	threads::update_threads( );
	const auto is_empty = threads_id.empty( );
	for ( const auto& thread : threads::get_thread_list( ) )
	{
		if ( thread.first == GetCurrentThreadId( ) )
			continue;
		if ( is_empty )
		{
			threads_id.push_back( thread.first );
			continue;
		}
		if ( address_added.empty( ) )
			return;
		auto to_add = true;
		for ( auto thread_id : threads_id )
			if ( thread.first == thread_id )
			{
				to_add = true;
				break;
			}
		if ( to_add )
		{
			for ( auto* added : address_added )
				if ( is_valid_handle( thread.second ) )
				{
					added->apply_debug_control( thread.second, true );
				}
			threads_id.push_back( thread.first );
		}
	}
}

bool VExDebug::start_monitor_address( const uintptr_t address, const hw_brk_type type, const hw_brk_size size )
{
	VEH_internal::HookVEHHandlers( internal_handler, o_internal_handler );
	//if ( !p_VExDebug )
		//p_VExDebug = SetUnhandledExceptionFilter(internal_handler);
		//p_VExDebug = RtlAddVectoredExceptionHandler( 1, internal_handler );
	dos_update( );
	if ( threads_id.empty( ) )
		return false;
	auto fail_count = 0;
	new hw_bkp( address, size, type );
	for ( const auto& thread_id : threads_id )
	{
		if ( thread_id == GetCurrentThreadId( ) )
			continue;
		auto* const h_thread = threads::get_thread_list( )[ thread_id ];
		if ( is_valid_handle( h_thread ) )
		{
			if ( !hw_bkp::i( )->apply_debug_control( h_thread ) )
				++fail_count;
		}
		else printf( "fail open thread_id[%u]\n", thread_id );
	}

	for ( auto& address_assoc : address_assoc_exp )
		if ( !address_assoc.second )
		{
			address_assoc.second = address;
			break;
		}
	address_added.push_back( hw_bkp::i( ) );
	return true;
}

void VExDebug::remove_monitor_address( const uintptr_t  address )
{
	hw_bkp* hw_bkp_remove = nullptr;
	auto i_remove_hw = -1;
	for ( auto* p_hw_bkp : address_added )
	{
		++i_remove_hw;
		if ( p_hw_bkp->get_address( ) == address )
		{
			hw_bkp_remove = p_hw_bkp;
			break;
		}
	}
	if ( hw_bkp_remove )
	{
		hw_bkp_remove->add_bkp( ) = false;
		for ( auto thread_id : threads_id )
		{
			auto* const h_thread = threads::get_thread_list( )[ thread_id ];
			hw_bkp_remove->apply_debug_control( h_thread );
		}
		for ( auto& address_assoc : address_assoc_exp )
		{
			if ( address_assoc.second != address )
				continue;
			address_assoc.second = 0;
			exp_address_count( ).swap( exp_assoc_address[ address_assoc.first ] );
		}
		address_added.erase( address_added.begin( ) + i_remove_hw );
	}
}

void VExDebug::print_exceptions( )
{
	for ( const auto& exp_assoc : VExDebug::get_exp_assoc_address( ) )
	{
		auto* const ha_address = r_cast<void*>( VExDebug::get_address_assoc_exp( )[ exp_assoc.first ] );
		printf( "# => Index: %d, Address: %p\n", exp_assoc.first, ha_address );

		for ( const auto exp_info : exp_assoc.second )
			printf( "# === Count %d, Address: %p\n", exp_info.second, exp_info.first );
		printf( "\n" );
	}
}

auto base = r_cast<uint8_t*>( GetModuleHandle( nullptr ) ) + 0x1000;
void main_thread( )
{
	printf( "load\n" );
	VExDebug::start_monitor_address( r_cast<uintptr_t>( base ), hw_brk_type::hw_brk_readwrite, hw_brk_size::hw_brk_size_1 );
	Sleep( 5000 );
	for ( const auto& exp_assoc : exp_assoc_address )
	{
		auto* const ha_address = r_cast<void*>( address_assoc_exp[ exp_assoc.first ] );
		if ( !ha_address || exp_assoc.second.empty( ) )
			continue;
		printf( "== address %p, index %u\n", ha_address, exp_assoc.first );
		for ( const auto exp_info : exp_assoc.second )
			printf( "===> Exception in: %p, count: %d\n", exp_info.first, exp_info.second );
		printf( "*********************************************\n" );
	}
}

void VExDebug::init( )
{
	//wrap::create_thread_ex( r_cast<HANDLE>(-1), r_cast<void*>( main_thread ), nullptr, nullptr );
}

