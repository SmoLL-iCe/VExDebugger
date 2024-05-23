include ksamd64.inc

.code
NtProtectVirtualMemoryAsm proc

	mov r10,rcx
	mov eax,50h
	syscall 
	ret 

NtProtectVirtualMemoryAsm endp

end
