#include <sys/syscall.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char entries[512];
    const char *path = argc > 1 ? argv[1] : 0;
    long length = syscall(SYS_typephp_listdir, path, entries, sizeof(entries));
    if (length < 0) {
        (void) write(STDERR_FILENO, "ls: cannot read directory\n",
            sizeof("ls: cannot read directory\n") - 1);
        return 1;
    }
    (void) write(STDOUT_FILENO, entries, (size_t) length);
    return 0;
}
