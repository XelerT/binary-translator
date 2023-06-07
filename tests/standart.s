section .text

global _start

_start:
        mov qword [r15], 11
        add rax, rcx
        sub rax, r15
        add [rax], rcx
        mul rcx
        mul r15
        div qword [rcx]

        mov rdx, r8
        mov [r10], rcx
        push qword [r15]
        pop qword [r15]
        pop rax
        push 255
