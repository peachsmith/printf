_text segment

public extract_float_win64

extract_float_win64:
	push rbp
	mov rbp, rsp
	
	sub rsp, 32
	
	movss dword ptr [rbp-32], xmm0
	mov eax, [rbp-32]
	
	mov rsp, rbp
	pop rbp
	ret

_text ends

end
