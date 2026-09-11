#ifndef TYPEPHP_OS_USER_SYSCALL_H
#define TYPEPHP_OS_USER_SYSCALL_H

#include <typephp_os_syscall.h>

typedef unsigned long size_t;

static inline long typephp_syscall(
    long number, long first, long second, long third)
{
    register long rax __asm__("rax") = number;
    register long rdi __asm__("rdi") = first;
    register long rsi __asm__("rsi") = second;
    register long rdx __asm__("rdx") = third;
    __asm__ volatile("int $0x80"
        : "+a"(rax)
        : "D"(rdi), "S"(rsi), "d"(rdx)
        : "rcx", "r11", "memory");
    return rax;
}

static inline size_t typephp_strlen(const char *value)
{
    size_t length = 0;
    while (value[length] != '\0') {
        ++length;
    }
    return length;
}

static inline void typephp_write_bytes(const char *value, size_t length)
{
    (void) typephp_syscall(TYPEPHP_SYS_WRITE, 1, (long) value, (long) length);
}

static inline void typephp_write(const char *value)
{
    typephp_write_bytes(value, typephp_strlen(value));
}

static inline void typephp_exit(long status)
{
    (void) typephp_syscall(TYPEPHP_SYS_EXIT, status, 0, 0);
    for (;;) {
        __asm__ volatile("pause");
    }
}

#endif
