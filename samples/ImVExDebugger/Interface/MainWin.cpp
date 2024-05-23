#include "MainWin.h"
#include <map>
#include <sstream>
#include <VExDebugger.h>
#include "CustomConfig.hpp"
#include <string>
#include "../Utils/Utils.h"
#include <stb_image.h>

//#define TEST_MODE

#ifndef _WIN64
#define IREG(v) "E" #v
#define REG(v) E##v
#define HEX_FMT "0x%08X"
#else
#define IREG(v) "R" #v
#define REG(v) R##v
#define HEX_FMT "0x%016llX"
#endif


#ifdef TEST_MODE
#include "../../../VExDebugger/Headers/VExInternal.h"

static
int random( int min, int max )
{
    static bool first = true;
    if ( first )
    {
        srand( (uint32_t)time( nullptr ) );
        first = false;
    }
    return min + rand( ) % ( ( max + 1 ) - min );
}

void test( ) {

    static bool bOnce = false;

    if ( bOnce )
		return;

    bOnce = true;

    const uintptr_t uTargetAddress = 0x1E005C2000;
    const uintptr_t uTriggedAddress = 0x14108F2C24; // ExceptionAddress

    VExInternal::GetBreakpointList( )[ uTargetAddress ] = {
        .Method = BkpMethod::Hardware,
        .Trigger = BkpTrigger::Execute,
        .Size = BkpSize::Size_1,
        .Pos = 0,
    };

    CONTEXT ctx = { 0 };

    ctx.ContextFlags = CONTEXT_ALL;

    GetThreadContext( GetCurrentThread( ), &ctx );

    VExInternal::GetAssocExceptionList( )[ uTargetAddress ][ uTriggedAddress ] = {
        .Details = {
			.Count = 45,
			.ThreadId = GetTickCount() % GetCurrentThreadId(),
			.Ctx = ctx,
		}
	};

}

std::uint8_t* pPageTest = nullptr;

static
void triggerPage( std::uint8_t* pPageRead )
{
    Sleep( GetTickCount( ) % GetCurrentThreadId( ) + 1000 );

    static auto nTotal = 0; 

    while ( true )
    {
        auto v = random( 1, 20 );
        if ( v != nTotal )
        {
            nTotal = v;
            break;
        }

    }

    std::uint32_t( __fastcall * pTestRead )( std::uint8_t*, std::uint32_t ) = reinterpret_cast<decltype( pTestRead )>( pPageRead );
    std::uint32_t( __fastcall * pTest )( std::uint32_t, std::uint32_t ) = reinterpret_cast<decltype( pTest )>( pPageTest );


    for ( int i = 0; i < nTotal; i++ )
    {
        std::uint32_t result = 0;

        //result = *reinterpret_cast<std::uint32_t*>( pPageTest );

        result = pTestRead( pPageTest, random( 1, 20 ) );

        //result = pTest( random( 1, 20 ), random( 1, 20 ) );

        printf( "ThreadId: %d, Result: %d\n", GetCurrentThreadId( ), result );
    }
}

static
void testPage( )
{
    static bool bOnce = false;

    if ( bOnce )
        return;

    bOnce = true;

    if ( !pPageTest )
    {
        pPageTest = reinterpret_cast<std::uint8_t*>( VirtualAlloc( nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );

        if ( !pPageTest )
            return;

        *reinterpret_cast<std::uint64_t*>( pPageTest ) = 0x000090C3D001C18B;

        const size_t szTotal = 100;

        auto uPageRead = reinterpret_cast<std::uintptr_t>( VirtualAlloc( nullptr, 0x100 * szTotal, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );

        if ( !uPageRead )
            return;

        for ( size_t i = 0; i < szTotal; i++ )
        {
            *reinterpret_cast<std::uint64_t*>( uPageRead ) = 0x000090C3D001018B; // read ptr

            CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( triggerPage ), reinterpret_cast<void*>( uPageRead ), 0, nullptr );

            uPageRead += sizeof( std::uint64_t ) + random( 1, 8 );
        }

        Sleep( 100 );

        VExDebugger::StartMonitorAddress( pPageTest, BkpMethod::PageExceptions, BkpTrigger::ReadWrite, BkpSize::Size_1 );
    }


}

#endif // TEST_MODE


void Gui::Main( GLWindow* Instance, bool * pVisible )
{
#ifdef TEST_MODE
    //test( );

    testPage( );
#endif

    static bool once = true;

    if ( once )
    {
        once = false;

        Instance->SetFramePos( 0.f, 0.f );

        Instance->SetSize( 800, 560 );

        Instance->Center( );
    }

    ImGui::SetNextWindowPos( Instance->GetFramePos(), ImGuiCond_FirstUseEver );

    ImGui::SetNextWindowSize( Instance->GetSize() );

    auto WindowFlags = 0;

    WindowFlags |= ImGuiWindowFlags_NoScrollbar;
    WindowFlags |= ImGuiWindowFlags_NoScrollWithMouse;
    WindowFlags |= ImGuiWindowFlags_NoMove;
    WindowFlags |= ImGuiWindowFlags_NoResize;
    WindowFlags |= ImGuiWindowFlags_NoCollapse;

    if ( ImGui::Begin( "VExDebugger", pVisible, WindowFlags ) )
    {
        static auto SaveLogs = false;

        ImGui::TextCustom( ImVec2( 10.f, 30.f ), { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.4f, 0.4f, 0.4f, 1.0f }, true, "VExDebugger" );

        ImGui::NewLine( );

        ImGui::NewLine( );

        ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 3.0f );

        ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 1.0f );

        static uintptr_t uTargetSelected{};
        static uintptr_t uTriggedSelected{};
        static BkpTrigger nBkpTrigger{};
        static CatchedDetails selectedCatchedDetails{};

        auto vPos = ImGui::GetCursorPos( );
        static const float fFirstTabWidth = ( ( Instance->GetSize( ).x / 100 ) * 60.f ) - 16.f;
        static const float fSecondTabWidth = ( ( Instance->GetSize( ).x / 100 ) * 40.f ) - 16.f;
        static const float fFirstTabWidthPercent = fFirstTabWidth / 100.f;
         
        if ( ImGui::BeginChild( "#panel", { fFirstTabWidth, 350.f }, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
        {
            const ImGuiTabBarFlags TabBarFlags = ImGuiTabBarFlags_None;

            if ( ImGui::BeginTabBar( "#TabBar", TabBarFlags ) )
            {
                VExDebugger::CallBreakpointList( [ ]( TBreakpointList BreakpointList ) -> void {

                    auto nInc = 0;

                    for ( const auto& [Address, BpInfo] : BreakpointList )
                    {
                        if ( !Address )
                            continue;

                        auto const HexStr = "0x" + Utils::ValToHexStr( Address );

                        if ( ImGui::BeginTabItem( ( std::to_string( ++nInc ) + " | " + HexStr ).c_str( ) ) )
                        {

                            if ( ImGui::Button( ( "Remove " + HexStr ).c_str( ), { fFirstTabWidthPercent * 93.f, 25.f } ) )
                                VExDebugger::RemoveAddress( Address, BpInfo.Method, BpInfo.Trigger );

                            VExDebugger::CallAssocExceptionList( [ & ]( TAssocExceptionList AssocExceptionList ) -> void {

                                auto ItExceptionList = AssocExceptionList.find( Address );

                                if ( ItExceptionList == AssocExceptionList.end( ) )
                                    return;

                                ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.f, 8.f ) );
                                if ( ImGui::BeginChild( "#address", { fFirstTabWidth - 16.f, 290.f }, true ) )
                                {
                                    auto& ExceptionList = ItExceptionList->second;

                                    std::string SaveLogsStr{};

                                    if ( SaveLogs )
                                        SaveLogsStr.append( "\n# List for " + HexStr + "\n" );

                                    auto NumItems = ExceptionList.size( );

                                    for ( const auto& [ExceptionAddress, ExceptionInfo] : ExceptionList )
                                    {
                                        ImGui::PushID( 18 * NumItems );

                                        const auto hue = NumItems * 0.05f;

                                        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, static_cast<ImVec4>( ImColor::HSV( hue, 0.7f, 0.7f ) ) );

                                        ImGui::PushStyleColor( ImGuiCol_ButtonActive, static_cast<ImVec4>( ImColor::HSV( hue, 0.8f, 0.8f ) ) );

                                        char pFormat[ 100 ]{};

                                        if ( BpInfo.Trigger != BkpTrigger::Execute )
                                        {
                                            sprintf_s( pFormat, sizeof( pFormat ), "Count %4d, \tAddress: 0x%p", (int)ExceptionInfo.Details.Count, (void*)ExceptionAddress );
                                        }
                                        else
                                        {
                                            sprintf_s( pFormat, sizeof( pFormat ), "Count %4d, \tThread Id: %8lld", (int)ExceptionInfo.Details.Count, static_cast<uint64_t>( ExceptionAddress ) );
                                        }

                                        if ( SaveLogs )
                                            SaveLogsStr.append( std::string( pFormat ) + "\n" );

                                        if ( ImGui::Button( pFormat, ImVec2( fFirstTabWidthPercent * 93.f, 20.f ) ) )
                                        {
                                            nBkpTrigger = BpInfo.Trigger;
                                            uTargetSelected = Address;
                                            uTriggedSelected = ExceptionAddress;
                                            selectedCatchedDetails = ExceptionInfo.Details;
                                        }

                                        ImGui::PopStyleColor( 2 );
                                        ImGui::PopID( );

                                        ++NumItems;
                                    }

                                    if ( SaveLogs )
                                    {
                                        Utils::CreateFileFromText( L"VExDebugger_" + std::wstring( HexStr.begin( ), HexStr.end( ) ) + L".log", SaveLogsStr );
                                        SaveLogsStr.clear( );
                                    }
                                }
                                ImGui::PopStyleVar( );

                                ImGui::EndChild( );

                                } );

                            ImGui::EndTabItem( );
                        }
                    }

                    } );

                if ( SaveLogs )
                {
                    SaveLogs = false;
                }

                ImGui::EndTabBar( );
            }
        }
        ImGui::EndChild( );

        const char* pTypes[ ] = {
            "Execute",
            "Read/Write","Write" };
        const char* pSizes[ ] = { "1 Byte","2 Bytes","8 Bytes","4 Bytes" };
        static int MethodCurrent = 0;
        static int TypeCurrent = 0;
        static int SizeCurrent = 0;
        static char BuffAddress[ 100 ]{};
        const float ySpace = 5.f;

        ImGui::Dummy( { 0.f, ySpace } );

        ImGui::PopStyleVar( );
        ImGui::PopStyleVar( );


        ImGui::PushID( 5722 );
        ImGui::PushItemWidth( fFirstTabWidthPercent * 30.f );
        ImGui::InputText( "", BuffAddress, sizeof( BuffAddress ) );
        ImGui::PopItemWidth( );
        ImGui::PopID( );

        ImGui::SameLine( 0.0f, fFirstTabWidthPercent * 5.f );

        ImGui::PushID( 5712 );
        ImGui::PushItemWidth( fFirstTabWidthPercent * 30.f );
        ImGui::Combo( "", &TypeCurrent, pTypes, IM_ARRAYSIZE( pTypes ) );
        ImGui::PopItemWidth( );
        ImGui::PopID( );

        ImGui::SameLine( 0.0f, fFirstTabWidthPercent * 5.f );

        ImGui::PushID( 5710 );
        ImGui::PushItemWidth( fFirstTabWidthPercent * 30.f );
        ImGui::Combo( "", &SizeCurrent, pSizes, IM_ARRAYSIZE( pSizes ) );
        ImGui::PopItemWidth( );
        ImGui::PopID( );

        ImGui::Dummy( { 0.f, ySpace } );
        ImGui::BeginGroup( );
        ImGui::RadioButton( "Hardware Breakpoints", &MethodCurrent, 0 );
        ImGui::SameLine( 0.0f, 40.f );
        ImGui::RadioButton( "Page Exceptions", &MethodCurrent, 1 );
        ImGui::EndGroup( );

        ImGui::Dummy( { 0.f, ySpace } );

        if ( ImGui::Button( "Add", { fFirstTabWidth, 20.f } ) )
        {
            std::string StrAddress( BuffAddress );

            if ( Utils::IsValidHex( StrAddress ) )
            {
                const auto ResultConverted = ( ( sizeof uintptr_t ) < 8 ) ?
                    static_cast<uintptr_t>( strtoul( StrAddress.c_str( ), nullptr, 16 ) ) : static_cast<uintptr_t>( strtoull( StrAddress.c_str( ), nullptr, 16 ) );

                if ( ResultConverted )
                {
                    VExDebugger::StartMonitorAddress( ResultConverted, static_cast<BkpMethod>( MethodCurrent ), static_cast<BkpTrigger>( TypeCurrent + 1 ), static_cast<BkpSize>( SizeCurrent ) );
                }
            }
        }

        ImGui::Dummy( { 0.f, ySpace } );

        if ( ImGui::Button( "Save logs", { fFirstTabWidth, 20.f } ) )
            SaveLogs = true;

        ImGui::SetCursorPos( { vPos.x + (fFirstTabWidth + 16.f), vPos.y } );
        if ( ImGui::BeginChild( "#panelDetails", { fSecondTabWidth, 480.f }, true ) )
        {
            //ImGui::GetCurrentWindow( )->DC.CursorPos.x = 10.f;
#define DISPLAY_CONTEXT( x ) ImGui::Text( "   - %8s, ", IREG(x) ); ImGui::SameLine( 0.0f, 0.f ); ImGui::TextColored( ImGui::GetStyle( ).Colors[ImGuiCol_Button], HEX_FMT, selectedCatchedDetails.Ctx.REG(x) );
            const float ySpace = 5.f;
            ImGui::Dummy( { 0.f, ySpace } );

            if ( nBkpTrigger != BkpTrigger::Execute )
            {
                ImGui::Text( " - Trigged Address: " ); ImGui::SameLine( 0.0f, 0.f ); ImGui::TextColored( ImGui::GetStyle( ).Colors[ ImGuiCol_Button ], HEX_FMT, uTriggedSelected );
                ImGui::Dummy( { 0.f, ySpace } );
            }
            else
            {
                ImGui::Text( " - Trigged Thread Id: "); ImGui::SameLine( 0.0f, 0.f );  ImGui::TextColored( ImGui::GetStyle( ).Colors[ ImGuiCol_Button ], "%lld", static_cast<uint64_t>( uTriggedSelected ) );
                ImGui::Dummy( { 0.f, ySpace } );
            }

            if ( uTargetSelected )
            {
                ImGui::Text( " - Target Address: " ); ImGui::SameLine( 0.0f, 0.f ); ImGui::TextColored( ImGui::GetStyle( ).Colors[ ImGuiCol_Button ], HEX_FMT, uTargetSelected );
                ImGui::Dummy( { 0.f, ySpace } );

                ImGui::Text( " - Trigged Count: " ); ImGui::SameLine( 0.0f, 0.f );  ImGui::TextColored( ImGui::GetStyle( ).Colors[ ImGuiCol_Button ], "%d", selectedCatchedDetails.Count );
                ImGui::Dummy( { 0.f, ySpace } );

                if ( nBkpTrigger != BkpTrigger::Execute )
                {
                    ImGui::Text( " - Last Thread Id: " ); ImGui::SameLine( 0.0f, 0.f );  ImGui::TextColored( ImGui::GetStyle( ).Colors[ ImGuiCol_Button ], "%d", selectedCatchedDetails.ThreadId );
                    ImGui::Dummy( { 0.f, ySpace } );
                }

                ImGui::Text( " - Last Context Registers:" );
                ImGui::Dummy( { 0.f, ySpace } );


                DISPLAY_CONTEXT( ax );

                DISPLAY_CONTEXT( cx );

                DISPLAY_CONTEXT( dx );

                DISPLAY_CONTEXT( bx );

                DISPLAY_CONTEXT( sp );

                DISPLAY_CONTEXT( bp );

                DISPLAY_CONTEXT( si );

                DISPLAY_CONTEXT( di );

#ifdef _WIN64
                DISPLAY_CONTEXT( 8 );

                DISPLAY_CONTEXT( 9 );

                DISPLAY_CONTEXT( 10 );

                DISPLAY_CONTEXT( 11 );

                DISPLAY_CONTEXT( 12 );

                DISPLAY_CONTEXT( 13 );

                DISPLAY_CONTEXT( 14 );

                DISPLAY_CONTEXT( 15 );
#endif

            }
        }
        ImGui::EndChild( );


        ImGui::End( );
    }
}

