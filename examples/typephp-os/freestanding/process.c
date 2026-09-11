/*
   +----------------------------------------------------------------------+
   | TypePHP OS                                                          |
   +----------------------------------------------------------------------+
   | Single foreground process, ELF64 loader and int 0x80 syscall core.  |
   | SPDX-License-Identifier: BSD-3-Clause                               |
   +----------------------------------------------------------------------+
*/

#include "typephp_os_abi.h"
#include "typephp_os_syscall.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    USER_BEGIN = 32u * 1024u * 1024u,
    USER_END = 36u * 1024u * 1024u,
    USER_STACK = USER_END - 16u,
    USER_CODE_SELECTOR = 0x23,
    USER_DATA_SELECTOR = 0x1b,
    KERNEL_CODE_SELECTOR = 0x08,
    TSS_SELECTOR = 0x28,
    IDT_SYSCALL = 0x80,
};

enum {
    ELF_PT_LOAD = 1,
    ELF_ET_EXEC = 2,
    ELF_MACHINE_X86_64 = 62,
};

typedef struct __attribute__((packed)) {
    unsigned char ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf64_header;

typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} elf64_program_header;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} descriptor_pointer;

typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} idt_gate;

typedef struct __attribute__((packed)) {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} task_state_segment;

typedef struct {
    uint64_t rax;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} syscall_frame;

typedef struct {
    uint64_t rax;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t vector;
    uint64_t error;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} exception_frame;

typedef struct {
    uint64_t pid;
    int running;
    char cwd[16];
    syscall_frame parent_frame;
    int parent_waiting;
} process_state;

extern const unsigned char typephp_user_sh_elf_start[];
extern const unsigned char typephp_user_sh_elf_end[];
extern const unsigned char typephp_user_ls_elf_start[];
extern const unsigned char typephp_user_ls_elf_end[];
extern const unsigned char typephp_user_cd_elf_start[];
extern const unsigned char typephp_user_cd_elf_end[];
extern const unsigned char typephp_user_date_elf_start[];
extern const unsigned char typephp_user_date_elf_end[];
extern const unsigned char typephp_user_pwd_elf_start[];
extern const unsigned char typephp_user_pwd_elf_end[];
extern const unsigned char typephp_user_fault_elf_start[];
extern const unsigned char typephp_user_fault_elf_end[];
extern void typephp_os_syscall_entry(void);
extern void typephp_os_enter_user(uint64_t entry, uint64_t stack);
extern long typephp_os_time_seconds(void);
extern void typephp_os_exception_0(void);
extern void typephp_os_exception_3(void);
extern void typephp_os_exception_5(void);
extern void typephp_os_exception_6(void);
extern void typephp_os_exception_10(void);
extern void typephp_os_exception_11(void);
extern void typephp_os_exception_12(void);
extern void typephp_os_exception_13(void);
extern void typephp_os_exception_14(void);

static uint64_t gdt[7] __attribute__((aligned(16)));
static idt_gate idt[256] __attribute__((aligned(16)));
static task_state_segment tss;
static unsigned char syscall_stack[64u * 1024u] __attribute__((aligned(16)));
static process_state foreground_process = {.pid = 1, .cwd = "/"};

static void panic(const char *message)
{
    typephp_os_write("Process panic: ", sizeof("Process panic: ") - 1);
    typephp_os_panic(message);
}

static int range_inside(uint64_t address, uint64_t size, uint64_t begin, uint64_t end)
{
    return address >= begin && size <= end - begin && address <= end - size;
}

static int user_buffer(const void *pointer, size_t size)
{
    return range_inside((uint64_t) (uintptr_t) pointer, size, USER_BEGIN, USER_END);
}

static size_t bounded_user_string(const char *string, size_t maximum)
{
    size_t length = 0;
    if (!user_buffer(string, 1)) {
        return (size_t) -1;
    }
    while (length < maximum && user_buffer(string + length, 1) && string[length] != '\0') {
        ++length;
    }
    return length < maximum && user_buffer(string + length, 1) ? length : (size_t) -1;
}

static void install_tss_descriptor(uint64_t base, uint32_t limit)
{
    gdt[5] = ((uint64_t) (limit & 0xffffu))
        | ((base & 0xffffffu) << 16u)
        | (UINT64_C(0x89) << 40u)
        | ((uint64_t) ((limit >> 16u) & 0x0fu) << 48u)
        | (((base >> 24u) & 0xffu) << 56u);
    gdt[6] = base >> 32u;
}

static void install_idt_gate(unsigned int vector, void (*entry)(void), uint8_t attributes)
{
    uintptr_t handler = (uintptr_t) entry;
    idt[vector].offset_low = handler & 0xffffu;
    idt[vector].selector = KERNEL_CODE_SELECTOR;
    idt[vector].attributes = attributes;
    idt[vector].offset_middle = (handler >> 16u) & 0xffffu;
    idt[vector].offset_high = handler >> 32u;
}

static void install_descriptor_tables(void)
{
    descriptor_pointer pointer;

    memset(gdt, 0, sizeof(gdt));
    gdt[1] = UINT64_C(0x00af9a000000ffff);
    gdt[2] = UINT64_C(0x00cf92000000ffff);
    gdt[3] = UINT64_C(0x00cff2000000ffff);
    gdt[4] = UINT64_C(0x00affa000000ffff);
    memset(&tss, 0, sizeof(tss));
    tss.rsp0 = (uint64_t) (uintptr_t) (syscall_stack + sizeof(syscall_stack));
    tss.iomap_base = sizeof(tss);
    install_tss_descriptor((uint64_t) (uintptr_t) &tss, sizeof(tss) - 1u);

    pointer.limit = sizeof(gdt) - 1u;
    pointer.base = (uint64_t) (uintptr_t) gdt;
    __asm__ volatile("lgdt %0" : : "m"(pointer) : "memory");
    __asm__ volatile(
        "mov %[data], %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "ltr %[tss]\n"
        :
        : [data] "i"(0x10), [tss] "r"((uint16_t) TSS_SELECTOR)
        : "rax", "memory");

    memset(idt, 0, sizeof(idt));
    install_idt_gate(0, typephp_os_exception_0, 0x8e);
    install_idt_gate(3, typephp_os_exception_3, 0xee);
    install_idt_gate(5, typephp_os_exception_5, 0x8e);
    install_idt_gate(6, typephp_os_exception_6, 0x8e);
    install_idt_gate(10, typephp_os_exception_10, 0x8e);
    install_idt_gate(11, typephp_os_exception_11, 0x8e);
    install_idt_gate(12, typephp_os_exception_12, 0x8e);
    install_idt_gate(13, typephp_os_exception_13, 0x8e);
    install_idt_gate(14, typephp_os_exception_14, 0x8e);
    install_idt_gate(IDT_SYSCALL, typephp_os_syscall_entry, 0xee);
    pointer.limit = sizeof(idt) - 1u;
    pointer.base = (uint64_t) (uintptr_t) idt;
    __asm__ volatile("lidt %0" : : "m"(pointer) : "memory");
}

static uint64_t load_user_elf(const unsigned char *image, const unsigned char *image_end)
{
    const size_t image_size = (size_t) (image_end - image);
    const elf64_header *header;

    if (image_size < sizeof(elf64_header)) {
        panic("truncated user ELF\n");
    }
    header = (const elf64_header *) image;
    if (header->ident[0] != 0x7f || header->ident[1] != 'E'
        || header->ident[2] != 'L' || header->ident[3] != 'F'
        || header->ident[4] != 2 || header->ident[5] != 1
        || header->type != ELF_ET_EXEC || header->machine != ELF_MACHINE_X86_64
        || header->phentsize != sizeof(elf64_program_header)) {
        panic("invalid user ELF64 header\n");
    }
    if (!range_inside(header->phoff,
            (uint64_t) header->phnum * sizeof(elf64_program_header), 0, image_size)) {
        panic("invalid user ELF program table\n");
    }

    for (uint16_t index = 0; index < header->phnum; ++index) {
        const elf64_program_header *segment = (const elf64_program_header *)
            (image + header->phoff + (uint64_t) index * sizeof(*segment));
        if (segment->type != ELF_PT_LOAD) {
            continue;
        }
        if (segment->filesz > segment->memsz
            || !range_inside(segment->offset, segment->filesz, 0, image_size)
            || !range_inside(segment->vaddr, segment->memsz, USER_BEGIN, USER_END)) {
            panic("invalid user ELF load segment\n");
        }
        memcpy((void *) (uintptr_t) segment->vaddr, image + segment->offset, segment->filesz);
        memset((void *) (uintptr_t) (segment->vaddr + segment->filesz),
            0, segment->memsz - segment->filesz);
    }
    if (!range_inside(header->entry, 1, USER_BEGIN, USER_END)) {
        panic("invalid user ELF entry\n");
    }
    return header->entry;
}

static int command_image(
    const char *name,
    const unsigned char **image,
    const unsigned char **image_end)
{
    if (strcmp(name, "ls") == 0) {
        *image = typephp_user_ls_elf_start;
        *image_end = typephp_user_ls_elf_end;
        return 1;
    }
    if (strcmp(name, "cd") == 0) {
        *image = typephp_user_cd_elf_start;
        *image_end = typephp_user_cd_elf_end;
        return 1;
    }
    if (strcmp(name, "date") == 0) {
        *image = typephp_user_date_elf_start;
        *image_end = typephp_user_date_elf_end;
        return 1;
    }
    if (strcmp(name, "pwd") == 0) {
        *image = typephp_user_pwd_elf_start;
        *image_end = typephp_user_pwd_elf_end;
        return 1;
    }
    if (strcmp(name, "fault") == 0) {
        *image = typephp_user_fault_elf_start;
        *image_end = typephp_user_fault_elf_end;
        return 1;
    }
    return 0;
}

static long syscall_getcwd(char *buffer, size_t size)
{
    size_t length = strlen(foreground_process.cwd) + 1u;
    if (size < length || !user_buffer(buffer, size)) {
        return -1;
    }
    memcpy(buffer, foreground_process.cwd, length);
    return (long) length;
}

static long syscall_chdir(const char *path)
{
    char normalized[16];
    size_t length = bounded_user_string(path, sizeof(normalized));
    if (length == (size_t) -1 || length == 0) {
        return -1;
    }
    if ((length == 1 && path[0] == '/')
        || (length == 2 && path[0] == '.' && path[1] == '.')) {
        normalized[0] = '/';
        normalized[1] = '\0';
    } else if (length == 1 && path[0] == '.') {
        return 0;
    } else {
        size_t source = path[0] == '/' ? 1u : 0u;
        size_t name_length = length - source;
        if (name_length == 0 || name_length > 12u) {
            return -1;
        }
        normalized[0] = '/';
        memcpy(normalized + 1, path + source, name_length);
        normalized[name_length + 1u] = '\0';
    }
    if (typephp_os_fs_path_type(normalized) != 2) {
        return -1;
    }
    memcpy(foreground_process.cwd, normalized, strlen(normalized) + 1u);
    return 0;
}

static long syscall_readdir(const char *path, char *buffer, size_t capacity)
{
    char resolved[16];
    const char *directory = foreground_process.cwd;
    if (path != 0) {
        size_t length = bounded_user_string(path, sizeof(resolved));
        if (length == (size_t) -1) {
            return -1;
        }
        if (length != 0) {
            memcpy(resolved, path, length + 1u);
            directory = resolved;
        }
    }
    if (!user_buffer(buffer, capacity)) {
        return -1;
    }
    return typephp_os_fs_list(directory, buffer, capacity);
}

static long syscall_exec(syscall_frame *frame, const char *name, const char *argument)
{
    const unsigned char *image;
    const unsigned char *image_end;
    char command[8];
    size_t name_length;

    if (foreground_process.parent_waiting) {
        return -1;
    }
    name_length = bounded_user_string(name, sizeof(command));
    if (name_length == (size_t) -1 || name_length == 0) {
        return -1;
    }
    memcpy(command, name, name_length + 1u);
    if (!command_image(command, &image, &image_end)) {
        return -1;
    }
    if (argument != 0 && bounded_user_string(argument, 64) == (size_t) -1) {
        return -1;
    }

    memcpy(&foreground_process.parent_frame, frame, sizeof(*frame));
    foreground_process.parent_waiting = 1;
    foreground_process.pid = 2;
    frame->rip = load_user_elf(image, image_end);
    frame->rsp = 35u * 1024u * 1024u - 16u;
    frame->rdi = (uint64_t) (uintptr_t) argument;
    frame->rsi = 0;
    frame->rdx = 0;
    return 0;
}

static long syscall_exit(syscall_frame *frame, long status)
{
    if (!foreground_process.parent_waiting) {
        foreground_process.running = 0;
        typephp_os_panic("shell process exited\n");
    }
    memcpy(frame, &foreground_process.parent_frame, sizeof(*frame));
    foreground_process.parent_waiting = 0;
    foreground_process.pid = 1;
    return status;
}

static void write_unsigned(uint64_t value)
{
    char digits[20];
    size_t count = 0;
    do {
        digits[count++] = (char) ('0' + value % 10u);
        value /= 10u;
    } while (value != 0);
    while (count != 0) {
        --count;
        typephp_os_write(&digits[count], 1);
    }
}

static void write_hex(uint64_t value)
{
    static const char digits[] = "0123456789abcdef";
    char output[18];
    int index;
    output[0] = '0';
    output[1] = 'x';
    for (index = 0; index < 16; ++index) {
        output[index + 2] = digits[(value >> ((15 - index) * 4)) & 0x0fu];
    }
    typephp_os_write(output, sizeof(output));
}

static const char *exception_name(uint64_t vector)
{
    switch (vector) {
    case 0: return "divide error";
    case 3: return "breakpoint";
    case 5: return "bounds";
    case 6: return "invalid opcode";
    case 10: return "invalid TSS";
    case 11: return "segment not present";
    case 12: return "stack fault";
    case 13: return "general protection";
    case 14: return "page fault";
    default: return "unknown";
    }
}

static void restore_shell_after_fault(exception_frame *frame)
{
    syscall_frame *parent = &foreground_process.parent_frame;
    /* The general-register prefixes of both frame formats are identical. */
    memcpy(frame, parent, offsetof(syscall_frame, rip));
    frame->rax = (uint64_t) -1;
    frame->rip = parent->rip;
    frame->cs = parent->cs;
    frame->rflags = parent->rflags;
    frame->rsp = parent->rsp;
    frame->ss = parent->ss;
    foreground_process.parent_waiting = 0;
    foreground_process.pid = 1;
}

void typephp_os_exception_dispatch(exception_frame *frame)
{
    uint64_t fault_address = 0;
    if (frame->vector == 14) {
        __asm__ volatile("mov %%cr2, %0" : "=r"(fault_address));
    }

    typephp_os_write("User process ", sizeof("User process ") - 1);
    write_unsigned(foreground_process.pid);
    typephp_os_write(" fault: ", sizeof(" fault: ") - 1);
    typephp_os_write(exception_name(frame->vector), strlen(exception_name(frame->vector)));
    typephp_os_write(" (#", sizeof(" (#") - 1);
    write_unsigned(frame->vector);
    typephp_os_write(") at ", sizeof(") at ") - 1);
    write_hex(frame->rip);
    if (frame->vector == 14) {
        typephp_os_write(", address ", sizeof(", address ") - 1);
        write_hex(fault_address);
    }
    typephp_os_write("\n", 1);

    if ((frame->cs & 3u) != 3u) {
        panic("kernel-mode exception\n");
    }
    if (!foreground_process.parent_waiting) {
        panic("resident shell faulted\n");
    }
    restore_shell_after_fault(frame);
}

long typephp_os_syscall_dispatch(syscall_frame *frame)
{
    switch (frame->rax) {
    case TYPEPHP_SYS_READ:
        if (frame->rdi != 0 || !user_buffer((void *) frame->rsi, frame->rdx)) {
            return -1;
        }
        return typephp_os_console_read((void *) frame->rsi, frame->rdx);
    case TYPEPHP_SYS_WRITE:
        if ((frame->rdi != 1 && frame->rdi != 2)
            || !user_buffer((void *) frame->rsi, frame->rdx)) {
            return -1;
        }
        typephp_os_write((const char *) frame->rsi, frame->rdx);
        return (long) frame->rdx;
    case TYPEPHP_SYS_EXEC:
        return syscall_exec(frame, (const char *) frame->rdi, (const char *) frame->rsi);
    case TYPEPHP_SYS_GETCWD:
        return syscall_getcwd((char *) frame->rdi, frame->rsi);
    case TYPEPHP_SYS_CHDIR:
        return syscall_chdir((const char *) frame->rdi);
    case TYPEPHP_SYS_TIME:
        return typephp_os_time_seconds();
    case TYPEPHP_SYS_READDIR:
        return syscall_readdir((const char *) frame->rdi,
            (char *) frame->rsi, frame->rdx);
    case TYPEPHP_SYS_EXIT:
        return syscall_exit(frame, (long) frame->rdi);
    default:
        return -1;
    }
}

void typephp_os_process_start(void)
{
    uint64_t entry;
    memset((void *) (uintptr_t) USER_BEGIN, 0, USER_END - USER_BEGIN);
    entry = load_user_elf(typephp_user_sh_elf_start, typephp_user_sh_elf_end);
    install_descriptor_tables();
    foreground_process.running = 1;
    typephp_os_write("Process 1: sh.elf (Ring 3)\n",
        sizeof("Process 1: sh.elf (Ring 3)\n") - 1);
    typephp_os_enter_user(entry, USER_STACK);
    panic("Ring-3 entry returned\n");
}
