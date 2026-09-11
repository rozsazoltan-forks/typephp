.section .text.boot, "ax"
.code64
.global kernel64_entry
.type kernel64_entry, @function
.extern kernel_entry
.extern __bss_start
.extern __bss_end

kernel64_entry:
    cld
    mov %rdi, %r12
    mov %rsi, %r13

    # The raw payload does not carry ELF NOBITS data, so clear it explicitly.
    lea __bss_start(%rip), %rdi
    lea __bss_end(%rip), %rcx
    sub %rdi, %rcx
    xor %eax, %eax
    rep stosb

    mov %r12, %rdi
    mov %r13, %rsi
    call kernel_entry

halt64:
    cli
    hlt
    jmp halt64
.size kernel64_entry, . - kernel64_entry

.section .note.GNU-stack, "", @progbits
