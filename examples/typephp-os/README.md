# TypePHP freestanding OS experiment

This is a long-running experimental x86_64 TypePHP operating system. It boots
under QEMU and runs an ordinary TypePHP Nano program without linking hosted
PHP, libc, or libstdc++. TypePHP implements the startup self-check, a custom
Zend class, the prime-number demo, and the resident application loop. Small C
and assembly layers provide the machine bootstrap and current kernel services.

The architectural rules and staged plan are maintained in
[ROADMAP.md](ROADMAP.md).

## Architecture

There is no kernel-specific tpc mode or reduced PHP Nano profile. The 64-bit
payload is compiled with the normal command:

```shell
./bin/tpc.php --nano examples/typephp-os/project.yml
```

This composes the complete php-nano and PHPX source manifests. The TypePHP OS
project owns the freestanding boundary under `freestanding/abi`: implemented
C/POSIX and C++ ABI functions live there, while APIs required for linking but
not implemented by the kernel are exported as panic stubs. Consequently an
unsupported operation fails immediately with its ABI symbol instead of
silently returning fabricated data. Filesystem APIs are the next subsystem to
replace with real implementations; sockets are outside the current scope.

Cross-project portability uses general feature switches only:

- `PHP_NANO_NO_LIBC` asks the embedding host to provide Nano's clock, sleep,
  and entropy hooks;
- `PHPX_NO_EXCEPTION` routes PHPX exception propagation to the host abort hook;
- `PHPX_NO_RTTI` selects PHPX's non-RTTI checked cast policy;
- `TYPEPHP_NO_MAIN` lets an embedding host provide the process/kernel entry.

None of these switches refers to TypePHP OS or to a kernel build. Ordinary
Nano and PHPX builds preserve their hosted defaults.

The payload uses PHP's original Zend allocator, GC, strings, HashTables,
objects, classes, exceptions, and built-in extension registration. Generated
projects and built-ins are registered through their ordinary MINIT paths; the
only excluded capability is dynamic PHP execution through ZendVM.

## Build and run

Required host tools are TypePHP's PHP/Composer dependencies, GCC/G++, GNU
binutils, GNU make, and `qemu-system-x86_64`.

From this directory, build and boot with:

```shell
make
make run
```

The build produces two useful files:

- `build/kernel64.elf`: the 64-bit TypePHP + ordinary Nano payload produced by
  tpc;
- `build/typephp-os.elf`: the final Multiboot kernel accepted by QEMU.

Run the automated serial-output smoke test with:

```shell
make test
```

The Makefile compiles only the 32-bit Multiboot bootstrap externally because
its `-m32` ABI cannot participate in the 64-bit payload link. All ordinary
64-bit `.c`, `.cc`, and `.S` files remain in `project.yml` and use tpc's generic
`c-flags`, `cxx-flags`, and `asm-flags`. Generic same-ABI prebuilt objects can
be supplied with `objects`; the architecture-changing bootstrap is instead
combined during the final packaging link.

After tpc emits `kernel64.elf`, the Makefile uses `objcopy` to turn the payload
into a raw binary and then an ELF32 data object. GNU ld combines that object
with the 32-bit bootstrap. The bootstrap is loaded through Multiboot v1,
identity-maps the first GiB, enters x86_64 long mode, and transfers control to
the payload linked at 2 MiB.

To invoke QEMU manually:

```shell
qemu-system-x86_64 -m 128M \
  -kernel examples/typephp-os/build/typephp-os.elf \
  -display none -serial stdio -monitor none -no-reboot -no-shutdown
```

Press `Ctrl+A`, then `X`, to leave headless QEMU.

## Current capabilities

The bootstrap passes the Multiboot memory map to the 64-bit kernel. The
physical layer reserves the complete kernel image, selects mapped usable RAM,
and installs the remaining arena behind the project-owned `posix_memalign()`.
Upstream `zend_alloc` then obtains and subdivides aligned 2 MiB chunks. C++
global `new` and `delete`, including `std::vector` allocations, use Zend MM.

The QEMU smoke test currently verifies:

- 64-bit TypePHP scalar and control-flow execution;
- real Zend allocation, GC, string, array, object, class, and exception data;
- PHPX `Variant`, `Str`, `Array`, custom classes, and a typed
  `std::vector<int>`;
- built-in date handling and `sleep()` through Zend Bridge;
- a TypePHP prime calculation for 0–100;
- a resident loop that prints the RTC-derived UTC time and
  `Hello TypePHP-OS!` every two seconds.

The 64-bit payload currently occupies about 7 MiB, so the first 16 MiB is
reserved before physical memory is handed to Zend MM. Returning entire Zend
chunks to a future physical page allocator remains later work.

Filesystem ABI entries already link as explicit panic stubs. The next
milestone will replace them with an in-kernel filesystem and make PHP's local
file stream operations real. Network sockets, dynamic module loading,
`include`/`require`/`eval`, and external process execution remain unavailable.
