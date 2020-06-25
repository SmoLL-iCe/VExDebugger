#pragma once
#include <windows.h>
#include <GL/gl3w.h> 
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GL/glcorearb.h>
#include <imgui.h>
#include <vector>
class gl_window
{
private:

	void routine( );
	float m_top				= 0;
	float m_left			= 0;
	int m_width				= 0;
	int m_height			= 0;
	const char * m_caption	= nullptr;
	int m_status			= 0;
	GLFWwindow* m_window	= nullptr;
	HWND h_wnd				= nullptr;
	void* m_frame_controls	= nullptr;
	bool m_show_window		= true;
	bool m_close			= false;
	static void __stdcall run_thead( gl_window* inst );
	static LRESULT CALLBACK wnd_proc( const HWND h_wnd, const UINT message, const WPARAM w_param, const LPARAM l_param );
	std::vector<ImFont*> m_fonts {};
public:
	gl_window( const char * w_caption, int w_width, int w_height );
	void set_frame_controls( void* f_controls );
	void set_size( int w_width, int w_height );
	ImVec2 get_size( );
	void set_frame_pos( float w_left, float w_top );
	ImVec2 get_frame_pos( );
	void center( );
	void close( ) const;
	std::vector<ImFont*> get_fonts( );
};
