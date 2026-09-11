#ifndef TYPEPHP_OS_USER_UNISTD_H
#define TYPEPHP_OS_USER_UNISTD_H

#include <stddef.h>
#include <stdint.h>

typedef long ssize_t;
typedef long off_t;

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

ssize_t read(int fd, void *buffer, size_t count);
ssize_t write(int fd, const void *buffer, size_t count);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
int chdir(const char *path);
char *getcwd(char *buffer, size_t size);
int unlink(const char *path);
int rmdir(const char *path);
int brk(void *address);
void *sbrk(intptr_t increment);
void _exit(int status) __attribute__((noreturn));

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif
