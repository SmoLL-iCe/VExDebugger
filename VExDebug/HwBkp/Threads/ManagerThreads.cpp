#include "../../Headers/Header.h"
#include "ManagerThreads.h"
#include "../../Tools/Logs.h"
#include "../../Tools/WinWrap.h"

PSYSTEM_PROCESS_INFORMATION EnumSystemThreads( )
{
	NTSTATUS RetVal = 0;

	auto DataLength = 0x10000UL;

	PSYSTEM_PROCESS_INFORMATION ProcInfo = nullptr;

	do
	{
		ProcInfo		= s_cast<PSYSTEM_PROCESS_INFORMATION>( malloc( DataLength ) );

		RetVal			= NtQuerySystemInformation( SystemExtendedProcessInformation, ProcInfo, DataLength, &DataLength );

		if ( RetVal == STATUS_INFO_LENGTH_MISMATCH )
		{
			if ( ProcInfo )
				free( ProcInfo );

			DataLength *= 2;
		}
	} while ( RetVal == STATUS_INFO_LENGTH_MISMATCH );

	if ( RetVal != 0 ) 
		return nullptr;

	return ProcInfo;
}

PSYSTEM_HANDLE_INFORMATION EnumSystemHandles( )
{
	NTSTATUS RetVal = 0;

	auto DataLength = 0x10000UL;

	PSYSTEM_HANDLE_INFORMATION HandlesInfo{};
	do
	{
		HandlesInfo		= s_cast<PSYSTEM_HANDLE_INFORMATION>( malloc( DataLength ) );

		RetVal			= NtQuerySystemInformation( SystemHandleInformation, HandlesInfo, DataLength, &DataLength );

		if ( RetVal == STATUS_INFO_LENGTH_MISMATCH )
		{
			if ( HandlesInfo )
				free( HandlesInfo );

			DataLength *= 2;
		}
	} while ( RetVal == STATUS_INFO_LENGTH_MISMATCH );

	if ( RetVal != 0 ) 
		return nullptr;

	return HandlesInfo;
}

std::map<uint32_t, HANDLE> ListThreadIdem = {};

bool MgrThreads::UpdateThreads( )
{
	if ( auto* HandlesInfo = EnumSystemHandles( ) )
	{
		for ( uint32_t i = 0; i < HandlesInfo->NumberOfHandles; i++ )
		{
			auto const HandleInfo = HandlesInfo->Handles[ i ];

			if ( s_cast<DWORD>( HandleInfo.UniqueProcessId ) == GetCurrentProcessId( ) )
			{
				auto* const Handle	= r_cast<HANDLE>( HandleInfo.HandleValue );

				auto const Tid		= GetThreadId( Handle );

				if ( !Tid )
					continue;

				if ( !( HandleInfo.GrantedAccess & THREAD_GET_CONTEXT && HandleInfo.GrantedAccess & THREAD_SET_CONTEXT ) ) // needed GET/SET CONTEXT
					continue;

				ListThreadIdem[ Tid ] = Handle;
			}
		}

		free( HandlesInfo );
	}

	if ( auto* const ProcInfo = EnumSystemThreads( ) )
	{
		const uint32_t ProcessId	= GetCurrentProcessId( );

		auto* CurrentProc			= ProcInfo;

		do
		{
			CurrentProc				= r_cast<PSYSTEM_PROCESS_INFORMATION>( r_cast<uintptr_t>( CurrentProc ) + CurrentProc->NextEntryOffset );

			if ( *r_cast<uint32_t*>( &CurrentProc->UniqueProcessId ) != ProcessId )
				continue;

			for ( DWORD t = 0; t < CurrentProc->NumberOfThreads; ++t )
			{
				auto* CurrentThread = &CurrentProc->Threads[ t ];

				const auto Tid		= *r_cast<uint32_t*>( &CurrentThread->ThreadInfo.ClientId.UniqueThread );

				auto Add			= true;

				for ( auto& ThreadInf : ListThreadIdem )
					if ( ThreadInf.first == Tid )
					{
						Add = false;

						break;
					}

				if ( Add )
				{
					auto* const hThread		= WinWrap::OpenThread( THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, Tid );

					auto const Access		= WinWrap::IsValidHandle( hThread );

					if ( Access && Access & THREAD_GET_CONTEXT && Access & THREAD_SET_CONTEXT )
						ListThreadIdem[ Tid ] = hThread;
					else
						log_file( "[-] Fail open thread with get/set ctx [%d]\n", Tid );
				}
			}

		} while ( CurrentProc->NextEntryOffset );

		free( ProcInfo );
	}

	return ( !ListThreadIdem.empty( ) );
}

std::map<uint32_t, HANDLE> MgrThreads::GetThreadList( )
{
	return ListThreadIdem;
}
