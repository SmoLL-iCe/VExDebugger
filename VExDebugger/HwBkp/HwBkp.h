#pragma once
#include <windows.h>
#include <cmath>

#define SET_BITS(dr7, low_bit, bits, new_value) \
    do { \
        const auto mask = (std::intptr_t(1) << bits) - 1; \
        dr7 = (dr7 & ~(mask << low_bit)) | (std::intptr_t(new_value) << low_bit); \
    } while (0)

#define GET_BITS(dr7, low_bit, bits) \
    ((dr7 >> low_bit) & ((std::intptr_t(1) << bits) - 1))

#define IS_ENABLE_DR7_INDEX( ctx, index )  GET_BITS( ctx->Dr7, static_cast<std::uintptr_t>( index ) * 2, 1 ) == 1

#define IS_TYPE_X_DR7_INDEX( ctx, index )  GET_BITS( ctx->Dr7, static_cast<std::uintptr_t>( 16 ) + index * 4, 2 ) == 0

#define IS_TYPE_W_DR7_INDEX( ctx, index )  GET_BITS( ctx->Dr7, static_cast<std::uintptr_t>( 16 ) + index * 4, 2 ) == 1

#define IS_TYPE_RW_DR7_INDEX( ctx, index ) GET_BITS( ctx->Dr7, static_cast<std::uintptr_t>( 16 ) + index * 4, 2 ) == 3

#define SET_DR7_INDEX_TYPE( ctx, index, type ) SET_BITS( ctx->Dr7, static_cast<std::uintptr_t>( 16 ) + index * 4, 2, type );

#define SET_DR7_INDEX_SIZE( ctx, index, size ) SET_BITS( ctx->Dr7, static_cast<std::uintptr_t>( 18 ) + index * 4, 2, size );

#define SET_DR7_INDEX_ENABLE( ctx, index, onoff ) SET_BITS( ctx->Dr7, static_cast<std::uintptr_t>( index ) * 2, 1, onoff );

class HwBkp
{
private:

	uintptr_t Address			= 0;

	BkpTrigger Trigger			= BkpTrigger::Execute;

	BkpSize Size				= BkpSize::Size_1;

	int DbgRegAvailable			= 0; //max 4

	bool Add					= true;

	bool AnySuccess				= false;

public:

	HwBkp( uintptr_t Address, BkpSize Size, BkpTrigger Type, bool Add = true );

	static HwBkp* i( );

	void SetRemove( );

	bool ApplyHwbkpDebugConfig( HANDLE hThread, uint32_t ThreadId, bool useExisting = false );

	int GetPos( ) const;

	bool GetAnySuccess( ) const;

	uintptr_t GetAddress( ) const;

	BkpTrigger GetTriggerType( ) const;

	int GetTriggerCondition( ) const;

	BkpSize GetSize( ) const;

	void SetDr7Config( PCONTEXT pContext );
};