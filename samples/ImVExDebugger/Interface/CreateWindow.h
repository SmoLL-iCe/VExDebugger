#pragma once
#include <Windows.h>
#include <GL/gl3w.h> 
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GL/glcorearb.h>
#include <imgui.h>
#include <vector>

class GLWindow
{
private:

	void Routine( );

	float m_Top				= 0;

	float m_Left			= 0;

	int m_Width				= 0;

	int m_Height			= 0;

	const char * m_Caption	= nullptr;

	int m_Status			= 0;

	GLFWwindow* m_Window	= nullptr;

	HWND m_hwnd				= nullptr;

	void* m_FrameControls	= nullptr;

	bool m_ShowWindow		= true;

	bool m_Close			= false;

	static void __stdcall RunThead( GLWindow* Instance );

	static LRESULT CALLBACK WndProc( const HWND hwnd, const UINT Msg, const WPARAM wParam, const LPARAM lParam );

	std::vector<ImFont*> m_Fonts {};
public:
	GLWindow( const char * wCaption, int wWidth, int wHeight );

	void SetFrameControls( void* fControls );

	void SetSize( int wWidth, int wHeight );

	ImVec2 GetSize( );

	void SetFramePos( float wLeft, float wTop );

	ImVec2 GetFramePos( );

	void Center( );

	void Close( ) const;

	std::vector<ImFont*> GetFonts( );
};
