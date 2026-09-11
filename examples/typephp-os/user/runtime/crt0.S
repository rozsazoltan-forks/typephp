.section .text.start, "ax"
.code64

.global _start
.type _start, @function
.extern main
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
    # Linux x86-64 exit_group(2); TypePHP-OS keeps the same number and ABI.
    mov $231, %eax
    int $0x80
    ud2
.size _start, . - _start

.section .note.GNU-stack, "", @progbits
