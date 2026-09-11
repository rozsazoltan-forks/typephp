#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#include <typephp_os_syscall.h>

static __attribute__((noreturn)) void unsupported(const char *name)
{
    static const char prefix[] = "unsupported TypePHP-OS user ABI: ";
    (void) syscall(TYPEPHP_SYS_WRITE, STDERR_FILENO, prefix, sizeof(prefix) - 1);
    (void) syscall(TYPEPHP_SYS_WRITE, STDERR_FILENO, name, strlen(name));
    (void) syscall(TYPEPHP_SYS_WRITE, STDERR_FILENO, "\n", 1);
    (void) syscall(TYPEPHP_SYS_EXIT, 126);
    for (;;) {
        __asm__ volatile("pause");
    }
}

ssize_t read(int fd, void *buffer, size_t count)
{
    return (ssize_t) syscall(TYPEPHP_SYS_READ, fd, buffer, count);
}

ssize_t write(int fd, const void *buffer, size_t count)
{
    return (ssize_t) syscall(TYPEPHP_SYS_WRITE, fd, buffer, count);
}

int close(int fd)
{
    return (int) syscall(TYPEPHP_SYS_CLOSE, fd);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return (off_t) syscall(TYPEPHP_SYS_LSEEK, fd, offset, whence);
}

int openat(int directory_fd, const char *path, int flags, ...)
{
    return (int) syscall(TYPEPHP_SYS_OPENAT, directory_fd, path, flags, 0);
}

int open(const char *path, int flags, ...)
{
    return openat(AT_FDCWD, path, flags, 0);
}

int chdir(const char *path)
{
    return (int) syscall(TYPEPHP_SYS_CHDIR, path);
}

int mkdir(const char *path, mode_t mode)
{
    return (int) syscall(TYPEPHP_SYS_MKDIR, path, mode);
}

int unlink(const char *path)
{
    return (int) syscall(TYPEPHP_SYS_UNLINK, path);
}

int rmdir(const char *path)
{
    return (int) syscall(TYPEPHP_SYS_RMDIR, path);
}

int brk(void *address)
{
    return (void *) syscall(TYPEPHP_SYS_BRK, address) == address ? 0 : -1;
}

void *sbrk(intptr_t increment)
{
    const intptr_t current = syscall(TYPEPHP_SYS_BRK, 0);
    const intptr_t requested = current + increment;
    if ((increment > 0 && requested < current)
        || (increment < 0 && requested > current)
        || (increment != 0 && syscall(TYPEPHP_SYS_BRK, requested) != requested)) {
        errno = ENOMEM;
        return (void *) -1;
    }
    return (void *) current;
}

void *mmap(void *address, size_t length, int protection, int flags, int fd, off_t offset)
{
    return (void *) syscall(TYPEPHP_SYS_MMAP,
        address, length, protection, flags, fd, offset);
}

int mprotect(void *address, size_t length, int protection)
{
    return (int) syscall(TYPEPHP_SYS_MPROTECT, address, length, protection);
}

int munmap(void *address, size_t length)
{
    return (int) syscall(TYPEPHP_SYS_MUNMAP, address, length);
}

time_t time(time_t *result)
{
    return (time_t) syscall(TYPEPHP_SYS_TIME, result);
}

int gettimeofday(struct timeval *value, void *timezone)
{
    (void) timezone;
    value->tv_sec = time(NULL);
    value->tv_usec = 0;
    return 0;
}

int clock_gettime(clockid_t clock_id, struct timespec *value)
{
    (void) clock_id;
    value->tv_sec = time(NULL);
    value->tv_nsec = 0;
    return 0;
}

unsigned int sleep(unsigned int seconds)
{
    const time_t deadline = time(NULL) + seconds;
    while (time(NULL) < deadline) {
        __asm__ volatile("pause");
    }
    return 0;
}

int nanosleep(const struct timespec *duration, struct timespec *remaining)
{
    (void) remaining;
    sleep((unsigned int) duration->tv_sec + (duration->tv_nsec != 0));
    return 0;
}

int uname(struct utsname *value)
{
    if (value == NULL) {
        errno = EFAULT;
        return -1;
    }
    memset(value, 0, sizeof(*value));
    memcpy(value->sysname, "TypePHP-OS", sizeof("TypePHP-OS"));
    memcpy(value->nodename, "typephp-os", sizeof("typephp-os"));
    memcpy(value->release, "0.1", sizeof("0.1"));
    memcpy(value->version, "TypePHP Nano user mode", sizeof("TypePHP Nano user mode"));
    memcpy(value->machine, "x86_64", sizeof("x86_64"));
    return 0;
}

int isatty(int fd)
{
    return fd >= STDIN_FILENO && fd <= STDERR_FILENO;
}

int rename(const char *old_path, const char *new_path)
{
    return (int) syscall(TYPEPHP_SYS_RENAME, old_path, new_path);
}

int access(const char *path, int mode)
{
    (void) path;
    (void) mode;
    unsupported("access");
}

int stat(const char *path, struct stat *value)
{
    (void) path;
    (void) value;
    unsupported("stat");
}

int lstat(const char *path, struct stat *value)
{
    (void) path;
    (void) value;
    unsupported("lstat");
}

int fstat(int fd, struct stat *value)
{
    (void) fd;
    (void) value;
    unsupported("fstat");
}

DIR *opendir(const char *path)
{
    (void) path;
    unsupported("opendir");
}

struct dirent *readdir(DIR *directory)
{
    (void) directory;
    unsupported("readdir");
}

int closedir(DIR *directory)
{
    (void) directory;
    unsupported("closedir");
}

void rewinddir(DIR *directory)
{
    (void) directory;
    unsupported("rewinddir");
}

int fsync(int fd)
{
    (void) fd;
    unsupported("fsync");
}

int ftruncate(int fd, off_t length)
{
    (void) fd;
    (void) length;
    unsupported("ftruncate");
}
