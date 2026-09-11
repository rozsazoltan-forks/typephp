#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

_Static_assert(sizeof(uintptr_t) == 8, "TypePHP-OS requires a 64-bit mlibc ABI");

static void typecheck(void)
{
    struct dirent entry;
    struct stat status;
    struct timespec timestamp;
    struct utsname identity;

    (void) entry;
    (void) status;
    (void) timestamp;
    (void) identity;
}
