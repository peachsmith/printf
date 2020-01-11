_text segment

public extract_double_win64

extract_double_win64:
	push rbp
	mov rbp, rsp
	
	sub rsp, 32
	
	movsd qword ptr [rbp-32], xmm0
	mov rax, [rbp-32]
	
	mov rsp, rbp
	pop rbp
	ret

_text ends

end
