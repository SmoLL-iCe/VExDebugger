#pragma once
#include "nt_impt.h"
#include "MinHook.h"
namespace m
{
	static auto once = true;
	template <typename A1, typename A2, typename A3>
	inline bool hook( A1 detour, A2 original, A3 & r_original )
	{
		if ( once )
			once = !( MH_Initialize( ) == MH_OK );
		const auto p_original = reinterpret_cast<void*>( original );
		auto mh_status = MH_CreateHook( p_original, reinterpret_cast<void*>( detour ), reinterpret_cast<void**>( &r_original ) );
		if ( mh_status != MH_OK )
			return false;
		mh_status = MH_EnableHook( p_original );
		return ( mh_status == MH_OK );
	}
	template <typename A1, typename A2>
	inline bool hook( A1 detour, A2& r_original )
	{
		if ( once )
			once = !( MH_Initialize( ) == MH_OK );
		auto* p_original = reinterpret_cast<void*>( r_original );
		auto mh_status = MH_CreateHook( p_original, reinterpret_cast<void*>( detour ), reinterpret_cast<void**>( &r_original ) );
		if ( mh_status != MH_OK )
			return false;
		mh_status = MH_EnableHook( p_original );
		return ( mh_status == MH_OK );
	}
	template <typename A1>
	inline bool eneble_hook( A1 r_original )
	{
		auto * p_original = reinterpret_cast<void*>( r_original );
		const auto mh_status = MH_EnableHook( p_original );
		return ( mh_status == MH_OK );
	}
	template <typename A1>
	inline bool rem_hook( A1 r_original )
	{
		auto* p_original = reinterpret_cast<void*>( r_original );
		const auto mh_status = MH_DisableHook( p_original );
		return ( mh_status == MH_OK );
	}
	template<typename T, typename V>
	inline bool apply_vtable( T v_table, V v_detour, void** original )
	{
		auto ret = false;
		auto const change_this = reinterpret_cast<uintptr_t*>( v_table );
		*original = reinterpret_cast<void*>( change_this[ 0 ] );
		DWORD p = 0;
		if ( wrap_VirtualProtect( change_this, 0x100, PAGE_EXECUTE_READWRITE, &p ) )
		{
			change_this[ 0 ] = static_cast<uintptr_t>( v_detour );
			ret = true;
			wrap_VirtualProtect( change_this, 0x100, p, &p );
		}
		return ret;
	}

	template<typename T> __forceinline T vtable_func( void* base, __int64 index )
	{
		const auto v_table = *static_cast<uintptr_t**>( base );
		return (T)v_table[ index ];
	}

	template<typename T> __forceinline T vtable_modify( void* base, __int64 index, T hk_function )
	{
		const auto v_table = static_cast<uintptr_t**>( base );
		DWORD p;
		if ( wrap_VirtualProtect( &v_table[ index ], sizeof uintptr_t, PAGE_READWRITE, &p ) )
		{
			auto o_function = v_table[ index ];
			v_table[ index ] =  static_cast<uintptr_t*>(hk_function) ;
			if ( wrap_VirtualProtect( &v_table[ index ], sizeof uintptr_t, p, &p ) )
				return (T)o_function;
		}
		return (T)0;
	}
}
