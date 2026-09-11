#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

_Static_assert(sizeof(struct stat) == 144, "Linux x86-64 stat ABI changed");

static int fail(const char *operation)
{
    size_t length = 0;
    while (operation[length] != '\0') {
        ++length;
    }
    (void) write(STDERR_FILENO, "systest: ", sizeof("systest: ") - 1);
    (void) write(STDERR_FILENO, operation, length);
    (void) write(STDERR_FILENO, " failed\n", sizeof(" failed\n") - 1);
    return 1;
}

int main(int argc, char **argv)
{
    static const char path[] = "/SYS.TMP";
    static const char payload[] = "abcdef";
    struct stat info;
    struct timeval wall;
    struct timespec realtime;
    struct timespec monotonic;
    struct timespec resolution;
    int fd;
    (void) argc;
    (void) argv;

    if (stat("/HELLO.TXT", &info) != 0 || !S_ISREG(info.st_mode)
        || info.st_size <= 0 || access("/HELLO.TXT", R_OK) != 0) {
        return fail("stat/access");
    }
    if (fstatat(AT_FDCWD, "/HELLO.TXT", &info, AT_SYMLINK_NOFOLLOW) != 0
        || faccessat(AT_FDCWD, "/HELLO.TXT", R_OK, 0) != 0) {
        return fail("*at metadata");
    }

    (void) unlink(path);
    fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd < 0 || write(fd, payload, sizeof(payload) - 1) != sizeof(payload) - 1
        || fsync(fd) != 0 || ftruncate(fd, 3) != 0
        || fstat(fd, &info) != 0 || info.st_size != 3
        || fdatasync(fd) != 0 || close(fd) != 0) {
        return fail("descriptor persistence");
    }
    if (truncate(path, 1) != 0 || lstat(path, &info) != 0
        || info.st_size != 1 || unlink(path) != 0) {
        return fail("path truncate");
    }

    if (getpid() != 2 || gettid() != 2 || getppid() != 1
        || getuid() != 0 || geteuid() != 0 || getgid() != 0 || getegid() != 0) {
        return fail("single-task identity");
    }
    if (gettimeofday(&wall, NULL) != 0 || wall.tv_sec <= 0
        || clock_gettime(CLOCK_REALTIME, &realtime) != 0
        || clock_gettime(CLOCK_MONOTONIC, &monotonic) != 0
        || clock_getres(CLOCK_MONOTONIC, &resolution) != 0
        || realtime.tv_sec <= 0 || monotonic.tv_sec <= 0
        || resolution.tv_sec != 1 || resolution.tv_nsec != 0) {
        return fail("clock ABI");
    }

    (void) write(STDOUT_FILENO, "basic syscalls: OK\n",
        sizeof("basic syscalls: OK\n") - 1);
    return 0;
}
