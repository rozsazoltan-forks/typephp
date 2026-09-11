.section .text
.code64

.global typephp_os_syscall_entry
.type typephp_os_syscall_entry, @function
.extern typephp_os_syscall_dispatch
typephp_os_syscall_entry:
    # Save every general-purpose register. The saved RAX slot is overwritten
    # with the syscall result before the frame is restored.
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    push %rax

    mov %rsp, %rdi
    call typephp_os_syscall_dispatch
    mov %rax, (%rsp)

    pop %rax
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    iretq
.size typephp_os_syscall_entry, . - typephp_os_syscall_entry

.global typephp_os_enter_user
.type typephp_os_enter_user, @function
typephp_os_enter_user:
    # RDI = ELF entry, RSI = top of the user stack. Interrupts remain masked;
    # int 0x80 is synchronous and the first process manager has no scheduler.
    mov $0x1b, %ax
    mov %ax, %ds
    mov %ax, %es
    pushq $0x1b
    push %rsi
    pushq $0x2
    pushq $0x23
    push %rdi
    iretq
.size typephp_os_enter_user, . - typephp_os_enter_user

.section .note.GNU-stack, "", @progbits
