#define GLFW_EXPOSE_NATIVE_WIN32
#include "create_window.h"
#include "../ImGui/imgui.h"
#include <imgui_internal.h>
#include "../ImGui/glfw_opengl3/imgui_impl_glfw.h"
#include "../ImGui/glfw_opengl3/imgui_impl_opengl3.h"
#include <cstdio>
#include <iostream>
#include <map>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
LRESULT CALLBACK wnd_proc( HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param );

void glfw_error_callback( int error, const char* description )
{
    printf( "glfw error %d: %s\n", error, description );
}

gl_window* instance = nullptr;
gl_window::gl_window( const char* w_caption, int w_width, int w_height ) : m_width( w_width ), m_height( w_height ), m_caption(w_caption)
{
    instance = this;
    CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( run_thead ), this, 0, nullptr );
    while ( !h_wnd )
        Sleep( 1 );
}

inline ImFont* add_font_from_data( const void* data, int const i_size, const char* f_name, float const f_size, const ImWchar* ranges = nullptr )
{
    ImFontConfig font_cfg = ImFontConfig( );
    font_cfg.OversampleH = font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH = true;
    ImGuiIO& io = ImGui::GetIO( );
    if ( font_cfg.Name[ 0 ] == '\0' )
        strcpy_s( font_cfg.Name, f_name );

    if ( font_cfg.SizePixels <= 0.0f )
        font_cfg.SizePixels = f_size;

    io.IniFilename = "";
    return io.Fonts->AddFontFromMemoryCompressedTTF( data, i_size, font_cfg.SizePixels, &font_cfg, ranges );
}

std::vector<ImFont*> gl_window::get_fonts( )
{
    return m_fonts;
}

WNDPROC wnd_proc_old = nullptr;
void gl_window::routine( )
{
    glfwSetErrorCallback( glfw_error_callback );
    if ( !glfwInit( ) )
    {
        m_status = 1;
        return;
    }
    const auto glsl_version = "#version 130";
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
    glfwWindowHint( GLFW_DECORATED, false );
    glfwWindowHint( GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE );
    m_window = glfwCreateWindow( m_width, m_height, m_caption, nullptr, nullptr );
    if ( !m_window )
    {
        m_status = 2;
        return;
    }
    h_wnd = glfwGetWin32Window( m_window );
    SetLayeredWindowAttributes( h_wnd, 0x0, 0, LWA_COLORKEY );
    glfwSetWindowMonitor( m_window, nullptr, 100, 100, m_width, m_height, 0 );
    glfwMakeContextCurrent( m_window );
    glfwSwapInterval( 1 );
    const auto err = gl3wInit( ) != 0;
    if ( err )
    {
        m_status = 3;
        return;
    }
    ImGui::CreateContext( );
    ImGui::StyleColorsDark( );
    auto style = &ImGui::GetStyle( );
    style->FrameRounding         = 3.0f;
    style->TabRounding           = 3.0f;
    style->WindowRounding        = 3.0f;
    style->WindowTitleAlign.x    = 0.5f;
    style->Alpha                 = 1.0f;
    ImGui_ImplGlfw_InitForOpenGL( m_window, true );
    ImGui_ImplOpenGL3_Init( glsl_version );

    wnd_proc_old = reinterpret_cast<WNDPROC>( SetWindowLongPtr( h_wnd, GWLP_WNDPROC, LONG_PTR( wnd_proc ) ) );
    while ( !glfwWindowShouldClose( m_window ) )
    {
        if ( !m_show_window )
            close( );

        glfwPollEvents( );
        ImGui_ImplOpenGL3_NewFrame( );
        ImGui_ImplGlfw_NewFrame( );
        ImGui::NewFrame( );


        if ( m_frame_controls )
            reinterpret_cast<void( __stdcall* )( gl_window*, bool * )>( m_frame_controls )( this, &m_show_window );
         	
        ImGui::Render( );
        int display_w, display_h;
        glfwGetFramebufferSize( m_window, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( 0, 0, 0, 0);
        glClear( GL_COLOR_BUFFER_BIT );
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData( ) );

        glfwSwapBuffers( m_window );
    }
    ImGui_ImplOpenGL3_Shutdown( );
    ImGui_ImplGlfw_Shutdown( );
    ImGui::DestroyContext( );

    glfwDestroyWindow( m_window );
    glfwTerminate( );
}

void gl_window::close() const
{
    glfwSetWindowShouldClose( m_window, GLFW_TRUE );
}

void __stdcall gl_window::run_thead( gl_window* inst )
{
    inst->routine( );
}

void gl_window::set_frame_controls(void* f_controls)
{
    m_frame_controls = f_controls;
}

void gl_window::set_size( int w_width, int w_height )
{
    ::SetWindowPos( h_wnd, nullptr, 0, 0, w_width, w_height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
    m_width     = w_width;
    m_height    = w_height;
}

ImVec2 gl_window::get_size( )
{
    return ImVec2( float(m_width) - int( m_left ), float(m_height) - int( m_top ) );
}

void gl_window::set_frame_pos( float w_left, float w_top )
{
    m_top   = w_top;
    m_left  = w_left;
    set_size( m_width + int( w_left ), m_height + int( w_top ) );
}

ImVec2 gl_window::get_frame_pos( )
{
    return ImVec2( m_left, m_top );
}

void gl_window::center( )
{
    auto screen_cx  = GetSystemMetrics( SM_CXSCREEN );
    auto screen_cy  = GetSystemMetrics( SM_CYSCREEN );
    auto center_x   = ( screen_cx / 2 ) - ( m_width / 2 );
    auto center_y   = ( screen_cy / 2 ) - ( m_height / 2 );
    SetWindowPos( h_wnd, nullptr, center_x, center_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

LRESULT CALLBACK gl_window::wnd_proc(const HWND h_wnd, const UINT message, const WPARAM w_param, const LPARAM l_param )
{
    static int x_click;
    static int y_click;
    static auto move_window = false;
    switch ( message )
    {
    case WM_LBUTTONDOWN:
    {
        const auto title_bar_height = ImGui::GetTextLineHeight( ) + ImGui::GetStyle( ).FramePadding.y * 2.0f;
        RECT rc_window;
        if ( GetWindowRect( h_wnd, &rc_window ) )
        {
            x_click = LOWORD( l_param );
            y_click = HIWORD( l_param );
            move_window = ( static_cast<float>( y_click ) >= instance->m_top && static_cast<float>( y_click ) <= ( instance->m_top + title_bar_height ) );
            if ( move_window )
                SetCapture( h_wnd );           
        }
    }
    break;
    case WM_LBUTTONUP:
        ReleaseCapture( );
        move_window = false;
        break;
    case WM_MOUSEMOVE:
    {
        if ( GetCapture( ) == h_wnd && move_window )
        {
            POINT cur_pos;
            RECT rc_window;

            if ( !GetCursorPos( &cur_pos ) )
                break;

            if ( !GetWindowRect( h_wnd, &rc_window ) )
                break;

            if ( !ScreenToClient( h_wnd, &cur_pos ) )
                break;

            const int x_window = rc_window.left + cur_pos.x - x_click;
            const int y_window = rc_window.top + cur_pos.y - y_click;
            SetWindowPos( h_wnd, nullptr, x_window, y_window, 0, 0, SWP_NOSIZE | SWP_NOZORDER );       
        }
        break;
    }
    default:;
    }
    return CallWindowProc( wnd_proc_old, h_wnd, message, w_param, l_param );	
}
