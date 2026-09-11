#include "syscall.h"

static int string_equal(const char *left, const char *right)
{
    while (*left != '\0' && *left == *right) {
        ++left;
        ++right;
    }
    return *left == *right;
}

static size_t read_line(char *buffer, size_t capacity)
{
    size_t length = 0;
    while (capacity > 1) {
        char character = 0;
        if (typephp_syscall(TYPEPHP_SYS_READ, 0, (long) &character, 1) != 1) {
            continue;
        }
        if (character == '\r' || character == '\n') {
            typephp_write("\n");
            break;
        }
        if (character == 8 || character == 127) {
            if (length != 0) {
                --length;
                typephp_write("\b \b");
            }
            continue;
        }
        if (character >= 32 && character < 127) {
            buffer[length++] = character;
            --capacity;
            typephp_write_bytes(&character, 1);
        }
    }
    buffer[length] = '\0';
    return length;
}

static void show_prompt(void)
{
    char cwd[16];
    typephp_write("typephp-os:");
    if (typephp_syscall(TYPEPHP_SYS_GETCWD, (long) cwd, sizeof(cwd), 0) > 0) {
        typephp_write(cwd);
    } else {
        typephp_write("?");
    }
    typephp_write("$ ");
}

void _start(void)
{
    char line[80];
    unsigned short code_selector;
    __asm__ volatile("mov %%cs, %0" : "=r"(code_selector));
    if ((code_selector & 3u) != 3u) {
        typephp_write("sh: Ring-3 transition failed\n");
        typephp_exit(1);
    }
    typephp_write("TypePHP-OS user shell\n");
    typephp_write("Ring 3 confirmed\n");
    typephp_write("Commands: ls, cd <directory>, pwd, date, fault\n");
    for (;;) {
        char *argument;
        show_prompt();
        if (read_line(line, sizeof(line)) == 0) {
            continue;
        }
        argument = line;
        while (*argument != '\0' && *argument != ' ') {
            ++argument;
        }
        if (*argument != '\0') {
            *argument++ = '\0';
            while (*argument == ' ') {
                ++argument;
            }
        }
        if (string_equal(line, "ls") || string_equal(line, "cd")
            || string_equal(line, "pwd") || string_equal(line, "date")
            || string_equal(line, "fault")) {
            if (typephp_syscall(TYPEPHP_SYS_EXEC,
                    (long) line, (long) argument, 0) < 0) {
                typephp_write("sh: unable to execute command\n");
            }
        } else {
            typephp_write("sh: command not found: ");
            typephp_write(line);
            typephp_write("\n");
        }
    }
}
