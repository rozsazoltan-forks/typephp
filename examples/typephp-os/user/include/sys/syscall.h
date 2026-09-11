#ifndef TYPEPHP_OS_USER_SYS_SYSCALL_H
#define TYPEPHP_OS_USER_SYS_SYSCALL_H

#include <typephp_os_syscall.h>

#define SYS_read TYPEPHP_SYS_READ
#define SYS_write TYPEPHP_SYS_WRITE
#define SYS_close TYPEPHP_SYS_CLOSE
#define SYS_stat TYPEPHP_SYS_STAT
#define SYS_fstat TYPEPHP_SYS_FSTAT
#define SYS_lstat TYPEPHP_SYS_LSTAT
#define SYS_lseek TYPEPHP_SYS_LSEEK
#define SYS_mmap TYPEPHP_SYS_MMAP
#define SYS_mprotect TYPEPHP_SYS_MPROTECT
#define SYS_munmap TYPEPHP_SYS_MUNMAP
#define SYS_brk TYPEPHP_SYS_BRK
#define SYS_access TYPEPHP_SYS_ACCESS
#define SYS_getpid TYPEPHP_SYS_GETPID
#define SYS_exit TYPEPHP_SYS_EXIT
#define SYS_uname TYPEPHP_SYS_UNAME
#define SYS_fcntl TYPEPHP_SYS_FCNTL
#define SYS_fsync TYPEPHP_SYS_FSYNC
#define SYS_fdatasync TYPEPHP_SYS_FDATASYNC
#define SYS_truncate TYPEPHP_SYS_TRUNCATE
#define SYS_ftruncate TYPEPHP_SYS_FTRUNCATE
#define SYS_getcwd TYPEPHP_SYS_GETCWD
#define SYS_chdir TYPEPHP_SYS_CHDIR
#define SYS_mkdir TYPEPHP_SYS_MKDIR
#define SYS_rmdir TYPEPHP_SYS_RMDIR
#define SYS_unlink TYPEPHP_SYS_UNLINK
#define SYS_gettimeofday TYPEPHP_SYS_GETTIMEOFDAY
#define SYS_getuid TYPEPHP_SYS_GETUID
#define SYS_getgid TYPEPHP_SYS_GETGID
#define SYS_geteuid TYPEPHP_SYS_GETEUID
#define SYS_getegid TYPEPHP_SYS_GETEGID
#define SYS_getppid TYPEPHP_SYS_GETPPID
#define SYS_gettid TYPEPHP_SYS_GETTID
#define SYS_time TYPEPHP_SYS_TIME
#define SYS_getdents64 TYPEPHP_SYS_GETDENTS64
#define SYS_clock_gettime TYPEPHP_SYS_CLOCK_GETTIME
#define SYS_clock_getres TYPEPHP_SYS_CLOCK_GETRES
#define SYS_exit_group TYPEPHP_SYS_EXIT_GROUP
#define SYS_openat TYPEPHP_SYS_OPENAT
#define SYS_newfstatat TYPEPHP_SYS_NEWFSTATAT
#define SYS_faccessat TYPEPHP_SYS_FACCESSAT

/* Private bootstrap services; these deliberately do not claim Linux syscall
 * numbers or semantics. */
#define SYS_typephp_spawn TYPEPHP_SYS_SPAWN
#define SYS_typephp_rename TYPEPHP_SYS_RENAME

long syscall(long number, ...);

#endif
