# Third-party source policy

The generated `thirdparty/` directory is intentionally ignored by Git. Every
third-party source dependency must instead be declared by a script under
`tools/` with all of the following information:

- an immutable upstream version or commit;
- the canonical download URL;
- a SHA-256 checksum for every downloaded archive or individual source file;
- an explicit list of files copied into the build tree, or an explicitly
  documented complete release tree for source-port candidates;
- the upstream license file.

Run the common fetch entry point before compiling:

```shell
./tools/fetch-thirdparty.sh
```

`make` invokes this command automatically. Each dependency has a dedicated
`fetch-*.sh` script, while `fetch-thirdparty.sh` is only the common entry point.
A matching local version marker makes repeated invocations offline and
effectively free. Third-party source must not be committed directly.

Run the upstream integration probes separately:

```shell
make thirdparty-smoke
```

This target requires Meson 1.3 or newer for mlibc. It configures a headers-only
mlibc sysroot and builds a deliberately small hosted Toybox. Neither candidate
is installed into the TypePHP-OS disk image by this probe.

## OpenLibm

- Upstream: <https://github.com/JuliaMath/openlibm>
- Version: `v0.8.7`
- Commit: `9fbeafcd4f1b6ef6aa3946c1c8faead50f38a94d`
- Archive SHA-256: `e328a1d59b94748b111e022bca6a9d2fc0481fb57d23c87d90f394b559d4f062`
- Selected files: [`tools/openlibm-files.txt`](tools/openlibm-files.txt)

Only the architecture/compatibility headers and C sources required by
TypePHP-OS's current x86_64 double-precision math ABI are installed. The
selection keeps unused implementations out of the freestanding payload.

## LLVM compiler-rt builtins

- Upstream: <https://github.com/llvm/llvm-project/tree/llvmorg-23.1.1/compiler-rt/lib/builtins>
- Version: `23.1.1` (`llvmorg-23.1.1`)
- Selected files and per-file SHA-256 values:
  [`tools/compiler-rt-builtins-files.sha256`](tools/compiler-rt-builtins-files.sha256)

The source selection provides the x86-64 128-bit integer shift, multiply,
divide, and remainder helpers. It is compiled into
`build/libcompiler-rt-builtins.a` and linked after every kernel, bootstrap C,
and TypePHP Nano userspace object set. The `builtins.elf` smoke command forces
signed and unsigned 128-bit division so the archive is tested as a real linker
dependency instead of merely being compiled.

## mlibc

- Upstream: <https://github.com/managarm/mlibc>
- Version: `v7.0.0`
- Commit: `7c2a178142625cc9852e59a1a090468c61a62d3b`
- Archive SHA-256: `56d62366a6a15bc3fd8741bebcb311ca543553f7b734b1e780aeae33691bd621`

The complete release tree is retained as a libc porting reference. The smoke
target configures mlibc's official Meson build in headers-only/demo-sysdeps
mode and compiles a TypePHP-OS API header probe. mlibc 7.0's implementation
build requires C++23 and GCC 13 or newer, while TypePHP-OS keeps C++17 as its
runtime baseline. Its implementation therefore does not replace the current
small libc yet; doing so also requires a real TypePHP-OS sysdeps port.

The mlibc 6 series does not have one uniform compiler baseline. Releases 6.0
and 6.1 request C++20 and do not contain the GCC 13 gate added later, so they
are more practical with an older compiler. Releases 6.2 and 6.3 already
request C++23. None of them satisfies TypePHP-OS's C++17 baseline unchanged;
using 6.0/6.1 would therefore be a separate port/toolchain choice rather than
a transparent downgrade of the pinned reference.

## Toybox

- Upstream: <https://codeberg.org/landley/toybox>
- Version: `0.8.14`
- Commit: `b7ec52ac35e075caffca5d330995d44e8dbfc8c3`
- Archive SHA-256: `827e4cdfd69f5da973e00e2a59b30b3c9857fb7fae74c362fd0b4f96be7929b0`
- Selected applets: [`tools/toybox-miniconfig`](tools/toybox-miniconfig)

The hosted probe enables only `cat`, `date`, `echo`, `pwd`, and `uname`; it
does not enable Toybox's shell or process-management commands. The undefined
host ABI is written to `build/thirdparty-smoke/toybox-undefined-symbols.txt`.
`ioctl` is a system-call API used by Toybox's shared C support library, not a
Toybox command. None of those five applet source files calls it directly, but
the common terminal/daemon helpers still leave an `ioctl` reference in the
hosted binary. Toybox is therefore intentionally not linked into TypePHP-OS.
The project will not implement `ioctl` or `termios` merely to satisfy it;
adoption requires pruning the unused common helpers or completing a different
compatible libc/runtime path.
