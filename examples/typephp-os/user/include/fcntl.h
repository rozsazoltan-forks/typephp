#ifndef TYPEPHP_OS_USER_FCNTL_H
#define TYPEPHP_OS_USER_FCNTL_H

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 64
#define O_TRUNC 512
#define O_APPEND 1024

#define AT_FDCWD (-100)
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_EACCESS 0x200

int open(const char *path, int flags, ...);
int openat(int directory_fd, const char *path, int flags, ...);

#endif
