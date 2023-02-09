#include <windows.h>
#include <iostream>

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

int main()
{

    CONTEXT ctx{};

    ctx.ContextFlags = CONTEXT_ALL;

    GetThreadContext(GetCurrentThread(), &ctx);

    return getchar();
}
