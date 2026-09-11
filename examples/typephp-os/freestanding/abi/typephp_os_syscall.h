#ifndef TYPEPHP_OS_SYSCALL_H
#define TYPEPHP_OS_SYSCALL_H

/* Keep familiar Linux x86_64 numbers where the operation has a close match.
 * READDIR is currently a compact TypePHP-OS directory-list operation rather
 * than Linux getdents64's binary record ABI. */
enum typephp_os_syscall_number {
    TYPEPHP_SYS_READ = 0,
    TYPEPHP_SYS_WRITE = 1,
    TYPEPHP_SYS_EXEC = 59,
    TYPEPHP_SYS_EXIT = 60,
    TYPEPHP_SYS_GETCWD = 79,
    TYPEPHP_SYS_CHDIR = 80,
    TYPEPHP_SYS_TIME = 201,
    TYPEPHP_SYS_READDIR = 217,
};

#endif
