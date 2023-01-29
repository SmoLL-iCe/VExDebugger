include ksamd64.inc

extern HandleKiUserExceptionDispatcher:near
extern pKiUserExceptionDispatcherTrampo:near

.code
KiUserExceptionDispatcherAsm proc
    add rsp, 8
	call HandleKiUserExceptionDispatcher
    cmp eax, 1
    je _return
    jmp qword ptr[ pKiUserExceptionDispatcherTrampo ]
_return:
    ret
KiUserExceptionDispatcherAsm endp

end
