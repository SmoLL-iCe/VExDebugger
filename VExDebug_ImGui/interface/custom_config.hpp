#pragma once
namespace ImGui
{
    inline auto TextCustom( ImVec2 vec, const ImVec4& col, const ImVec4& border, const bool stroke, const char* p_text, ... ) -> void
    {
        va_list va_a_list;
        char buf[ 1024 ] = { 0 };
        va_start( va_a_list, p_text );
        _vsnprintf_s( buf, sizeof buf , p_text, va_a_list );
        va_end( va_a_list );
        auto window     = ImGui::GetCurrentWindow( );
        auto draw_pos   = vec;
        auto text       = buf;    
        if ( stroke )
        {         
            window->DrawList->AddText( ImGui::GetFont( ), ImGui::GetFont( )->FontSize, ImVec2( draw_pos.x + 1, draw_pos.y + 1 ), ImGui::GetColorU32( border ), text );
            window->DrawList->AddText( ImGui::GetFont( ), ImGui::GetFont( )->FontSize, ImVec2( draw_pos.x - 1, draw_pos.y - 1 ), ImGui::GetColorU32( border ), text );
            window->DrawList->AddText( ImGui::GetFont( ), ImGui::GetFont( )->FontSize, ImVec2( draw_pos.x + 1, draw_pos.y - 1 ), ImGui::GetColorU32( border ), text );
            window->DrawList->AddText( ImGui::GetFont( ), ImGui::GetFont( )->FontSize, ImVec2( draw_pos.x - 1, draw_pos.y + 1 ), ImGui::GetColorU32( border ), text );
        }
        window->DrawList->AddText( ImGui::GetFont( ), ImGui::GetFont( )->FontSize, ImVec2( draw_pos.x, draw_pos.y ), ImGui::GetColorU32( col ), text );
    }
}