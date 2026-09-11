# Third-party source policy

The generated `thirdparty/` directory is intentionally ignored by Git. Every
third-party source dependency must instead be declared by a script under
`tools/` with all of the following information:

- an immutable upstream version or commit;
- the canonical download URL;
- a SHA-256 checksum for the downloaded archive;
- an explicit list of files copied into the build tree;
- the upstream license file.

Run the common fetch entry point before compiling:

```shell
./tools/fetch-thirdparty.sh
```

`make` invokes this command automatically. A matching local version marker
makes repeated invocations offline and effectively free. Adding another
library must extend this same entry point or call a dedicated, equivalently
strict fetch script from it; third-party source must not be committed directly.

## OpenLibm

- Upstream: <https://github.com/JuliaMath/openlibm>
- Version: `v0.8.7`
- Commit: `9fbeafcd4f1b6ef6aa3946c1c8faead50f38a94d`
- Archive SHA-256: `e328a1d59b94748b111e022bca6a9d2fc0481fb57d23c87d90f394b559d4f062`
- Selected files: [`tools/openlibm-files.txt`](tools/openlibm-files.txt)

Only the architecture/compatibility headers and C sources required by
TypePHP-OS's current x86_64 double-precision math ABI are installed. The
selection keeps unused implementations out of the freestanding payload.
