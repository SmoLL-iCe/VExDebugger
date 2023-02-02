#include <Windows.h>
#include "FrmMain.h"
#include <iostream>
#include <TlHelp32.h>
#include <vector>
#include <map>
#include <string>
#include <malloc.h>
#include "Utils/Utils.h"
#include <vcclr.h>

using namespace VExDebuggerform;
using namespace System;
using namespace System::IO;

#define MM "MD"

#define ARCH "86"
#ifdef _WIN64
#undef ARCH
#define ARCH "64"
#endif

#define RD ""
#ifdef _DEBUG
#undef RD
#define RD "d"
#endif

#define IMPORT_LIB "x" ARCH "\\VExDebugger" MM RD ".lib"

#pragma comment(lib, IMPORT_LIB)

#include <VExDebugger.h>

void DisplayListItems( System::Windows::Forms::ListBox^ CurrentListBox, ExceptionAddressCount& ExcpAssoc)
{
	CurrentListBox->Items->Clear();

	for ( const auto& ExcpInfo : ExcpAssoc )
		CurrentListBox->Items->Add( Utils::CharStrToSystemStr( Utils::FormatString( "Count %08d \t Address: %p", ExcpInfo.second.Count, ExcpInfo.first ) ) );
}


System::Void FrmMain::BtnAdd_Click( System::Object^ sender, System::EventArgs^ e )
{

	std::string AddressStr = Utils::SystemStrToCharStr( TbAddress->Text );

	if ( !Utils::IsValidHex( AddressStr ) )
	{
		Utils::MsgBox( Handle, "invalid hex address", "error", 0 );

		return;
	}

	auto ResultConverted = ( ( sizeof uintptr_t ) < 8 ) ?
		uintptr_t( strtoul( AddressStr.c_str( ), nullptr, 16 ) ) : uintptr_t( strtoull( AddressStr.c_str( ), nullptr, 16 ) );

	if ( !ResultConverted )
	{
		Utils::MsgBox( Handle, "conversion fail", "error", 0 );

		return;
	}

	VExDebugger::StartMonitorAddress( ResultConverted, HwbkpType( CbType->SelectedIndex ), HwbkpSize( CbSize->SelectedIndex ) );
}

System::Void FrmMain::BtnSave_Click( System::Object^ sender, System::EventArgs^ e )
{
	String^ path = L"C:\\log.txt";

	if ( File::Exists( path ) )
		File::Delete( path );

	auto s = File::Open( path, FileMode::CreateNew );

	s->Close( );

	StreamWriter^ wt = gcnew StreamWriter( path );

	for each ( auto var in ExcpList1->Items )
		wt->WriteLine( var );

	for each ( auto var in ExcpList2->Items )
		wt->WriteLine( var );

	for each ( auto var in ExcpList3->Items )
		wt->WriteLine( var );

	for each ( auto var in ExcpList4->Items )
		wt->WriteLine( var );

	wt->Close( );
}


System::Void FrmMain::Timer1_Tick( System::Object^ sender, System::EventArgs^ e )
{
	for ( auto& [KeyPos, ExceptionInfo] : VExDebugger::GetExceptionAssocAddress( ) )
	{
		const auto Address = reinterpret_cast<void*>( VExDebugger::GetAddressAssocException( )[ KeyPos ] );

		switch ( KeyPos )
		{
		case 0:
		{
			tp_1->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", Address ) );

			DisplayListItems( ExcpList1, ExceptionInfo );

			break;
		}
		case 2:
		{
			tp_2->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", Address ) );

			DisplayListItems( ExcpList2, ExceptionInfo );

			break;
		}
		case 4:
		{
			tp_3->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", Address ) );

			DisplayListItems( ExcpList3, ExceptionInfo );

			break;
		}
		case 8:
		{
			tp_4->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", Address ) );

			DisplayListItems( ExcpList4, ExceptionInfo );

			break;
		}
		default:
			break;
		}
	}
}