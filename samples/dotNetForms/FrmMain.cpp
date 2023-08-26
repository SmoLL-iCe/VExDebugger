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
using namespace System::Collections::Generic;

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

VExDebuggerform::FrmMain^* g_MainForm = nullptr;

void __stdcall VExDebuggerform::InitForm( )
{
	Utils::OpenConsole( "" );

	VExDebugger::Init( HandlerType::UnhandledExceptionFilter, true );

	auto MainForm	= gcnew VExDebuggerform::FrmMain( );

	g_MainForm		= &MainForm;

	MainForm->Text	= "VExDebugger";

	MainForm->ShowDialog( );

	while ( true )
	{
		Sleep( 5000 );
	}
}

void DisplayListItems( System::Windows::Forms::ListBox^ CurrentListBox, ExceptionInfoList& ExcpAssoc)
{
	CurrentListBox->Items->Clear();

	for ( const auto& ExcpInfo : ExcpAssoc )
		CurrentListBox->Items->Add( Utils::CharStrToSystemStr( Utils::FormatString( "Count %08d \t Address: %p", ExcpInfo.second.Details.Count, ExcpInfo.first ) ) );
}

System::Void FrmMain::FrmMain_Load( System::Object^ sender, System::EventArgs^ e )
{
	CbType->SelectedIndex = 1;
	CbSize->SelectedIndex = 3;
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

	VExDebugger::StartMonitorAddress( ResultConverted, BkpMethod::Hardware, BkpTrigger( CbType->SelectedIndex ), BkpSize( CbSize->SelectedIndex ) );
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

void ListExceptions( )
{
	VExDebugger::CallBreakpointList( []( TBreakpointList& BreakpointList ) {

		for ( const auto& [Address, BpInfo] : BreakpointList )
		{
			if ( BpInfo.Method != BkpMethod::Hardware )
				continue;

			auto pAddress = Address;

			auto& pBpInfo = BpInfo;

			System::String^ str = Utils::CharStrToSystemStr( Utils::FormatString( "%p", pAddress ) );

			( *g_MainForm )->ChangeTPIndex( pBpInfo.Pos, str );

			VExDebugger::CallAssocExceptionList( 

				[ & ]( TAssocExceptionList AssocExceptionList ) -> void {

					auto ItExceptionList = AssocExceptionList.find( pAddress );

					if ( ItExceptionList == AssocExceptionList.end( ) )
						return;

					auto& ExceptionList = ItExceptionList->second;

					DisplayListItems( ( *g_MainForm )->GetLbFromIndex( pBpInfo.Pos ), ExceptionList );
				} 
			);
		}
	} );
}

System::Void FrmMain::ChangeTPIndex( int index, System::String^ str )
{
	tabPages[ index ]->Text = str;
}

Windows::Forms::ListBox^ FrmMain::GetLbFromIndex( int index )
{
	return listBoxes[ index ];
}

System::Void FrmMain::Timer1_Tick( System::Object^ sender, System::EventArgs^ e )
{
	ListExceptions( );
}