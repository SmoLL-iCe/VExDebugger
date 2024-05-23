#include "DoHook.h"
#include "../Tools/WinWrap.h"

#define ImageNtHeader( u ) (PIMAGE_NT_HEADERS)( (UINT_PTR)u + ((PIMAGE_DOS_HEADER)u)->e_lfanew )

std::uint8_t* PageTrampo    = nullptr;
std::uint8_t* PagePoint     = nullptr;

void copyToPage( void* _Src, size_t _Size )
{
    memcpy( PagePoint, _Src, _Size );
    PagePoint += _Size;
}

template <typename T = std::uintptr_t>
void copyToPage( T _Src )
{
    memcpy( PagePoint, &_Src, sizeof( T ) );
    PagePoint += sizeof( T );
}

std::uint8_t* DoHook::GetNextPage( void* pAddress )
{
#ifdef _WIN64

    MEMORY_BASIC_INFORMATION mbi{};

    auto const uAddress     = reinterpret_cast<std::uintptr_t>( pAddress );

    std::uintptr_t MaxDis   = 0x7FFFFFFF;

    auto NextAddress        = reinterpret_cast<void*>( uAddress - MaxDis );

    SIZE_T SizeRet          = 0;

    while ( WinWrap::QueryMemory( NextAddress, MemoryBasicInformation , &mbi, sizeof(mbi), &SizeRet ) && SizeRet )
    {
        auto uPage  = reinterpret_cast<std::uintptr_t>( mbi.BaseAddress );

        NextAddress = reinterpret_cast<void*>( uPage + mbi.RegionSize );

        auto diff   = static_cast<std::uintptr_t>( _abs64( uPage - uAddress ) );

        if ( diff < MaxDis && ( ( mbi.State & MEM_FREE ) == MEM_FREE || ( mbi.State & MEM_COMMIT ) == MEM_COMMIT ) )
        {
            if ( mbi.RegionSize >= 0x1000 )
            {
                auto t = mbi.RegionSize / 0x1000;

                for ( size_t i = 0; i < t; i++ )
                {
                     auto Alloc = WinWrap::AllocMemory( reinterpret_cast<void*>( uPage + ( 0x1000 * i ) ), 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

                     if ( Alloc )
                         return reinterpret_cast<std::uint8_t*>( Alloc );
                }
            }
            else
            {
                auto Alloc = WinWrap::AllocMemory( mbi.BaseAddress, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

                if ( Alloc )
                    return reinterpret_cast<std::uint8_t*>( Alloc );
            }

        }

        if ( reinterpret_cast<std::uintptr_t>( NextAddress ) >= ( uAddress + MaxDis ) )
            break;
    }

    return nullptr;

#else
    return reinterpret_cast<std::uint8_t*>( WinWrap::AllocMemory( nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );
#endif 
}

std::vector<uint8_t> DoHook::MakeJmp( void* SrcAddress, void* DstAddress )
{
    std::vector<uint8_t> result{};

    result.reserve( 5 );

    result.resize( 5 );

    result[ 0 ] = 0xE9;

    *reinterpret_cast<uint32_t*>( &result[ 1 ] ) = ( *reinterpret_cast<uint32_t*>( &DstAddress ) - 
        *reinterpret_cast<uint32_t*>( &SrcAddress ) ) - 5;

    return result;
}

bool DoHook::SetInlineHook( void* TargetAddress, void* pDetourFunc, void** pOriginalFunc, int RestoreSize )
{
    if ( !TargetAddress || !pDetourFunc || !pOriginalFunc || !RestoreSize )
        return false;

    if ( !PageTrampo )
    {
        PageTrampo = GetNextPage( TargetAddress );

        if ( !PageTrampo )
            return false;

        PagePoint = PageTrampo;
    }

    DWORD p = 0;

    if ( !WinWrap::ProtectMemory( TargetAddress, 0x1, PAGE_EXECUTE_READWRITE, &p ) )
        return false;

    // Create Trampo

    auto OrigFunc = PagePoint;

    copyToPage( TargetAddress, RestoreSize );

    auto JmpToAfterHook = MakeJmp( PagePoint, (void*)( (std::uintptr_t)(TargetAddress)+RestoreSize ) );

    copyToPage( JmpToAfterHook.data( ), JmpToAfterHook.size( ) );


    // Create to Detour

#ifdef _WIN64

    auto NewDetour = PagePoint;

    //absolut jmp
    copyToPage<std::uint16_t>( 0x25FF ); // opcode  FF25/jmp 

    copyToPage<std::uint32_t>( 0x00000000 ); // read data offset zero

    copyToPage( pDetourFunc );  // dst address

    pDetourFunc = NewDetour;

#endif

    *pOriginalFunc = OrigFunc;

    auto TargetJmp = MakeJmp( TargetAddress, pDetourFunc );

    memcpy( TargetAddress, TargetJmp.data( ), TargetJmp.size() );

    return true;
}

size_t DoHook::GetFuncSize( void* pModule, void* pFunc )
{
    if ( !pModule || !pFunc )
        return 0;
    
    auto const      Nt           = ImageNtHeader( pModule );

    std::uintptr_t  uModuleBase  = (std::uintptr_t)pModule;

    std::uintptr_t  uModuleEnd   = uModuleBase + Nt->OptionalHeader.SizeOfImage;

    std::uintptr_t  FuncPoint    = (std::uintptr_t)( pFunc );

    if ( uModuleBase > FuncPoint || uModuleEnd < FuncPoint )
        return 0;

    size_t SizeFunc = 0;

#ifdef _WIN64

    auto RuntimeFunc = RtlLookupFunctionEntry( FuncPoint, &uModuleBase, nullptr );

    if ( !RuntimeFunc )
        return 0;

    SizeFunc = static_cast<size_t>( RuntimeFunc->EndAddress ) - RuntimeFunc->BeginAddress;

#else

    auto const EndSize  = uModuleEnd - FuncPoint;

    auto const Point    = reinterpret_cast<uint8_t*>( pFunc );

    for ( size_t i = 0xA; i < EndSize; i++ )
    {
        if ( Point[ i ] == 0xc3 || Point[ i ] == 0xc2 )
        {
            SizeFunc = i;
            break;
        }
    }

#endif // _WIN64

    return SizeFunc;
}