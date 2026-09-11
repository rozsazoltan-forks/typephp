#include "syscall.h"

void _start(const char *argument)
{
    char cwd[16];
    (void) argument;
    if (typephp_syscall(TYPEPHP_SYS_GETCWD, (long) cwd, sizeof(cwd), 0) < 0) {
        typephp_write("pwd: working directory unavailable\n");
        typephp_exit(1);
    }
    typephp_write(cwd);
    typephp_write("\n");
    typephp_exit(0);
}
