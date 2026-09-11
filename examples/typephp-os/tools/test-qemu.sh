#!/usr/bin/env bash

set -euo pipefail

kernel=${1:?kernel ELF is required}
disk=${2:?FAT16 disk image is required}
log=${3:?log path is required}

if timeout 8 qemu-system-x86_64 \
        -kernel "${kernel}" \
        -drive file="${disk}",format=raw,if=ide,index=0 \
        -display none \
        -serial stdio \
        -monitor none \
        -no-reboot \
        -no-shutdown \
        -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
        >"${log}" 2>&1 \
        <<< $'\nls\ncd DOCS\nls\ncd ..\nls\n'; then
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
grep -q "FAT16 root: DATA/, HELLO.TXT" "${log}"
grep -q "PHP file stream: PHP stream through TypePHP FAT16" "${log}"
grep -q "PHP directory scan: ., .., DATA, DOCS, HELLO.TXT, STREAM.TXT" "${log}"
grep -q "Calculate primes: 0-100" "${log}"
grep -q "Prime count: 25" "${log}"
grep -q "Prime list: 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97" "${log}"
grep -Eq '^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2} Hello TypePHP-OS![[:space:]]*$' "${log}"
grep -q "Process 1: sh.elf (Ring 3)" "${log}"
grep -q "TypePHP-OS user shell" "${log}"
grep -q "Ring 3 confirmed" "${log}"
grep -Fq 'typephp-os:/$ ' "${log}"
grep -Fq 'typephp-os:/DOCS$ ' "${log}"
grep -q '^HELLO.TXT' "${log}"
cat "${log}"
