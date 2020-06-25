#define STB_IMAGE_IMPLEMENTATION
#include "../header.h"
#include "interface.h"
#include "../ImGui/imgui.h"
#include <imgui_internal.h>
#include "../ImGui/glfw_opengl3/imgui_impl_glfw.h"
#include "../ImGui/glfw_opengl3/imgui_impl_opengl3.h"
#include "create_window.h"
#include <stb_image.h>
#include "custom_config.hpp"
#include "../utils/utils.h"
#include "main.hpp"

void __stdcall frame_controls( gl_window* instance, bool* visible );
void gui::init( )
{
    auto window = new gl_window( "VExDebug_ImGui", 10, 10 );
    window->set_frame_controls( reinterpret_cast<void*>( frame_controls ) );
    window->center( );
}

int frame = -1;
void gui::set_frame( int index )
{
    frame = index;
}

int gui::get_frame( )
{
    return frame;
}

void __stdcall frame_controls( gl_window* instance, bool* visible )
{
    switch ( frame )
    {
        case 0:
        {
            gui::draw_login( instance, visible );
            break;
        }
        case 1:
        {
            break;
        }
    default:
        break;
    }
}