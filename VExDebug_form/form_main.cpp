#include <Windows.h>
#include "form_main.h"
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <vector>
#include <map>
#include <string>

using namespace VExDebugform;
using namespace System;
using namespace System::IO;
#pragma warning(disable: 4793)
#ifdef _WIN64
#pragma comment(lib, "..\\import\\libs\\x64\\VExDebug.lib")
#pragma comment(lib, "..\\import\\libs\\x64\\winapi_wrapper.lib")
#else
#pragma comment(lib, "..\\import\\libs\\x86\\VExDebug.lib")
#pragma comment(lib, "..\\import\\libs\\x86\\winapi_wrapper.lib")
#endif

#include "../import/include/VExDebug.h"


std::string str_replace(std::string str, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

bool is_valided_hex(std::string& s)
{
	s = str_replace(s, " ", "");
	s = str_replace(s, "0x", "");
	return
		s.size() >= 2 &&
		s.find_first_not_of("0123456789abcdefABCDEF", 1) == std::string::npos;
}

char * format_string(const char* fmt, ...)
{
	if (!fmt) return nullptr;
	va_list va_a_list = {};
	va_start(va_a_list, fmt);
	auto ret = _vscprintf(fmt, va_a_list);
	auto const length = _vscprintf(fmt, va_a_list) + 1;
	auto* log_buf = new char[length];
	_vsnprintf_s(log_buf, length, _TRUNCATE, fmt, va_a_list);
	va_end(va_a_list);
	return log_buf;
}

System::String^ str(char* text)
{
	return gcnew System::String(text);
}

void to_list(System::Windows::Forms::ListBox ^ listbox, exp_address_count exp_assoc)
{
	listbox->Items->Clear();
	for (const auto exp_info : exp_assoc)
		listbox->Items->Add(str(format_string("Count %08d \t Address: %p", exp_info.second, exp_info.first)));
}

const char * to_char_array(System::String ^ text)
{
	auto us_ptr = Marshal::StringToHGlobalAnsi(text);
	auto converted_text = static_cast<char*>(us_ptr.ToPointer());
	Marshal::FreeHGlobal(us_ptr);
	return converted_text;
}

void msgbox(IntPtr handle , const char* text, const char* caption, UINT typ)
{
	static void * func_ptr = nullptr;
	if (!func_ptr)
	{
		auto mod = GetModuleHandle(L"user32.dll");
		if (!mod) return;
		func_ptr = GetProcAddress(mod, "MessageBoxA");
		if (!func_ptr) return;
	}
	reinterpret_cast<int(__stdcall*)(void*, const char*, const char*, UINT)>(func_ptr)
		(handle.ToPointer(), text, caption, typ);
}

System::Void form_main::Button1_Click(System::Object^ sender, System::EventArgs^ e)
{

	std::string address_str = to_char_array(tb_address->Text);
	if (!is_valided_hex(address_str))
	{
		msgbox(Handle, "invalid hex address", "error", 0);
		return;
	}
	auto result_convert = ((sizeof uintptr_t) < 8) ?
	uintptr_t(strtoul(address_str.c_str(), nullptr, 16)) : uintptr_t(strtoull(address_str.c_str(), nullptr, 16));
	if (!result_convert)
	{
		msgbox(Handle, "conversion fail", "error", 0);
		return;
	}
	VExDebug::start_monitor_address( result_convert, hw_brk_type( cb_type->SelectedIndex ), hw_brk_size( cb_size->SelectedIndex ) );
}
System::Void form_main::button2_Click( System::Object^ sender, System::EventArgs^ e )
{
	String^ path = L"C:\\log.txt";
	if ( File::Exists( path ) )
		File::Delete( path );
	auto s = File::Open( path, FileMode::CreateNew );
	s->Close( );
	StreamWriter^ wt = gcnew StreamWriter( path );
	for each ( auto var in exp_list_1->Items )
		wt->WriteLine( var );	
	for each ( auto var in exp_list_2->Items )
		wt->WriteLine( var );
	for each ( auto var in exp_list_3->Items )
		wt->WriteLine( var );
	for each ( auto var in exp_list_4->Items )
		wt->WriteLine( var );
	wt->Close( );
}

System::Void form_main::Form_main_Load(System::Object^ sender, System::EventArgs^ e)
{
	VExDebug::init( );
}

System::Void form_main::Timer1_Tick(System::Object^ sender, System::EventArgs^ e)
{
	for (const auto& exp_assoc : VExDebug::get_exp_assoc_address())
	{
		const auto ha_address = reinterpret_cast<void*>(VExDebug::get_address_assoc_exp()[exp_assoc.first]);
		switch (exp_assoc.first)
		{
		case 0:
		{
			tp_1->Text = str(format_string("%p", ha_address));
			to_list(exp_list_1, exp_assoc.second);
		}
		case 2:
		{
			tp_2->Text = str(format_string("%p", ha_address));
			to_list(exp_list_2, exp_assoc.second);
		}
		case 4:
		{
			tp_3->Text = str(format_string("%p", ha_address));
			to_list(exp_list_3, exp_assoc.second);
		}
		case 8:
		{
			tp_4->Text = str(format_string("%p", ha_address));
			to_list(exp_list_4, exp_assoc.second);
		}
		default:
			break;
		}
	}	
}