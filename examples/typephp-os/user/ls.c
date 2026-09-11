#include "syscall.h"

void _start(const char *argument)
{
    char entries[512];
    const char *path = argument != 0 && argument[0] != '\0' ? argument : 0;
    long length = typephp_syscall(
        TYPEPHP_SYS_READDIR, (long) path, (long) entries, sizeof(entries));
    if (length < 0) {
        typephp_write("ls: cannot read directory\n");
        typephp_exit(1);
    }
    typephp_write_bytes(entries, (size_t) length);
    typephp_exit(0);
}
