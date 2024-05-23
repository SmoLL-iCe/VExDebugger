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

static
void glfw_error_callback( int error, const char* description )
{
    UNREFERENCED_PARAMETER( error );
    UNREFERENCED_PARAMETER( description );
    //printf( "glfw error %d: %s\n", error, description );
}

GLWindow* g_WindowInstance = nullptr;

GLWindow::GLWindow( const char* wCaption, int wWidth, int wHeight ) : m_Width( wWidth ), m_Height( wHeight ), m_Caption( wCaption )
{
    g_WindowInstance = this;

    CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( RunThead ), this, 0, nullptr );

    while ( !m_hwnd )
        Sleep( 1 );
}

static
ImFont* AddFontFromData( const void* pData, int const nSize, const char* pName, float const fSize, const ImWchar* ranges = nullptr )
{
    ImFontConfig fontCfg   = ImFontConfig( );

    fontCfg.OversampleH    = fontCfg.OversampleV = 1;

    fontCfg.PixelSnapH     = true;

    ImGuiIO& io            = ImGui::GetIO( );

    if ( fontCfg.Name[ 0 ] == '\0' )
        strcpy_s( fontCfg.Name, pName );

    if ( fontCfg.SizePixels <= 0.0f )
        fontCfg.SizePixels = fSize;

    io.IniFilename          = "";

    return io.Fonts->AddFontFromMemoryCompressedTTF( pData, nSize, fontCfg.SizePixels, &fontCfg, ranges );
}

std::vector<ImFont*> GLWindow::GetFonts( )
{
    return m_Fonts;
}


ImVec4 Hex2FloatColor( uint32_t hex_color, const float a  = 1.f )
{
    auto* const p_byte = reinterpret_cast<uint8_t*>( &hex_color );
    const auto r = static_cast<float>( static_cast<float>( p_byte[ 2 ] ) / 255.f );
    const auto g = static_cast<float>( static_cast<float>( p_byte[ 1 ] ) / 255.f );
    const auto b = static_cast<float>( static_cast<float>( p_byte[ 0 ] ) / 255.f );
    return { r, g, b, a };
}

static
void myStyleColors( )
{
	using glColor4f = ImVec4;
	auto* const style = &ImGui::GetStyle();
	auto* const colors = style->Colors;

    auto MainColor = Hex2FloatColor( 0x15BC6B );
    auto MainColor2 = Hex2FloatColor( 0x056F3B );
    auto MainColor3 = Hex2FloatColor( 0x34C77F );
    auto BgColor = Hex2FloatColor( 0x151a21 );
    auto BgColor2 = Hex2FloatColor( 0x2a2d31 );

	style->ChildRounding					= 3.0f;
	//style->TabRounding						= 3.0f;
	//style->FrameRounding					= 3.0f;	//border radius buttons
	//style->WindowTitleAlign.x				= 0.5f; //Centraliza o titulo do menu
	//style->GrabRounding						= 3.0f;	//radius para as trackbar...
	//style->WindowRounding					= 3.0f; //Bordas do menu sem radius
	//style->WindowBorderSize					= 0.0f; //deixa uma fina borda no menu

	colors[ImGuiCol_Text]					= ImVec4(1.00f, 1.00f, 1.00f, 1.00f); //Texto por completo do menu
	colors[ImGuiCol_TextDisabled]			= ImVec4(0.50f, 0.50f, 0.50f, 1.00f); //Texto desativado por completo do menu
	colors[ImGuiCol_WindowBg]				= BgColor; //Fundo do menu
	colors[ImGuiCol_ChildBg]				= BgColor2; //Fundo do child
	colors[ImGuiCol_PopupBg]				= ImVec4(0.08f, 0.08f, 0.08f, 0.94f); //Fundo da popup/modal ou não
	colors[ImGuiCol_Border]					= ImVec4(0.44f, 0.49f, 0.56f, 0.00f); //Linha da borda menu/childs/buttons
	colors[ImGuiCol_BorderShadow]			= ImVec4(0.30f, 0.30f, 0.30f, 0.30f); //ImVec4(0.00f, 0.00f, 0.00f, 0.00f); //-- linha entre tabs 
	colors[ImGuiCol_FrameBg]				= MainColor2; //Fundo da Slider
	colors[ImGuiCol_FrameBgHovered]			= MainColor2; //Fundo da Slider mouse sobre
	colors[ImGuiCol_FrameBgActive]			= ImVec4(0.34f, 0.05f, 0.62f, 0.67f); //Fundo da Slider quando interage
	colors[ImGuiCol_TitleBg]				= BgColor2; //Fundo do titulo da str_window quando "Não" ativa
	colors[ImGuiCol_TitleBgActive]			= BgColor2; //Fundo do titulo da str_window quando ativa
	colors[ImGuiCol_TitleBgCollapsed]		= ImVec4(0.28f, 0.04f, 0.50f, 0.53f); //Fundo do titulo da str_window quando colapsada
	colors[ImGuiCol_MenuBarBg]				= ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]			= BgColor;
	colors[ImGuiCol_ScrollbarGrab]			= ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]	= ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]	= ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark]				= MainColor;
	colors[ImGuiCol_SliderGrab]				= ImVec4(1.00f, 1.00f, 1.00f, 0.60f); //Fundo do ponteiro do Slider
	colors[ImGuiCol_SliderGrabActive]		= ImVec4(1.00f, 1.00f, 1.00f, 1.00f); //Fundo do ponteiro do Slider quando ativo
	colors[ImGuiCol_Button]					= MainColor; //glColor4f(0.556f, 0.266f, 0.678f, 1.0f); //button 
	colors[ImGuiCol_ButtonHovered]			= glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	colors[ImGuiCol_ButtonActive]			= glColor4f(0.2f, 0.2f, 0.3f, 1.0f);
	colors[ImGuiCol_Header]					= MainColor;
	colors[ImGuiCol_HeaderHovered]			= MainColor2; //selected
	colors[ImGuiCol_HeaderActive]			= MainColor3;
	colors[ImGuiCol_Separator]				= colors[ImGuiCol_Border];//ImVec4(0.61f, 0.61f, 0.61f, 1.00f);//Separador
	colors[ImGuiCol_SeparatorHovered]		= ImVec4(0.10f, 0.40f, 0.75f, 0.78f); //Separador mouse sobre
	colors[ImGuiCol_SeparatorActive]		= ImVec4(0.10f, 0.40f, 0.75f, 1.00f); //Separador quando ativo
	colors[ImGuiCol_ResizeGrip]				= ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered]		= ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]		= ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	//colors[ImGuiCol_CloseButton]			= ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
	//colors[ImGuiCol_CloseButtonHovered]	= ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	//colors[ImGuiCol_CloseButtonActive]	= ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	colors[ImGuiCol_PlotLines]				= ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]		= ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]			= ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]	= ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]			= ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	//colors[ImGuiCol_ModalWindowDarkening]	= ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget]			= ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_Tab]					= ImVec4(0.15f, 0.18f, 0.23f, 1.0f);
	colors[ImGuiCol_TabActive]				= MainColor;
	colors[ImGuiCol_TabHovered]				= ImVec4(0.00f, 0.00f, 0.00f, 1.0f);
	//colors[ImGuiCol_TabClick]				= ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
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

    myStyleColors( );

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

            MoveWin = ( static_cast<float>( ClickY ) >= g_WindowInstance->m_Top && static_cast<float>( ClickY ) <= ( g_WindowInstance->m_Top + TitleBarHeight ) );

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
