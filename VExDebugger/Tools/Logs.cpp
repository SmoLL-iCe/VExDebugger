#include "Logs.h"
#include <stdio.h>
#include "ntos.h"
#include "../Config/Config.h"

#define DEFAULT_LOG_FILE_NAME	L"\\??\\C:\\Users\\Public\\VExDebugger.log" 

bool DeleteFileF( )
{
	UNICODE_STRING		UniName = { 0 };

	OBJECT_ATTRIBUTES	ObjAttr = { 0 };

	RtlInitUnicodeString( &UniName, DEFAULT_LOG_FILE_NAME );

	InitializeObjectAttributes( &ObjAttr, &UniName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr );

	return NtDeleteFile( &ObjAttr ) == 0;
}

bool nLog::Init( )
{
	if ( !Config::i( )->m_Logs )
		return false;

	return DeleteFileF( );
}

bool nLog::file_n( const char* sText )
{
	if ( !Config::i( )->m_Logs )
		return false;

	//printf( sText );

	IO_STATUS_BLOCK		IoStatus{};

	OBJECT_ATTRIBUTES	ObjectAttributes{};

	HANDLE				FileHandle = nullptr;

	UNICODE_STRING		FileName{};

	FileName.Buffer			= nullptr;

	FileName.Length			= 0;

	FileName.MaximumLength	= sizeof( DEFAULT_LOG_FILE_NAME ) + sizeof( UNICODE_NULL );

	FileName.Buffer			= (PWCH)LocalAlloc( 0, FileName.MaximumLength );

	if ( !FileName.Buffer )
	{
		return FALSE;
	}

	RtlZeroMemory( FileName.Buffer, FileName.MaximumLength );
	auto status = RtlAppendUnicodeToString( &FileName, (PWSTR)DEFAULT_LOG_FILE_NAME );

	InitializeObjectAttributes( &ObjectAttributes,
		(PUNICODE_STRING)&FileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL );

	status = NtCreateFile( &FileHandle,
		FILE_APPEND_DATA | SYNCHRONIZE,
		&ObjectAttributes,
		&IoStatus,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0 );

	if ( NT_SUCCESS( status ) )
	{
		status = NtWriteFile( FileHandle,
			NULL,
			NULL,
			NULL,
			&IoStatus,
			(char*)sText,
			(ULONG)strlen( sText ),
			NULL,
			NULL );

		NtClose( FileHandle );
	}

	if ( FileName.Buffer )
		LocalFree( FileName.Buffer );

	return true;
}

bool nLog::file( const char* szFormat, ... )
{
	if ( !Config::i( )->m_Logs )
		return false;

	char MsgBuff[ 0x1000 ];

	IO_STATUS_BLOCK		IoStatus{};

	OBJECT_ATTRIBUTES	ObjectAttributes{};

	HANDLE				FileHandle = nullptr;

	UNICODE_STRING		FileName{};

	ZeroMemory( MsgBuff, sizeof( MsgBuff ) );

	va_list va_a_list = { };

	va_start( va_a_list, szFormat );

	auto const length = _vsnprintf_s( MsgBuff, sizeof( MsgBuff ), _TRUNCATE, szFormat, va_a_list );

	va_end( va_a_list );

	printf( MsgBuff );

	FileName.Buffer			= nullptr;

	FileName.Length			= 0;

	FileName.MaximumLength	= sizeof( DEFAULT_LOG_FILE_NAME ) + sizeof( UNICODE_NULL );

	FileName.Buffer			= (PWCH)LocalAlloc( 0, FileName.MaximumLength );

	if ( !FileName.Buffer )
	{
		return FALSE;
	}

	RtlZeroMemory( FileName.Buffer, FileName.MaximumLength );
	auto status = RtlAppendUnicodeToString( &FileName, (PWSTR)DEFAULT_LOG_FILE_NAME );

	InitializeObjectAttributes( &ObjectAttributes,
		(PUNICODE_STRING)&FileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL );

	status = NtCreateFile( &FileHandle,
		FILE_APPEND_DATA | SYNCHRONIZE,
		&ObjectAttributes,
		&IoStatus,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0 );


	if ( NT_SUCCESS( status ) )
	{
		status = NtWriteFile( FileHandle,
			NULL,
			NULL,
			NULL,
			&IoStatus,
			MsgBuff,
			length,
			NULL,
			NULL );

		NtClose( FileHandle );
	}

	if ( FileName.Buffer )
		LocalFree( FileName.Buffer );

	return true;
}
