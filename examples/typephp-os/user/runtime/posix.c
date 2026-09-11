#include <errno.h>
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

/* glibc hides this GNU extension when php-nano requests POSIX.1-2008 only. */
long syscall(long number, ...);
extern char **environ;

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
    return (int) syscall(TYPEPHP_SYS_GETTIMEOFDAY, value, timezone);
}

int clock_gettime(clockid_t clock_id, struct timespec *value)
{
    return (int) syscall(TYPEPHP_SYS_CLOCK_GETTIME, clock_id, value);
}

int clock_getres(clockid_t clock_id, struct timespec *value)
{
    return (int) syscall(TYPEPHP_SYS_CLOCK_GETRES, clock_id, value);
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
    return (int) syscall(TYPEPHP_SYS_UNAME, value);
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
    return (int) syscall(TYPEPHP_SYS_ACCESS, path, mode);
}

int stat(const char *path, struct stat *value)
{
    return (int) syscall(TYPEPHP_SYS_STAT, path, value);
}

int lstat(const char *path, struct stat *value)
{
    return (int) syscall(TYPEPHP_SYS_LSTAT, path, value);
}

int fstat(int fd, struct stat *value)
{
    return (int) syscall(TYPEPHP_SYS_FSTAT, fd, value);
}

int fstatat(int directory_fd, const char *path, struct stat *value, int flags)
{
    return (int) syscall(
        TYPEPHP_SYS_NEWFSTATAT, directory_fd, path, value, flags);
}

int faccessat(int directory_fd, const char *path, int mode, int flags)
{
    return (int) syscall(
        TYPEPHP_SYS_FACCESSAT, directory_fd, path, mode, flags);
}

int fsync(int fd)
{
    return (int) syscall(TYPEPHP_SYS_FSYNC, fd);
}

int fdatasync(int fd)
{
    return (int) syscall(TYPEPHP_SYS_FDATASYNC, fd);
}

int truncate(const char *path, off_t length)
{
    return (int) syscall(TYPEPHP_SYS_TRUNCATE, path, length);
}

int ftruncate(int fd, off_t length)
{
    return (int) syscall(TYPEPHP_SYS_FTRUNCATE, fd, length);
}

pid_t getpid(void)
{
    return (pid_t) syscall(TYPEPHP_SYS_GETPID);
}

pid_t getppid(void)
{
    return (pid_t) syscall(TYPEPHP_SYS_GETPPID);
}

pid_t gettid(void)
{
    return (pid_t) syscall(TYPEPHP_SYS_GETTID);
}

uid_t getuid(void)
{
    return (uid_t) syscall(TYPEPHP_SYS_GETUID);
}

uid_t geteuid(void)
{
    return (uid_t) syscall(TYPEPHP_SYS_GETEUID);
}

gid_t getgid(void)
{
    return (gid_t) syscall(TYPEPHP_SYS_GETGID);
}

gid_t getegid(void)
{
    return (gid_t) syscall(TYPEPHP_SYS_GETEGID);
}
