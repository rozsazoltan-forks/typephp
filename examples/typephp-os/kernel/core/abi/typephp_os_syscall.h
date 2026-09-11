#ifndef TYPEPHP_OS_SYSCALL_H
#define TYPEPHP_OS_SYSCALL_H

/* Keep Linux x86_64 numbers only where both the operation and its ABI match.
 * Experimental TypePHP-OS operations live in a private range so they do not
 * occupy numbers that a future glibc port expects to have Linux semantics. */
enum typephp_os_syscall_number {
    TYPEPHP_SYS_READ = 0,
    TYPEPHP_SYS_WRITE = 1,
    TYPEPHP_SYS_CLOSE = 3,
    TYPEPHP_SYS_LSEEK = 8,
    TYPEPHP_SYS_MMAP = 9,
    TYPEPHP_SYS_MPROTECT = 10,
    TYPEPHP_SYS_MUNMAP = 11,
    TYPEPHP_SYS_BRK = 12,
    TYPEPHP_SYS_EXIT = 60,
    TYPEPHP_SYS_UNAME = 63,
    TYPEPHP_SYS_GETCWD = 79,
    TYPEPHP_SYS_CHDIR = 80,
    TYPEPHP_SYS_MKDIR = 83,
    TYPEPHP_SYS_RMDIR = 84,
    TYPEPHP_SYS_UNLINK = 87,
    TYPEPHP_SYS_TIME = 201,
    TYPEPHP_SYS_OPENAT = 257,
    TYPEPHP_SYS_SPAWN = 0x54500001,
    TYPEPHP_SYS_LISTDIR = 0x54500002,
    TYPEPHP_SYS_RENAME = 0x54500003,
};

#endif
