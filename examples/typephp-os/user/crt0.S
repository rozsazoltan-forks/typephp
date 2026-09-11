.section .text
.code64

.global _start
.type _start, @function
.extern main
.extern _exit
_start:
    # Linux-compatible process entry stack:
    #   argc, argv[], NULL, envp[], NULL, auxv[]
    xor %ebp, %ebp
    mov (%rsp), %rdi
    lea 8(%rsp), %rsi
    lea 16(%rsp,%rdi,8), %rdx
    and $-16, %rsp
    call main

    mov %eax, %edi
    call _exit
    ud2
.size _start, . - _start

.section .note.GNU-stack, "", @progbits
