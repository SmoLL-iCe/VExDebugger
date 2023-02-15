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

VExDebuggerform::FrmMain^* pMainForm = nullptr;

void __stdcall VExDebuggerform::InitForm( )
{
	auto gMainForm = gcnew VExDebuggerform::FrmMain( );
	pMainForm = &gMainForm;
	gMainForm->Text = "VExDebugger";
	gMainForm->ShowDialog( );
}



void DisplayListItems( System::Windows::Forms::ListBox^ CurrentListBox, ExceptionInfoList& ExcpAssoc)
{
	CurrentListBox->Items->Clear();

	for ( const auto& ExcpInfo : ExcpAssoc )
		CurrentListBox->Items->Add( Utils::CharStrToSystemStr( Utils::FormatString( "Count %08d \t Address: %p", ExcpInfo.second.Details.Count, ExcpInfo.first ) ) );
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

	VExDebugger::StartMonitorAddress( ResultConverted, BkpTrigger( CbType->SelectedIndex ), BkpSize( CbSize->SelectedIndex ) );
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

void List( )
{
	VExDebugger::CallBreakpointList( []( TBreakpointList& BreakpointList ) {
		for ( const auto& [Address, BpInfo] : BreakpointList )
		{
			if ( !Address )
				continue;

			if ( BpInfo.Method != BkpMethod::Hardware )
				continue;

			auto pAddress = Address;
			auto& pBpInfo = BpInfo;

			VExDebugger::CallAssocExceptionList( 

			[ & ]( TAssocExceptionList AssocExceptionList ) -> void {

				auto ItExceptionList = AssocExceptionList.find( pAddress );

				if ( ItExceptionList == AssocExceptionList.end( ) )
					return;

				auto& ExceptionList = ItExceptionList->second;

				printf( "BpInfo.Pos", pBpInfo );
				switch ( pBpInfo.Pos )
				{
				case 0:
				{
					
					(*pMainForm)->tp_1->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", pAddress ) );

					DisplayListItems( ( *pMainForm )->ExcpList1, ExceptionList );

					break;
				}
				case 1:
				{
					( *pMainForm )->tp_2->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", pAddress ) );

					DisplayListItems( ( *pMainForm )->ExcpList2, ExceptionList );

					break;
				}
				case 2:
				{
					( *pMainForm )->tp_3->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", pAddress ) );

					DisplayListItems( ( *pMainForm )->ExcpList3, ExceptionList );

					break;
				}
				case 3:
				{
					( *pMainForm )->tp_4->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", pAddress ) );

					DisplayListItems( ( *pMainForm )->ExcpList4, ExceptionList );

					break;
				}
				default:
					break;
				}
			} );
		}
	} );
	
}

System::Void FrmMain::Timer1_Tick( System::Object^ sender, System::EventArgs^ e )
{

	List( );
	//for ( const auto& [ Address, BpInfo ] : VExDebugger::GetBreakpointList( ) )
	//{
	//	if ( !Address )
	//		continue;

	//	if ( BpInfo.Method != BkpMethod::Hardware )
	//		continue;

	//	auto ItExceptionList  = VExDebugger::GetAssocExceptionList( ).find( Address );

	//	if ( ItExceptionList == VExDebugger::GetAssocExceptionList( ).end( ) )
	//		continue;

	//	auto& ExceptionList     = ItExceptionList->second;

	//	switch ( BpInfo.Pos )
	//	{
	//	case 0:
	//	{
	//		tp_1->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", Address ) );

	//		DisplayListItems( ExcpList1, ExceptionList );

	//		break;
	//	}
	//	case 1:
	//	{
	//		tp_2->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", Address ) );

	//		DisplayListItems( ExcpList2, ExceptionList );

	//		break;
	//	}
	//	case 2:
	//	{
	//		tp_3->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", Address ) );

	//		DisplayListItems( ExcpList3, ExceptionList );

	//		break;
	//	}
	//	case 3:
	//	{
	//		tp_4->Text = Utils::CharStrToSystemStr( Utils::FormatString( "%p", Address ) );

	//		DisplayListItems( ExcpList4, ExceptionList );

	//		break;
	//	}
	//	default:
	//		break;
	//	}
	//}
}