#ifndef TYPEPHP_OS_USER_SYS_SYSCALL_H
#define TYPEPHP_OS_USER_SYS_SYSCALL_H

#include <typephp_os_syscall.h>

#define SYS_read TYPEPHP_SYS_READ
#define SYS_write TYPEPHP_SYS_WRITE
#define SYS_close TYPEPHP_SYS_CLOSE
#define SYS_lseek TYPEPHP_SYS_LSEEK
#define SYS_mmap TYPEPHP_SYS_MMAP
#define SYS_mprotect TYPEPHP_SYS_MPROTECT
#define SYS_munmap TYPEPHP_SYS_MUNMAP
#define SYS_brk TYPEPHP_SYS_BRK
#define SYS_exit TYPEPHP_SYS_EXIT
#define SYS_getcwd TYPEPHP_SYS_GETCWD
#define SYS_chdir TYPEPHP_SYS_CHDIR
#define SYS_mkdir TYPEPHP_SYS_MKDIR
#define SYS_rmdir TYPEPHP_SYS_RMDIR
#define SYS_unlink TYPEPHP_SYS_UNLINK
#define SYS_time TYPEPHP_SYS_TIME
#define SYS_openat TYPEPHP_SYS_OPENAT

/* Private bootstrap services; these deliberately do not claim Linux syscall
 * numbers or semantics. */
#define SYS_typephp_spawn TYPEPHP_SYS_SPAWN
#define SYS_typephp_listdir TYPEPHP_SYS_LISTDIR
#define SYS_typephp_rename TYPEPHP_SYS_RENAME

long syscall(long number, ...);

#endif
