.section .text
.code64

.global syscall
.type syscall, @function
syscall:
    # Translate the System V function-call ABI used by glibc's syscall(2)
    # wrapper into the Linux x86-64 syscall register ABI. The temporary
    # kernel entry is int 0x80; replacing it with `syscall` will not change
    # the C-visible contract.
    mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %r10
    mov %r9, %r8
    mov 8(%rsp), %r9
    int $0x80
    cmp $-4095, %rax
    jb 1f
    neg %eax
    mov %eax, errno(%rip)
    mov $-1, %rax
1:
    ret
.size syscall, . - syscall

.section .note.GNU-stack, "", @progbits
