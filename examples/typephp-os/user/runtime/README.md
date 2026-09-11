# TypePHP userspace runtime

This directory contains the TypePHP-OS platform layer shared by all
tpc-generated userspace programs. The top-level Makefile compiles it, the
kernel ABI shims, and OpenLibm into:

```text
build/libtypephp-os.a
```

Build the archive once with:

```shell
make user-runtime
```

A TypePHP project should contain only its own application sources and link the
shared archive through its project file:

```yaml
link-paths:
  - ../../build
link-libs:
  - typephp-os
```

The archive owns the userspace `_start`, PHP Nano host entry, syscall bridge,
POSIX/libc and C++ ABI shims, buffered `dirent` implementation, and math implementation. Application projects
must not compile private copies of these files. This allows any number of
projects under `user/` to reuse one platform archive and keeps their generated
object directories independent.

No compiler runtime library is currently required on x86-64; both the kernel
and Nano userspace links are audited for undefined symbols. Add selected
compiler-rt builtins only if generated code introduces an explicit helper
symbol in the future.

The archive and each consumer must use the same freestanding ABI flags and
PHP Nano/PHPX feature definitions. Those shared settings are maintained in the
top-level Makefile and the consumer project file respectively.
