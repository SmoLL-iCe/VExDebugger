#include "../Headers/Header.h"
#include "../../import/include/VExDebugger.h"
#include "HwBkp.h"
#include "../Tools/Logs.h"
#include "../Tools/WinWrap.h"
#include "MgrHwBkp.h"

HwBkp* g_CurrentHwBreakPt = nullptr;

HwBkp::HwBkp( const uintptr_t Address, const BkpSize Size, const BkpTrigger Trigger, const bool Add )
{
	this->Address		= Address;

	this->Size			= Size;

	this->Trigger		= Trigger;

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

int HwBkp::GetTriggerCondition( ) const
{
	switch ( Trigger )
	{
	case BkpTrigger::Execute:
		return 0;
	case BkpTrigger::ReadWrite:
		return 3;
	case BkpTrigger::Write:
		return 1;
		break;
	}
	return 0;
}

BkpTrigger HwBkp::GetTriggerType( ) const
{
	return Trigger;
}

BkpSize HwBkp::GetSize( ) const
{
	return Size;
}

int HwBkp::GetPos( ) const
{
	return DbgRegAvailable;
}

bool HwBkp::GetAnySuccess( ) const
{
	return AnySuccess;
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
	for ( auto i = 0; i < 4; i++ )
		if ( Ctx.Dr7 & static_cast<int>( std::pow( 4, i ) ) )
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

		auto FindEmptyDr                = useExisting;

		if ( useExisting )
			pDbgReg[ DbgRegAvailable ]  = Address; //replace existing
		else
			for ( auto i = 0; i < 4; i++ )
				if ( !DrBusy[ i ] )
				{
					FindEmptyDr         = true;

					DbgRegAvailable     = i;

					pDbgReg[ i ]        = Address;

					break;
				}

		if ( !FindEmptyDr ) // not found space
		{
			Resume( );

			return false;
		}

		Ctx.Dr6				= 0;

		SetDr7Config( &Ctx );
	}

	Ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	// set changes
	if ( !WinWrap::SetContextThread( hThread, &Ctx ) )
	{
		log_file( "fail set context" );
		return false;
	}

	if ( !AnySuccess )
		AnySuccess = true;

	// run again
	Resume( );

	return true;
}

void HwBkp::SetDr7Config( PCONTEXT pContext )
{
	auto DbgCondition	= 0;

	switch ( Trigger )
	{
	case BkpTrigger::Execute:
		DbgCondition	= 0;
		break;
	case BkpTrigger::ReadWrite:
		DbgCondition	= 3;
		break;
	case BkpTrigger::Write:
		DbgCondition	= 1;
		break;
	}

	const auto eSize	= s_cast<int>( Size );

	SET_DR7_INDEX_TYPE( pContext, DbgRegAvailable, DbgCondition );

	SET_DR7_INDEX_SIZE( pContext, DbgRegAvailable, eSize );

	SET_DR7_INDEX_ENABLE( pContext, DbgRegAvailable, true );
}