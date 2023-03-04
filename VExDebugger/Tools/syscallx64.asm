extern pAnySyscall:near

.code
SysCallAsm proc
    mov r10,rcx
    mov rax,gs:[2c0h]
    jmp qword ptr[ pAnySyscall ]
SysCallAsm  endp

end
