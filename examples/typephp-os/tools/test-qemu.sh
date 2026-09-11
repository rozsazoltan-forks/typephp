#!/usr/bin/env bash

set -euo pipefail

kernel=${1:?kernel ELF is required}
disk=${2:?FAT16 disk image is required}
log=${3:?log path is required}

if timeout 20 qemu-system-x86_64 \
        -kernel "${kernel}" \
        -drive file="${disk}",format=raw,if=ide,index=0 \
        -display none \
        -serial stdio \
        -monitor none \
        -no-reboot \
        -no-shutdown \
        -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
        >"${log}" 2>&1 \
        <<< $'\ndate\necho Hello TypePHP userspace\nhello Dynamically loaded\nmissing\nbad\nls /BIN\ncat HELLO.TXT\ntouch /EXPAND/F62.TXT\nls /EXPAND\nwrite NOTE.TXT Hello from Ring 3\ncat NOTE.TXT\ntouch EMPTY.TXT\nmkdir TMP\nls\nrm NOTE.TXT\ncat NOTE.TXT\nrmdir TMP\ncd TMP\nmkdir WORK\ncd WORK\nwrite NOTE.TXT Nested directory write\ncat NOTE.TXT\nmkdir SUB\ncd SUB\nwrite DEEP.TXT Deep directory write\nmv DEEP.TXT MOVED.TXT\ncat MOVED.TXT\ncat DEEP.TXT\npwd\ncd ..\nrmdir SUB\nrm SUB/MOVED.TXT\nrmdir SUB\nls\nrm NOTE.TXT\ncd ..\nrmdir WORK\ncd WORK\nmemtest\nfault\nvmfault\nwrfault\ndate\npwd\ncd BIN\npwd\nls\ncd ..\ncd DOCS\npwd\nls\ncd ..\nls\n'; then
    status=0
else
    status=$?
fi

if [[ ${status} -ne 124 ]]; then
    cat "${log}"
    echo "QEMU exited with ${status}; expected the resident kernel loop to reach the timeout" >&2
    exit 1
fi

grep -q "TypePHP OS POC" "${log}"
grep -q "Int bits: 64" "${log}"
grep -Eq '^RAM MiB: [1-9][0-9]*' "${log}"
grep -q "Zend MiB: 2" "${log}"
grep -q "Zend string/array: OK" "${log}"
grep -q "Kernel is!" "${log}"
grep -q "OpenLibm math: OK" "${log}"
grep -q "FAT16 file: Hello from TypePHP FAT16!" "${log}"
grep -q "FAT16 root: BIN/" "${log}"
grep -q "FAT16 sector cache: OK" "${log}"
grep -q "PHP file stream: PHP stream through TypePHP FAT16" "${log}"
grep -q "PHP directory scan: ., ..," "${log}"
grep -q "Calculate primes: 0-100" "${log}"
grep -q "Prime count: 25" "${log}"
grep -q "Prime list: 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97" "${log}"
grep -q "Process 1: sh.elf (Ring 3)" "${log}"
grep -q "TypePHP-OS user shell" "${log}"
grep -q "Ring 3 confirmed" "${log}"
grep -q "Commands: ls, cd, pwd, date, cat, echo, write, touch, mkdir, rm, rmdir, mv, memtest, fault, vmfault, wrfault" "${log}"
grep -Eq '^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2} UTC' "${log}"
grep -q '^Hello TypePHP userspace' "${log}"
grep -q '^Dynamically loaded' "${log}"
grep -q '^sh: missing: No such file or directory' "${log}"
grep -q '^sh: bad: Exec format error' "${log}"
grep -q '^/BIN' "${log}"
grep -q '^Hello from TypePHP FAT16!' "${log}"
grep -q '^F62.TXT' "${log}"
grep -q '^Hello from Ring 3' "${log}"
grep -q '^EMPTY.TXT' "${log}"
grep -q '^TMP' "${log}"
grep -q '^cat: No such file or directory' "${log}"
grep -q '^cd: no such directory' "${log}"
grep -q '^Nested directory write' "${log}"
grep -q '^Deep directory write' "${log}"
grep -q '^typephp-os:/WORK/SUB\$ cat DEEP.TXT' "${log}"
grep -q '^/WORK/SUB' "${log}"
grep -q '^rmdir: Directory not empty' "${log}"
grep -Fq 'typephp-os:/WORK$ ' "${log}"
grep -q '^brk/mmap: OK' "${log}"
grep -q 'User process 2 fault: invalid opcode (#6)' "${log}"
grep -q 'sh: fault: Input/output error' "${log}"
grep -q 'User process 2 fault: page fault (#14) .*address 0x0000000002000000' "${log}"
grep -q 'sh: vmfault: Input/output error' "${log}"
grep -q 'User process 2 fault: page fault (#14) .*address 0x00000000022ef000' "${log}"
grep -q 'sh: wrfault: Input/output error' "${log}"
grep -Fq 'typephp-os:/$ ' "${log}"
grep -Fq 'typephp-os:/DOCS$ ' "${log}"
grep -Fq 'typephp-os:/BIN$ ' "${log}"
grep -q '^/DOCS' "${log}"
grep -q '^HELLO.TXT' "${log}"
cat "${log}"
