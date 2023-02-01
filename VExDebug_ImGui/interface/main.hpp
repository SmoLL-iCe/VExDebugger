#pragma once

// for test
std::map<int, ExceptionAddressCount> ExcpAssocTests =
{
    {0, 
        { 
            { reinterpret_cast<void*>( 0x111111 ), 
                {
                    145, 15888, {}
                },
            }, 
            { reinterpret_cast<void*>( 0x1d1112 ),
                {
                    478, 12088, {}
                },
            },
            { reinterpret_cast<void*>( 0x1d1113 ),
                {
                    125, 32898, {}
                },
            },
            { reinterpret_cast<void*>( 0x1d1114 ),
                {
                    4825, 13213, {}
                },
            },
            { reinterpret_cast<void*>( 0x1d1115 ),
                {
                    941, 15888, {}
                },
            },
            { reinterpret_cast<void*>( 0x1d1116 ),
                {
                    325, 148744, {}
                },
            },
        } 
    },
    {2, 
        { 
            { reinterpret_cast<void*>( 0xAAAAAAA ),
                {
                    941, 15888, {}
                },
            },
    
            { reinterpret_cast<void*>( 0x9999999 ),
                {
                    941, 15888, {}
                },
            },

        } 
    },
    {4,  
        {
            { reinterpret_cast<void*>( 0xA919999 ),
                {
                    941, 15888, {}
                },
            },
        }
    },

};
std::map<int, uintptr_t> AddressAssocList =
{
    {0, 0x158976A},
    {2, 0xC580061},
    {4, 0},
    {8, 0}
};

namespace gui
{
    inline std::string ValToHexStr( intptr_t x )
    {
        std::stringstream stream;
        stream << std::hex << std::uppercase << x;
        return stream.str( );
    }

	inline void draw_login( gl_window* instance, bool * visible )
	{
        static bool once = true;
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
            static auto SaveLogs = false;
            ImGui::TextCustom( ImVec2( 10.f, 30.f ), { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.4f, 0.4f, 0.4f, 1.0f }, true, "VExDebug_ImGui" );
            ImGui::NewLine( );
            ImGui::NewLine( );

            ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 3.0f );
            ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 1.0f );
            ImGui::BeginChild( "#panel", { instance->get_size( ).x - 16.f, 350.f }, true );
            const ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if ( ImGui::BeginTabBar( "#TabBar", tab_bar_flags ) )
            {
                auto inc = 0;
                for ( const auto& ExcpAssoc : VExDebug::GetExceptionAssocAddress( ) )
                //for ( const auto& ExcpAssoc : ExcpAssocTests )
                {
                    const auto Address  = VExDebug::GetAddressAssocException( )[ ExcpAssoc.first ];
                    //const auto Address  = AddressAssocList[ ExcpAssoc.first ];

                    if ( !Address )
                        continue;

                    auto HexStr         = "0x" + ValToHexStr( Address );

                    if ( ImGui::BeginTabItem( ( std::to_string( ++inc ) + " | " + HexStr ).c_str( ) ) )
                    {
                        auto num_items = ExcpAssoc.first;
                       
                        if ( ImGui::Button( ( "Remove " + HexStr ).c_str() , { 320.f, 25.f } ) )
                            VExDebug::RemoveMonitorAddress( Address );

                        std::string SaveLogsStr{};

                        if ( SaveLogs )
                            SaveLogsStr.append( "\n# List for " + HexStr + "\n" );

                        for ( const auto& ExcpInfo : ExcpAssoc.second )
                        {
                            ImGui::PushID( 18 * num_items );
                            const auto hue = num_items * 0.05f;
                            ImGui::PushStyleColor( ImGuiCol_ButtonHovered,  static_cast<ImVec4>( ImColor::HSV( hue, 0.7f, 0.7f ) ) );
                            ImGui::PushStyleColor( ImGuiCol_ButtonActive,   static_cast<ImVec4>( ImColor::HSV( hue, 0.8f, 0.8f ) ) );

                            char format[ 100 ];
                            sprintf_s( format, "Count %8d Address: 0x%p", ExcpInfo.second.Count, ExcpInfo.first );

                            if ( SaveLogs )
                                SaveLogsStr.append( std::string( format ) + "\n" );
                            
                            if ( ImGui::Button( format, ImVec2( 320.f, 20.f ) ) )
                                printf( "clicked 0x%p\n", ExcpInfo.first );
                            
                            ImGui::PopStyleColor( 2 );
                            ImGui::PopID( );
                            ++num_items;
                        }

                        if ( SaveLogs )
                        {
                            utils::create_file_text( L"VExDebug_" + std::wstring( HexStr.begin(), HexStr.end() ) + L".log", SaveLogsStr );
                            SaveLogsStr.clear( );
                        }

                        ImGui::EndTabItem( );
                    }
                }
                if ( SaveLogs )
                {
                    SaveLogs = false;
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

            const char* types[] = { "Execute", "Read/Write","Write" };
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

            static bool once_address = true;

            if ( once_address )
            {
                once_address = false;
               // VExDebug::start_monitor_address( (uintptr_t)GetModuleHandle(nullptr) + 0x519198, static_cast<hw_brk_type>( 1 ), static_cast<hw_brk_size>( 0 ) );
            }
            if ( ImGui::Button( "Add", { 60.f, 20.f } ) )
            {
                std::string str_address( buff_address );   
                if ( utils::is_valided_hex( str_address ) )
                {
                    const auto result_convert = ( ( sizeof uintptr_t ) < 8 ) ?
                        static_cast<uintptr_t>( strtoul( str_address.c_str( ), nullptr, 16 ) ) : static_cast<uintptr_t>( strtoull( str_address.c_str( ), nullptr, 16 ) );
                    if ( result_convert )
                        VExDebug::StartMonitorAddress( result_convert, static_cast<HwbkpType>( type_current ), static_cast<HwbkpSize>( size_current ) );
                }
            }
            ImGui::NewLine();
            if ( ImGui::Button( "Save logs", { 354.f, 20.f } ) )
                SaveLogs = true;           
            ImGui::End( );
        }
	}

}