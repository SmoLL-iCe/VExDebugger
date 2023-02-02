#define GLFW_EXPOSE_NATIVE_WIN32
#include "CreateWindow.h"
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

void glfw_error_callback( int error, const char* description )
{
    UNREFERENCED_PARAMETER( error );
    UNREFERENCED_PARAMETER( description );
    //printf( "glfw error %d: %s\n", error, description );
}

GLWindow* gWindowInstance = nullptr;

GLWindow::GLWindow( const char* wCaption, int wWidth, int wHeight ) : m_Width( wWidth ), m_Height( wHeight ), m_Caption( wCaption )
{
    gWindowInstance = this;

    CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( RunThead ), this, 0, nullptr );

    while ( !m_hwnd )
        Sleep( 1 );
}

ImFont* AddFontFromData( const void* data, int const i_size, const char* f_name, float const f_size, const ImWchar* ranges = nullptr )
{
    ImFontConfig font_cfg   = ImFontConfig( );

    font_cfg.OversampleH    = font_cfg.OversampleV = 1;

    font_cfg.PixelSnapH     = true;

    ImGuiIO& io             = ImGui::GetIO( );

    if ( font_cfg.Name[ 0 ] == '\0' )
        strcpy_s( font_cfg.Name, f_name );

    if ( font_cfg.SizePixels <= 0.0f )
        font_cfg.SizePixels = f_size;

    io.IniFilename          = "";

    return io.Fonts->AddFontFromMemoryCompressedTTF( data, i_size, font_cfg.SizePixels, &font_cfg, ranges );
}

std::vector<ImFont*> GLWindow::GetFonts( )
{
    return m_Fonts;
}

WNDPROC WndProcOld = nullptr;

void GLWindow::Routine( )
{
    glfwSetErrorCallback( glfw_error_callback );

    if ( !glfwInit( ) )
    {
        m_Status = 1;

        return;
    }

    const auto glsl_version = "#version 130";

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );

    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

    glfwWindowHint( GLFW_DECORATED, false );

    glfwWindowHint( GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE );

    m_Window = glfwCreateWindow( m_Width, m_Height, m_Caption, nullptr, nullptr );

    if ( !m_Window )
    {
        m_Status = 2;

        return;
    }

    m_hwnd = glfwGetWin32Window( m_Window );

    SetLayeredWindowAttributes( m_hwnd, 0x0, 0, LWA_COLORKEY );

    glfwSetWindowMonitor( m_Window, nullptr, 100, 100, m_Width, m_Height, 0 );

    glfwMakeContextCurrent( m_Window );

    glfwSwapInterval( 1 );

    const auto err = gl3wInit( ) != 0;

    if ( err )
    {
        m_Status = 3;

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

    ImGui_ImplGlfw_InitForOpenGL( m_Window, true );

    ImGui_ImplOpenGL3_Init( glsl_version );

    WndProcOld = reinterpret_cast<WNDPROC>( SetWindowLongPtr( m_hwnd, GWLP_WNDPROC, LONG_PTR( WndProc ) ) );

    while ( !glfwWindowShouldClose( m_Window ) )
    {
        if ( !m_ShowWindow )
            Close( );

        glfwPollEvents( );

        ImGui_ImplOpenGL3_NewFrame( );

        ImGui_ImplGlfw_NewFrame( );

        ImGui::NewFrame( );


        if ( m_FrameControls )
            reinterpret_cast<void( __stdcall* )( GLWindow*, bool * )>( m_FrameControls )( this, &m_ShowWindow );
         	
        ImGui::Render( );

        int DisplayW, DisplayH;

        glfwGetFramebufferSize( m_Window, &DisplayW, &DisplayH );

        glViewport( 0, 0, DisplayW, DisplayH );

        glClearColor( 0, 0, 0, 0);

        glClear( GL_COLOR_BUFFER_BIT );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData( ) );

        glfwSwapBuffers( m_Window );
    }

    ImGui_ImplOpenGL3_Shutdown( );

    ImGui_ImplGlfw_Shutdown( );

    ImGui::DestroyContext( );

    glfwDestroyWindow( m_Window );

    glfwTerminate( );
}

void GLWindow::Close() const
{
    glfwSetWindowShouldClose( m_Window, GLFW_TRUE );
}

void __stdcall GLWindow::RunThead( GLWindow* Instance )
{
    Instance->Routine( );
}

void GLWindow::SetFrameControls(void* fControls)
{
    m_FrameControls = fControls;
}

void GLWindow::SetSize( int wWidth, int wHeight )
{
    ::SetWindowPos( m_hwnd, nullptr, 0, 0, wWidth, wHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );

    m_Width     = wWidth;

    m_Height    = wHeight;
}

ImVec2 GLWindow::GetSize( )
{
    return ImVec2( float( m_Width ) - int( m_Left ), float( m_Height ) - int( m_Top ) );
}

void GLWindow::SetFramePos( float wLeft, float wTop )
{
    m_Top       = wTop;

    m_Left      = wLeft;

    SetSize( m_Width + int( wLeft ), m_Height + int( wTop ) );
}

ImVec2 GLWindow::GetFramePos( )
{
    return ImVec2( m_Left, m_Top );
}

void GLWindow::Center( )
{
    auto ScreenCX  = GetSystemMetrics( SM_CXSCREEN );

    auto ScreenCY  = GetSystemMetrics( SM_CYSCREEN );

    auto CenterX   = ( ScreenCX / 2 ) - ( m_Width / 2 );

    auto CenterY   = ( ScreenCY / 2 ) - ( m_Height / 2 );

    SetWindowPos( m_hwnd, nullptr, CenterX, CenterY, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

LRESULT CALLBACK GLWindow::WndProc( const HWND hwnd, const UINT Msg, const WPARAM wParam, const LPARAM lParam )
{
    static int ClickX = 0;

    static int ClickY = 0;

    static auto MoveWin = false;

    switch ( Msg )
    {
    case WM_LBUTTONDOWN:
    {
        const auto TitleBarHeight = ImGui::GetTextLineHeight( ) + ImGui::GetStyle( ).FramePadding.y * 2.0f;

        RECT RectWindow{};

        if ( GetWindowRect( hwnd, &RectWindow ) )
        {
            ClickX = LOWORD( lParam );

            ClickY = HIWORD( lParam );

            MoveWin = ( static_cast<float>( ClickY ) >= gWindowInstance->m_Top && static_cast<float>( ClickY ) <= ( gWindowInstance->m_Top + TitleBarHeight ) );

            if ( MoveWin )
                SetCapture( hwnd );           
        }
    }
    break;
    case WM_LBUTTONUP:

        ReleaseCapture( );

        MoveWin = false;

        break;
    case WM_MOUSEMOVE:
    {
        if ( GetCapture( ) == hwnd && MoveWin )
        {
            POINT CursorPos{};

            RECT RectWindow{};

            if ( !GetCursorPos( &CursorPos ) )
                break;

            if ( !GetWindowRect( hwnd, &RectWindow ) )
                break;

            if ( !ScreenToClient( hwnd, &CursorPos ) )
                break;

            const int WindowX = RectWindow.left + CursorPos.x - ClickX;

            const int WindowY = RectWindow.top + CursorPos.y - ClickY;

            SetWindowPos( hwnd, nullptr, WindowX, WindowY, 0, 0, SWP_NOSIZE | SWP_NOZORDER );       
        }
        break;
    }
    default:;
    }
    return CallWindowProc( WndProcOld, hwnd, Msg, wParam, lParam );	
}
