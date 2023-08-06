#include <windows.h>
#include <iostream>
#include <vector>


// Você pode acessar os bits B0 a B3 do registrador DR6 na thread context usando C++ da seguinte maneira:
void PrintDR6Bits1(CONTEXT &ctx)
{
    DWORD dr6 = ctx.Dr6;
    std::cout << "DR6 Bits: ";
    std::cout << (dr6 & 0x01 ? "B0 " : "");
    std::cout << (dr6 & 0x02 ? "B1 " : "");
    std::cout << (dr6 & 0x04 ? "B2 " : "");
    std::cout << (dr6 & 0x08 ? "B3 " : "");
    std::cout << std::endl;
}

// Voc� pode acessar o bit BD (Bit de Dire��o) no registrador DR6 na thread context usando C++ da seguinte maneira:
void PrintDR6Bits(CONTEXT &ctx)
{
    DWORD dr6 = ctx.Dr6;
    std::cout << "DR6 Bits: ";
    std::cout << (dr6 & 0x01 ? "B0 " : "");
    std::cout << (dr6 & 0x02 ? "B1 " : "");
    std::cout << (dr6 & 0x04 ? "B2 " : "");
    std::cout << (dr6 & 0x08 ? "B3 " : "");
    std::cout << std::endl;
}

// Voc� pode acessar o bit BS (Bit de Status) no registrador DR6 na thread context usando C++ da seguinte maneira:
void PrintDR6BSBit(CONTEXT &ctx)
{
    DWORD dr6 = ctx.Dr6;
    std::cout << "DR6 BS Bit: ";
    std::cout << (dr6 & 0x8000 ? "1" : "0") << std::endl;
}

// Voc� pode acessar os bits LEN0-LEN4 (Tamanho de breakpoint) no registrador DR7 na thread context usando C++ da seguinte maneira:
void PrintDR7LEN(CONTEXT &ctx, int index)
{
    DWORD dr7 = ctx.Dr7;
    DWORD len = (dr7 >> (16 + (index * 2))) & 0x3;
    std::cout << "DR7 LEN" << index << ": ";
    switch (len)
    {
    case 0:
        std::cout << "1-byte breakpoint" << std::endl;
        break;
    case 1:
        std::cout << "2-byte breakpoint" << std::endl;
        break;
    case 2:
        std::cout << "8-byte breakpoint" << std::endl;
        break;
    case 3:
        std::cout << "4-byte breakpoint" << std::endl;
        break;
    }
}

// Voc� pode acessar os bits R/W0-R/W3 (Leitura/Escrita) no registrador DR7 na thread context usando C++ da seguinte maneira:
void PrintDR7RW(CONTEXT &ctx, int index)
{
    DWORD dr7 = ctx.Dr7;
    DWORD rw = (dr7 >> (18 + (index * 2))) & 0x3;
    std::cout << "DR7 R/W" << index << ": ";
    switch (rw)
    {
    case 0:
        std::cout << "Execute-only" << std::endl;
        break;
    case 1:
        std::cout << "Write-only" << std::endl;
        break;
    case 2:
        std::cout << "Read-only" << std::endl;
        break;
    case 3:
        std::cout << "Read/Write" << std::endl;
        break;
    }
}

// Em C++, voc� pode definir o bit TF (Trap Flag) no registrador EFLAGS da thread context para fazer um "step out" ou "step into" da seguinte maneira:

void SetEFlagsTF(CONTEXT &ctx, bool stepInto)
{
    DWORD eflags = ctx.EFlags;
    if (stepInto)
    {
        eflags |= 0x100;
    }
    else
    {
        eflags &= ~0x100;
    }
    ctx.EFlags = eflags;
}


static void msgx( )
{
    MessageBoxA( 0, "1", "2", 0 );
}

// Função para definir um breakpoint de hardware de execução em um endereço de memória na thread atual
void SetExecutionHardwareBreakpoint( void* address, DWORD threadId )
{    // Abra a thread com as permissões necessárias para modificar seu contexto
    HANDLE hThread = OpenThread( THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, threadId );
    if ( hThread == NULL )
    {
        // Trate o erro aqui, se necessário
        return;
    }

    // Suspender a execução da thread para evitar problemas de concorrência
    SuspendThread( hThread );

    CONTEXT context;
    context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    // Obter o contexto atual da CPU da thread
    GetThreadContext( hThread, &context );

    // Encontrar um registrador de depuração disponível
    int availableDebugRegister = -1;
    for ( int i = 0; i < 4; ++i )
    {
        if ( ( context.Dr7 & ( 1 << ( i * 2 ) ) ) == 0 )
        {
            availableDebugRegister = i;
            break;
        }
    }

    if ( availableDebugRegister == -1 )
    {
        // Não há registradores de depuração disponíveis
        // Você pode lidar com isso de alguma maneira, por exemplo, lançando uma exceção ou retornando um valor de erro.
        return;
    }

    // Definir o registrador de depuração disponível para o endereço de memória desejado
    DWORD_PTR* debugAddress = reinterpret_cast<DWORD_PTR*>( address );
    switch ( availableDebugRegister )
    {
    case 0:
        context.Dr0 = reinterpret_cast<DWORD_PTR>( debugAddress );
        break;
    case 1:
        context.Dr1 = reinterpret_cast<DWORD_PTR>( debugAddress );
        break;
    case 2:
        context.Dr2 = reinterpret_cast<DWORD_PTR>( debugAddress );
        break;
    case 3:
        context.Dr3 = reinterpret_cast<DWORD_PTR>( debugAddress );
        break;
    }

    // Configurar o registrador DR7 para habilitar o breakpoint de execução
    context.Dr7 |= ( 1 << ( availableDebugRegister * 2 ) ); // Habilitar o breakpoint local
    context.Dr7 &= ~( 3 << ( availableDebugRegister * 4 ) ); // Definir o tamanho do breakpoint para 1 byte (execução)
    context.Dr7 |= ( 1 << ( ( availableDebugRegister * 4 ) + 16 ) ); // Definir o tipo de breakpoint para execução (0x1)

    // Definir o contexto da CPU com as configurações atualizadas
    SetThreadContext( hThread, &context );

    // Continue a execução da thread
    ResumeThread( hThread );

    // Feche o identificador da thread
    CloseHandle( hThread );
}
bool CaughtVEHDebugger = false;

LONG CALLBACK TopLevelHandler( EXCEPTION_POINTERS* info )
{
    if ( info->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP ) //Detects when a VEH debugger single-steps through code
        CaughtVEHDebugger = true;

    printf( "Executed toplevelhandler, Exception: %X\n", info->ExceptionRecord->ExceptionCode ); //print any other exceptions we encounter
    return EXCEPTION_CONTINUE_SEARCH;
}

void thread( DWORD threadid )
{
    //SetExecutionHardwareBreakpoint( msgx, threadid );


	Sleep( 1000 );
	msgx( );
}

int main()
{
    AddVectoredExceptionHandler( 1, TopLevelHandler );

    CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( thread ), (void*)GetCurrentThreadId( ), 0, nullptr );

    Sleep( 1000 );

    msgx( );

    return getchar();
}
