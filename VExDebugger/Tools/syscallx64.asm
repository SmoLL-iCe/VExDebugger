extern pAnySyscall:near

.code
SysCallAsm proc
    mov r10,rcx
    mov rax,gs:[68h] 
    jmp qword ptr[ pAnySyscall ]
SysCallAsm  endp


;68h LastErrorValue offset 64 bits
;2c0h ExceptionCode offset 64 bits
end
