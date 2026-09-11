#include "syscall.h"

void _start(const char *argument)
{
    const char *path = argument != 0 && argument[0] != '\0' ? argument : "/";
    if (typephp_syscall(TYPEPHP_SYS_CHDIR, (long) path, 0, 0) != 0) {
        typephp_write("cd: no such directory\n");
        typephp_exit(1);
    }
    typephp_exit(0);
}
