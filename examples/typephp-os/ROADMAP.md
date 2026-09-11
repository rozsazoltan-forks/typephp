# TypePHP OS roadmap

This directory is a long-running experiment. Every milestone must remain
bootable in QEMU and must not change ordinary TypePHP, PHPX, or PHP Nano
behavior.

## Architecture rules

- The project is an ordinary `tpc --nano` consumer. tpc has no kernel option,
  and PHP Nano has no kernel source profile.
- PHP and Composer remain build-time tools; the kernel never links `libphp`.
- Keep copied php-src C/H files unchanged and compile the complete Nano source
  manifest. TypePHP OS owns the required libc/POSIX and C++ ABI boundary.
- Missing ABI operations must be linkable panic stubs. Replace each stub with
  a real kernel service before exposing the corresponding PHP capability.
- Cross-project changes must use generally useful portability contracts such
  as `PHP_NANO_NO_LIBC`, `PHPX_NO_EXCEPTION`, and `PHPX_NO_RTTI`; product-specific
  conditionals are not allowed in PHP Nano or PHPX.
- Ordinary same-ABI source files and flags belong in `project.yml`. Translation
  units that require incompatible per-file options are built by Make and may
  be linked through tpc's generic `objects` facility when their ABI matches.
  Architecture packaging and ELF/binary conversion happen after tpc emits the
  64-bit ELF.
- Every completed milestone must pass the serial-output QEMU smoke test and
  leave the 64-bit payload with no undefined symbols.

## Milestones

1. **Boot and scalar AOT — complete.** Multiboot v1 bootstrap, x86_64 long
   mode, VGA/COM1 output, TypePHP scalars, functions, and control flow.
2. **Memory and Zend containers — complete.** Physical arena, TypePHP OS ABI
   layer, original `zend_alloc`, `zend_gc`, `zend_string`, `zend_hash`, zval
   destruction, and generated TypePHP arrays/strings through PHPX.
3. **Zend object model — complete for the current demo.** Original object
   store, class entries, inheritance, interfaces, handlers, PHP exception
   classes, generated user classes, construction, properties, and destruction.
4. **Time service — complete.** CMOS wall clock, monotonic host hook, built-in
   date formatting, and the standard extension's `sleep()` path.
5. **Filesystem — first slice complete.** ATA PIO, a persistent FAT16 image,
   TypePHP-owned cluster/directory/file logic, file descriptors, POSIX
   directory/stat operations, and PHP's local file stream API. The current
   implementation is intentionally limited to root-level DOS 8.3 names;
   nested directories and long filenames are next.
6. **Single-task userspace — first slice complete.** ELF64 validation/loading,
   supervisor/user page separation, GDT/TSS, Ring-3 entry, synchronous
   `int 0x80` system calls, COM1 standard I/O, saved parent context, and
   independent freestanding C `sh`, `ls`, and `cd` programs.
7. **Native Class memory.** Exercise Wren GC through Zend MM and verify tracing
   of PHPX fields under sustained allocation.
8. **Kernel services.** Interrupt-driven timer, keyboard, physical-page
   reclamation, and a capability-oriented native API.
9. **Packaging and CI.** Automate the two-stage ELF32/ELF64 build and QEMU boot
   smoke test in GitHub Actions.

Later architectures may provide different bootstraps and host ABI adapters.
Generated TypePHP code and PHPX values remain 64-bit on every target.
