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

This target builds a deliberately small hosted Toybox and audits its undefined
ABI. It is not installed into the TypePHP-OS disk image by this probe.

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
hosted binary. TypePHP-OS now exposes the Linux-numbered syscall and implements
`TIOCGWINSZ` for its fixed console. Toybox still needs a freestanding build and
ABI audit before it can replace any existing command.
