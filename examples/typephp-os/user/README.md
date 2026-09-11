# TypePHP-OS userspace direction

TypePHP-OS userspace must evolve toward programs that GCC and glibc can build
and run directly. Every userspace ABI and API change must make that migration
easier, or clearly isolate temporary OS-specific behavior.

The following rules are normative for this directory:

1. C programs use the standard `int main(int argc, char **argv)` entry point.
   The shared `crt0.S` owns `_start` and receives a Linux-style initial stack:
   `argc`, `argv`, `envp`, and `auxv`.
2. Prefer ISO C and POSIX APIs and data structures. Small temporary wrappers
   should match their eventual libc signatures wherever the kernel already has
   the required semantics.
3. A Linux x86-64 syscall number may be used only when the operation, argument
   layout, return value, and observable semantics are compatible. Temporary
   TypePHP-OS services use the private syscall range instead.
4. Keep OS-specific calls behind the userspace ABI layer. Application and
   command code should gradually stop including raw syscall helpers as libc
   coverage grows.
5. TypePHP-OS is permanently a single-task system. It does not implement
   `fork`, `clone`, `execve`, `wait`, pipes, job control, threads, or signals
   for scheduling. The private shell launch service synchronously replaces
   the active command address space and restores the resident shell on exit;
   it is not a public POSIX process-creation API.
6. New kernel services should first consider the remaining interfaces needed
   by a static libc and PHP Nano: memory mapping, files and directories,
   clocks, TLS, terminal I/O, and single-task lifecycle. Socket support also
   remains out of scope.
7. Freestanding test programs remain libc-free only as a bootstrap constraint;
   it is not the final userspace programming model.

The current bootstrap libc provides `syscall`, `read`, `write`, `openat`,
`open`, `close`, `lseek`, `getcwd`, `chdir`, `mkdir`, `rmdir`, `unlink`,
`rename`, `stat`, `lstat`, `fstat`, `access`, `fsync`, `fdatasync`, `truncate`,
`ftruncate`, `time`, `gettimeofday`, `clock_gettime`, `clock_getres`, `uname`,
`getpid`, `getppid`, `gettid`, the root UID/GID queries, `brk`, `sbrk`, `mmap`,
`mprotect`, `munmap`, `strlen`, `strerror`, `perror`, and `_exit` with
libc-compatible C signatures. It also
translates kernel `-errno` results into `-1` plus the single-task userspace
`errno`. This list is a migration layer, not a reason to create
project-specific variants of standard functions.

`nano/` contains the first tpc-generated userspace project. Unlike the small C
commands, it composes the complete PHP Nano and PHPX source manifests into an
independent ELF64 executable. Shared startup, host, POSIX, libc/C++ ABI, and
math support live under `runtime/` and are built once as `libtypephp-os.a`.
Each TypePHP userspace project links that archive with `-ltypephp-os` instead
of recompiling the platform layer. The shared crt0 accepts the same Linux-style
initial stack, so the TypePHP entry receives the real command-line `argc` and
`argv`. The standard extension's `php_uname()` remains implemented in the
upstream `info.c`; the local `uname()` ABI reports `TypePHP-OS`. APIs whose
required system call is not available terminate through an explicit panic
stub. This keeps all Nano symbols linkable without pretending the incomplete
OS ABI is implemented.

The resident shell and each launched command have independent x86-64 address
spaces backed by recyclable 4 KiB physical pages. Current ELF files use
page-separated `RX` text/rodata and `RW` data/bss segments; the 64 KiB user
stack has an unmapped guard below it. The memory syscalls reserve their Linux
x86-64 numbers and semantics, with one deliberate initial subset: `mmap()`
accepts only `MAP_PRIVATE | MAP_ANONYMOUS`, `fd == -1`, and offset zero. File
mappings, shared mappings, fixed mappings, remapping, and demand paging are not
implemented yet. `brk()` and `mmap()` eagerly allocate zero-filled pages;
`munmap()` and process teardown return them to the physical-page pool.

The kernel additionally accepts Linux x86-64 `newfstatat`, `faccessat`, and
`exit_group`. File metadata uses the Linux x86-64 144-byte `struct stat`
layout. FAT16 currently has no owners, ACLs, symlinks, executable file bit, or
sub-second timestamps: UID/GID are always root, ordinary files are `0666`,
directories are `0777`, and clock resolution is one second.

`rename()` currently accepts only source and destination paths with the same
parent directory and does not replace an existing entry. Its transport uses a
private TypePHP-OS syscall number until the kernel implements the complete
Linux rename contract.

The shell and commands are independent ELF64 files installed under `/BIN` in
the FAT16 image. Command names must fit the DOS 8.3 base-name limit. Lookup maps `name` to the path
`/BIN/name.ELF`. The kernel streams and validates each ELF from the filesystem
and confines the resident shell and transient command to separate page tables,
even when they use overlapping virtual addresses. Adding a compatible command
file does not require relinking the kernel.

The current `int 0x80` entry is transitional. Before linking an ordinary
x86-64 glibc build, the kernel must also accept the `syscall` instruction with
the Linux register convention (`rax`, `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9`),
negative errno returns, and the expected `rcx`/`r11` clobbers.

Using unmodified upstream glibc will require substantially more than matching
syscall numbers. The kernel must eventually provide the expected ELF process
startup contract and enough Linux-compatible syscall behavior, or TypePHP-OS
must carry a small glibc `sysdeps` port. Until then, private behavior must not
masquerade as a Linux syscall.
