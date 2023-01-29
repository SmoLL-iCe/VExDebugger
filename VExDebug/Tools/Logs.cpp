#include "Logs.h"
#include <stdio.h>
#include "ntos.h"
#include "../Config/Config.h"

#define DEFAULT_LOG_FILE_NAME	L"\\??\\C:\\Users\\Public\\VExDebug.log" 

bool DeleteFileF( )
{
	UNICODE_STRING		uni_name = { 0 };

	OBJECT_ATTRIBUTES	obj_attr = { 0 };

	RtlInitUnicodeString( &uni_name, DEFAULT_LOG_FILE_NAME );

	InitializeObjectAttributes( &obj_attr, &uni_name, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr );

	auto r = NtDeleteFile( &obj_attr ) == 0;

	return r;
}

bool nLog::Init( )
{
	return DeleteFileF( );
}

bool nLog::file_n( const char* sText )
{
	if ( !Config::i( )->m_Logs )
		return false;

	//printf( sText );

	IO_STATUS_BLOCK  IoStatus;
	OBJECT_ATTRIBUTES objectAttributes;
	HANDLE FileHandle;
	UNICODE_STRING fileName;

	fileName.Buffer = nullptr;
	fileName.Length = 0;
	fileName.MaximumLength = sizeof( DEFAULT_LOG_FILE_NAME ) + sizeof( UNICODE_NULL );
	fileName.Buffer = (PWCH)LocalAlloc( 0, fileName.MaximumLength );

	if ( !fileName.Buffer )
	{
		return FALSE;
	}

	RtlZeroMemory( fileName.Buffer, fileName.MaximumLength );
	auto status = RtlAppendUnicodeToString( &fileName, (PWSTR)DEFAULT_LOG_FILE_NAME );

	InitializeObjectAttributes( &objectAttributes,
		(PUNICODE_STRING)&fileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL );

	status = NtCreateFile( &FileHandle,
		FILE_APPEND_DATA | SYNCHRONIZE,
		&objectAttributes,
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
			strlen( sText ),
			NULL,
			NULL );

		NtClose( FileHandle );
	}

	if ( fileName.Buffer )
		LocalFree( fileName.Buffer );

	return true;
}

bool nLog::file( const char* szFormat, ... )
{
	if ( !Config::i( )->m_Logs )
		return false;

	char messagebuf[ 0x1000 ];

	IO_STATUS_BLOCK  IoStatus;
	OBJECT_ATTRIBUTES objectAttributes;
	HANDLE FileHandle;
	UNICODE_STRING fileName;

	ZeroMemory( messagebuf, sizeof( messagebuf ) );

	va_list va_a_list = { };

	va_start( va_a_list, szFormat );

	auto const length = _vsnprintf_s( messagebuf, sizeof( messagebuf ), _TRUNCATE, szFormat, va_a_list );

	va_end( va_a_list );

	printf( messagebuf );

	fileName.Buffer = nullptr;
	fileName.Length = 0;
	fileName.MaximumLength = sizeof( DEFAULT_LOG_FILE_NAME ) + sizeof( UNICODE_NULL );
	fileName.Buffer = (PWCH)LocalAlloc( 0, fileName.MaximumLength );

	if ( !fileName.Buffer )
	{
		return FALSE;
	}

	RtlZeroMemory( fileName.Buffer, fileName.MaximumLength );
	auto status = RtlAppendUnicodeToString( &fileName, (PWSTR)DEFAULT_LOG_FILE_NAME );

	InitializeObjectAttributes( &objectAttributes,
		(PUNICODE_STRING)&fileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL );

	status = NtCreateFile( &FileHandle,
		FILE_APPEND_DATA | SYNCHRONIZE,
		&objectAttributes,
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
			messagebuf,
			length,
			NULL,
			NULL );

		NtClose( FileHandle );
	}

	if ( fileName.Buffer )
		LocalFree( fileName.Buffer );

	return true;
}
