#include "../framework.h"
#include "../../import/include/VExDebug.h"
#include "../framework.h"
#include "hw_bkp.h"
#include "../winapi_wrapper/winapi_wrapper.h"

hw_bkp* current_hard_break = nullptr;
hw_bkp::hw_bkp( const uintptr_t address, const hw_brk_size size, const hw_brk_type type, const bool add )
{
	this->address		= address;
	this->size			= size;
	this->type			= type;
	this->add			= add;
	current_hard_break	= this;
}

hw_bkp* hw_bkp::i( )
{
	return current_hard_break;
}

bool& hw_bkp::add_bkp( )
{
	return add;
}

uintptr_t hw_bkp::get_address( ) const
{
	return address;
}

void set_bits( DWORD_PTR& dr7, const intptr_t low_bit, const intptr_t bits, const intptr_t new_value )
{
	const auto mask = ( 1 << bits ) - 1;
	dr7 = ( dr7 & ~( mask << low_bit ) ) | ( new_value << low_bit );
}

bool hw_bkp::apply_debug_control( const HANDLE h_thread, bool use_existing )
{
	const auto suspend_count = wrap::suspend_thread( h_thread ) + 1;
	if ( suspend_count == uint32_t( -1 ) )
		printf( "fail suspend thread, status 0x%X\n", wrap::get_status( ) );

	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	if ( !wrap::get_context_thread( h_thread, &context ) )
	{
		//	printf("fail get context, status 0x%X\n", nt_func::i()->get_status());
		return false;
	}
	auto const dbg_reg = &context.Dr0;
	if ( add )
		for ( auto i = 0; i < 4; ++i )
			if ( address == dbg_reg[ i ] )
			{
				for ( uint32_t l = 0; l < suspend_count; ++l )
					wrap::resume_thread( h_thread );
				return true;
			}

	auto flag_bit = 0;
	bool dr_busy[ 4 ] = { false, false, false, false };
	for ( auto i = 0, bts = 1; i < 4; i++, bts = bts * 4 )
		if ( context.Dr7 & bts )
			dr_busy[ i ] = true;

	if ( !add )
	{ // Remove
		auto const dr_num = &context.Dr0;
		for ( auto i = 0, flg_bit = 0; i < 4; i++, flg_bit = i * 2 )
			if ( i_reg_busy == i )
			{
				flag_bit		= flg_bit;
				dr_num[ i ]		= 0;
				dr_busy[ i ]	= false;
			}
		context.Dr7 &= ~( 1 << flag_bit );
	}
	else
	{ // Add
		const auto dr_num	= &context.Dr0;
		auto found			= use_existing;
		if ( use_existing )
			dr_num[ i_reg_busy ] = address;
		else
			for ( auto i = 0; i < 4; i++ )
				if ( !dr_busy[ i ] )
				{
					found		= true;
					i_reg_busy	= i;
					dr_num[ i ] = address;
					break;
				}
		if ( !found )
		{
			for ( uint32_t l = 0; l < suspend_count; ++l )
				wrap::resume_thread( h_thread );
			return false;
		}
		context.Dr6 = 0;
		auto mode_dbg = 0;
		switch ( type )
		{
		case hw_brk_type::hw_brk_execute:
			mode_dbg = 0;
			break;
		case hw_brk_type::hw_brk_readwrite:
			mode_dbg = 3;
			break;
		case hw_brk_type::hw_brk_write:
			mode_dbg = 1;
			break;
		}
		const auto i_size = int( size );
		set_bits( context.Dr7, 16 + i_reg_busy * 4, 2, mode_dbg );
		set_bits( context.Dr7, 18 + i_reg_busy * 4, 2, i_size );
		set_bits( context.Dr7, i_reg_busy * 2, 1, 1 );
	}

	context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	if ( !wrap::set_context_thread( h_thread, &context ) )
	{
		printf( "fail set context, status 0x%X\n", wrap::get_status( ) );
		return false;
	}
	for ( uint32_t l = 0; l < suspend_count; ++l )
		wrap::resume_thread( h_thread );
	return true;
}