#include "../Headers/Header.h"
#include "../../import/include/VExDebugger.h"
#include "HwBkp.h"
#include "../Tools/Logs.h"
#include "../Tools/WinWrap.h"


#define SET_TRAP_FLAG(ctx) ctx->EFlags |= (1 << 8);
#define UNSET_TRAP_FLAG(ctx) ctx->EFlags &= ~(1 << 8);

HwBkp* g_CurrentHwBreakPt = nullptr;

HwBkp::HwBkp( const uintptr_t Address, const HwbkpSize Size, const HwbkpType Type, const bool Add )
{
	this->Address		= Address;

	this->Size			= Size;

	this->Type			= Type;

	this->Add			= Add;

	g_CurrentHwBreakPt	= this;
}

HwBkp* HwBkp::i( )
{
	return g_CurrentHwBreakPt;
}

void HwBkp::SetRemove( )
{
	Add = false;
}

uintptr_t HwBkp::GetAddress( ) const
{
	return Address;
}

int HwBkp::GetPos( ) const
{
	return DbgRegAvailable;
}

bool HwBkp::GetAnySuccess( ) const
{
	return AnySuccess;
}

#ifdef _WIN64
void setBits( uintptr_t& dr7, const intptr_t low_bit, const intptr_t bits, const intptr_t new_value )
#else
void setBits( DWORD& dr7, const intptr_t low_bit, const intptr_t bits, const intptr_t new_value )
#endif
{
	const auto mask = ( 1 << bits ) - 1;
	dr7 = ( dr7 & ~( mask << low_bit ) ) | ( new_value << low_bit );
}

bool HwBkp::ApplyHwbkpDebugConfig( const HANDLE hThread, uint32_t ThreadId, bool useExisting )
{
	const auto SuspendCount = WinWrap::SuspendThread( hThread ) + 1; // suspend thread for change dbg registers

	if ( SuspendCount == s_cast<uint32_t>( -1 ) )
		log_file( "[-] Fail suspend thread %d", ThreadId ); // report weird behavior

	CONTEXT Ctx			= { 0 };
	 
	Ctx.ContextFlags	= CONTEXT_DEBUG_REGISTERS;

	if ( !WinWrap::GetContextThread( hThread, &Ctx ) )
	{
		log_file( "[-] Fail get context, status 0x%X\n", WinWrap::GetErrorStatus( ) );

		return false;
	}

	auto Resume = [&]( )
	{
		for ( uint32_t l = 0; l < SuspendCount; ++l )
			WinWrap::ResumeThread( hThread );
	};

	auto* const pDbgReg = &Ctx.Dr0;

	if ( Add )
		for ( auto i = 0; i < 4; ++i )
			if ( Address == pDbgReg[ i ] ) // already exist
			{
				Resume( );

				return true;
			}

	auto bFlagPos = 0;

	bool DrBusy[ 4 ] = { false, false, false, false };

	//Max Capacity is 4
	for ( auto i = 0, bts = 1; i < 4; i++, bts = bts * 4 )
		if ( Ctx.Dr7 & bts )
			DrBusy[ i ] = true;

	if ( !Add )
	{ // Remove

		for ( auto i = 0, fPos = 0; i < 4; i++, fPos = i * 2 )
			if ( DbgRegAvailable == i )
			{
				bFlagPos		= fPos;

				pDbgReg[ i ]	= 0;

				DrBusy[ i ]		= false;

			}

		Ctx.Dr7 &= ~( 1 << bFlagPos );
	}
	else
	{ // Add

		auto FindEmptyDr			= useExisting;

		if ( useExisting )
			pDbgReg[ DbgRegAvailable ]	= Address; //replace existing
		else
			for ( auto i = 0; i < 4; i++ )
				if ( !DrBusy[ i ] )
				{
					FindEmptyDr			= true;

					DbgRegAvailable		= i;

					pDbgReg[ i ]	= Address;

					break;
				}

		if ( !FindEmptyDr ) // not found space
		{
			Resume( );

			return false;
		}

		Ctx.Dr6				= 0;

		auto DbgCondition	= 0;

		switch ( Type )
		{
		case HwbkpType::Execute:
			DbgCondition	= 0;
			break;
		case HwbkpType::ReadWrite:
			DbgCondition	= 3;
			break;
		case HwbkpType::Write:
			DbgCondition	= 1;
			break;
		}

		const auto eSize	= s_cast<int>( Size );

		setBits( Ctx.Dr7, 16 + DbgRegAvailable * 4, 2, DbgCondition );	// set dbg type

		setBits( Ctx.Dr7, 18 + DbgRegAvailable * 4, 2, eSize		);	// set dbg data size

		setBits( Ctx.Dr7, DbgRegAvailable * 2, 1, 1 );
	}

	Ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	// set changes
	if ( !WinWrap::SetContextThread( hThread, &Ctx ) )
	{
		printf( "fail set context\n" );
		return false;
	}

	if ( !AnySuccess )
		AnySuccess = true;

	// run again
	Resume( );

	return true;
}