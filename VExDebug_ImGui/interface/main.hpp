#pragma once

// for test
std::map<int, exp_address_count> exp_assoc_a =
{
    {0, { 
    { reinterpret_cast<void*>( 0x111111 ), 145 }, 
    { reinterpret_cast<void*>( 0x1d1112 ), 15 },
    { reinterpret_cast<void*>( 0x1d1113 ), 15 },
    { reinterpret_cast<void*>( 0x1d1114 ), 15 },
    { reinterpret_cast<void*>( 0x1d1115 ), 15 },
    { reinterpret_cast<void*>( 0x1d1116 ), 15 },
    { reinterpret_cast<void*>( 0x1d1117 ), 15 },
    { reinterpret_cast<void*>( 0x1d1118 ), 15 },
    { reinterpret_cast<void*>( 0x1d1119 ), 15 },
    { reinterpret_cast<void*>( 0x1d1125 ), 15 },
    { reinterpret_cast<void*>( 0x1d1135 ), 15 },
    { reinterpret_cast<void*>( 0x1d1145 ), 15 },
    { reinterpret_cast<void*>( 0x1d1155 ), 15 },
    { reinterpret_cast<void*>( 0x1d1165 ), 15 },
    { reinterpret_cast<void*>( 0x1d1175 ), 15 },
    { reinterpret_cast<void*>( 0x1d1185 ), 15 },
    { reinterpret_cast<void*>( 0x1d1195 ), 15 },
    { reinterpret_cast<void*>( 0x1d11A5 ), 15 },
    { reinterpret_cast<void*>( 0x1d11B5 ), 15 },
    { reinterpret_cast<void*>( 0x1d11C5 ), 15 },
    { reinterpret_cast<void*>( 0x1d11D5 ), 15 },
    { reinterpret_cast<void*>( 0x1d11E5 ), 15 },
    { reinterpret_cast<void*>( 0x1d11F5 ), 15 },
    { reinterpret_cast<void*>( 0x1d1215 ), 15 },
    { reinterpret_cast<void*>( 0x111112 ), 45 }
    } },
    {2, { { reinterpret_cast<void*>( 0xAAAAAAA ), 81 }, { reinterpret_cast<void*>( 0x9999999 ), 39 } } },
    {4, {}},
    {8, {}}
};
std::map<int, uintptr_t> address_assoc_e =
{
    {0, 0x158976A},
    {2, 0xC580061},
    {4, 0},
    {8, 0}
};

namespace gui
{
    inline std::string to_hex( intptr_t x )
    {
        std::stringstream stream;
        stream << std::hex << std::uppercase << x;
        return stream.str( );
    }

	inline void draw_login( gl_window* instance, bool * visible )
	{
        static bool once = true;
        static ImFont* font_1 = nullptr;
        if ( once )
        {
            once = false;
            instance->set_frame_pos( 0.f, 0.f );
            instance->set_size( 370, 500 );
            instance->center( );
        }

        ImGui::SetNextWindowPos( instance->get_frame_pos(), ImGuiCond_FirstUseEver );
        ImGui::SetNextWindowSize( instance->get_size() );
        auto window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        if ( ImGui::Begin( "VExDebug", visible, window_flags ) )
        {
            static auto save_logs = false;
            ImGui::TextCustom( ImVec2( 10.f, 30.f ), { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.4f, 0.4f, 0.4f, 1.0f }, true, "VExDebug_ImGui" );
            ImGui::NewLine( );
            ImGui::NewLine( );

            ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 3.0f );
            ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 1.0f );
            ImGui::BeginChild( "#panel", { instance->get_size( ).x - 16.f, 350.f }, true );
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if ( ImGui::BeginTabBar( "#TabBar", tab_bar_flags ) )
            {
                std::string save_logs_str;
                auto inc = 0;
                for ( const auto& exp_assoc : VExDebug::get_exp_assoc_address( ) )
                //for ( const auto& exp_assoc : exp_assoc_a )
                {
                    const auto address = VExDebug::get_address_assoc_exp( )[ exp_assoc.first ];
                    //const auto address = address_assoc_e[ exp_assoc.first ];
                    if ( !address )
                        continue;
                    auto hex_address = "0x" + to_hex( address );
                    if ( ImGui::BeginTabItem( ( std::to_string( ++inc ) + " | " + hex_address ).c_str( ) ) )
                    {
                        auto num_items = exp_assoc.first;
                       
                        if ( ImGui::Button( ( "Remove " + hex_address ).c_str() , { 320.f, 25.f } ) )
                            VExDebug::remove_monitor_address( address );
                        
                        if ( save_logs )
                            save_logs_str.append( "\n# List for " + hex_address + "\n" );
                        for ( const auto exp_info : exp_assoc.second )
                        {
                            ImGui::PushID( 18 * num_items );
                            const auto hue = num_items * 0.05f;
                            ImGui::PushStyleColor( ImGuiCol_ButtonHovered,  static_cast<ImVec4>( ImColor::HSV( hue, 0.7f, 0.7f ) ) );
                            ImGui::PushStyleColor( ImGuiCol_ButtonActive,   static_cast<ImVec4>( ImColor::HSV( hue, 0.8f, 0.8f ) ) );
                            char format[ 100 ];
                            sprintf_s( format, "Count %8d Address: 0x%p", exp_info.second, exp_info.first );
                            if ( save_logs )
                                save_logs_str.append( std::string( format ) + "\n" );
                            
                            if ( ImGui::Button( format, ImVec2( 320.f, 20.f ) ) )
                                printf( "clicked 0x%p\n", exp_info.first );
                            
                            ImGui::PopStyleColor( 2 );
                            ImGui::PopID( );
                            ++num_items;
                        }
                        ImGui::EndTabItem( );
                    }
                }
                if ( save_logs )
                {
                    save_logs = false;
                    utils::create_file_text( L"VExDebug.log", save_logs_str );
                }


                ImGui::EndTabBar( );
            }      
            ImGui::EndChild( );

            ImGui::PopStyleVar( );
            ImGui::PopStyleVar( );

            static char buff_address[ 100 ];
            ImGui::PushID( 5722 );
            ImGui::PushItemWidth( 100.f );
            ImGui::InputText( "", buff_address, 100);
            ImGui::PopItemWidth( );
            ImGui::PopID( );

            ImGui::SameLine( 115.f );

            const char* types[] = { "Read/Write","Write" };
            const char* sizes[] = { "Byte 1","Byte 2","Byte 8","Byte 4" };
            static int type_current = 0;
            static int size_current = 0;

            ImGui::PushID( 5712 );
            ImGui::PushItemWidth( 100.f );
            ImGui::Combo( "", &type_current, types, IM_ARRAYSIZE( types ) );
            ImGui::PopItemWidth( );
            ImGui::PopID( );

            ImGui::SameLine( 220.f );

            ImGui::PushID( 5710 );
            ImGui::PushItemWidth( 75.f );
            ImGui::Combo( "", &size_current, sizes, IM_ARRAYSIZE( sizes ) );
            ImGui::PopItemWidth( );
            ImGui::PopID( );

            ImGui::SameLine( 300.f );

            if ( ImGui::Button( "Add", { 60.f, 20.f } ) )
            {
                std::string str_address( buff_address );   
                if ( utils::is_valided_hex( str_address ) )
                {
                    auto result_convert = ( ( sizeof uintptr_t ) < 8 ) ?
                        uintptr_t( strtoul( str_address.c_str( ), nullptr, 16 ) ) : uintptr_t( strtoull( str_address.c_str( ), nullptr, 16 ) );
                    if ( result_convert )
                        VExDebug::start_monitor_address( result_convert, static_cast<hw_brk_type>( type_current + 1 ), static_cast<hw_brk_size>( size_current ) );                    
                }
            }
            ImGui::NewLine();
            if ( ImGui::Button( "Save logs", { 354.f, 20.f } ) )
                save_logs = true;           
            ImGui::End( );
        }
	}

}