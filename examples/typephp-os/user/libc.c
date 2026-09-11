#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

int errno;

static char unknown_error[] = "Unknown error";

size_t strlen(const char *string)
{
    size_t length = 0;
    while (string[length] != '\0') {
        ++length;
    }
    return length;
}

char *strerror(int error)
{
    switch (error) {
    case ENOENT: return "No such file or directory";
    case EIO: return "Input/output error";
    case ENOEXEC: return "Exec format error";
    case EBADF: return "Bad file descriptor";
    case ENOMEM: return "Cannot allocate memory";
    case EACCES: return "Permission denied";
    case EFAULT: return "Bad address";
    case EEXIST: return "File exists";
    case ENOTDIR: return "Not a directory";
    case EISDIR: return "Is a directory";
    case EINVAL: return "Invalid argument";
    case EMFILE: return "Too many open files";
    case EROFS: return "Read-only file system";
    case ERANGE: return "Numerical result out of range";
    case ENAMETOOLONG: return "File name too long";
    case ENOSYS: return "Function not implemented";
    case ENOTEMPTY: return "Directory not empty";
    default: return unknown_error;
    }
}

void perror(const char *prefix)
{
    if (prefix != NULL && prefix[0] != '\0') {
        (void) write(STDERR_FILENO, prefix, strlen(prefix));
        (void) write(STDERR_FILENO, ": ", 2);
    }
    (void) write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
    (void) write(STDERR_FILENO, "\n", 1);
}

ssize_t read(int fd, void *buffer, size_t count)
{
    return (ssize_t) syscall(SYS_read, fd, buffer, count);
}

ssize_t write(int fd, const void *buffer, size_t count)
{
    return (ssize_t) syscall(SYS_write, fd, buffer, count);
}

int close(int fd)
{
    return (int) syscall(SYS_close, fd);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return (off_t) syscall(SYS_lseek, fd, offset, whence);
}

int openat(int directory_fd, const char *path, int flags, ...)
{
    int mode = 0;
    if ((flags & O_CREAT) != 0) {
        va_list arguments;
        va_start(arguments, flags);
        mode = va_arg(arguments, int);
        va_end(arguments);
    }
    return (int) syscall(SYS_openat, directory_fd, path, flags, mode);
}

int open(const char *path, int flags, ...)
{
    int mode = 0;
    if ((flags & O_CREAT) != 0) {
        va_list arguments;
        va_start(arguments, flags);
        mode = va_arg(arguments, int);
        va_end(arguments);
    }
    return openat(AT_FDCWD, path, flags, mode);
}

int chdir(const char *path)
{
    return (int) syscall(SYS_chdir, path);
}

int mkdir(const char *path, mode_t mode)
{
    return (int) syscall(SYS_mkdir, path, mode);
}

int unlink(const char *path)
{
    return (int) syscall(SYS_unlink, path);
}

int rmdir(const char *path)
{
    return (int) syscall(SYS_rmdir, path);
}

int rename(const char *old_path, const char *new_path)
{
    return (int) syscall(SYS_typephp_rename, old_path, new_path);
}

int brk(void *address)
{
    long result = syscall(SYS_brk, address);
    if ((void *) result != address) {
        errno = ENOMEM;
        return -1;
    }
    return 0;
}

void *sbrk(intptr_t increment)
{
    intptr_t current = syscall(SYS_brk, 0);
    intptr_t requested;
    if ((increment > 0 && current > INTPTR_MAX - increment)
        || (increment < 0 && current < INTPTR_MIN - increment)) {
        errno = ENOMEM;
        return (void *) -1;
    }
    requested = current + increment;
    if (increment != 0 && syscall(SYS_brk, requested) != requested) {
        errno = ENOMEM;
        return (void *) -1;
    }
    return (void *) current;
}

void *mmap(void *address, size_t length, int protection, int flags, int fd, long offset)
{
    return (void *) syscall(SYS_mmap,
        address, length, protection, flags, fd, offset);
}

int mprotect(void *address, size_t length, int protection)
{
    return (int) syscall(SYS_mprotect, address, length, protection);
}

int munmap(void *address, size_t length)
{
    return (int) syscall(SYS_munmap, address, length);
}

char *getcwd(char *buffer, size_t size)
{
    return syscall(SYS_getcwd, buffer, size) < 0 ? NULL : buffer;
}

time_t time(time_t *result)
{
    return (time_t) syscall(SYS_time, result);
}

int uname(struct utsname *value)
{
    return (int) syscall(SYS_uname, value);
}

void _exit(int status)
{
    (void) syscall(SYS_exit, status);
    for (;;) {
        __asm__ volatile("pause");
    }
}
