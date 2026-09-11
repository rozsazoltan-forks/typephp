#!/usr/bin/env bash

set -euo pipefail

project_dir=$(cd -- "$(dirname -- "$0")/.." && pwd)
build_dir="${project_dir}/build/thirdparty-smoke"
meson_command=${MESON:-meson}

if [[ ! -f "${project_dir}/build/libcompiler-rt-builtins.a" ]]; then
    echo "compiler-rt archive is missing; run make compiler-rt-builtins first" >&2
    exit 1
fi
for symbol in __udivti3 __udivmodti4 __divti3 __modti3 __multi3; do
    if ! nm -g --defined-only "${project_dir}/build/libcompiler-rt-builtins.a" \
            | grep -qE "[[:space:]]${symbol}$"; then
        echo "compiler-rt archive does not define ${symbol}" >&2
        exit 1
    fi
done
echo "compiler-rt builtins archive: OK"

if ! command -v "${meson_command}" >/dev/null 2>&1; then
    echo "Meson 1.3 or newer is required for the mlibc headers probe" >&2
    exit 1
fi

mlibc_build="${build_dir}/mlibc-build"
mlibc_sysroot="${build_dir}/mlibc-sysroot"
rm -rf "${mlibc_build}" "${mlibc_sysroot}"
"${meson_command}" setup \
    "${mlibc_build}" "${project_dir}/thirdparty/mlibc" \
    --cross-file "${project_dir}/tools/mlibc-demo-cross.ini" \
    --prefix=/usr \
    -Dheaders_only=true \
    -Dposix_option=enabled \
    -Dlinux_option=disabled \
    -Dglibc_option=disabled \
    -Dbsd_option=disabled
DESTDIR="${mlibc_sysroot}" "${meson_command}" install -C "${mlibc_build}"
compiler_headers=$(gcc -print-file-name=include)
gcc -std=c11 -ffreestanding -fsyntax-only -nostdinc \
    -isystem "${mlibc_sysroot}/usr/include" \
    -isystem "${compiler_headers}" \
    "${project_dir}/tools/mlibc-headers-smoke.c"
echo "mlibc public-header ABI probe: OK"

toybox_source="${project_dir}/thirdparty/toybox"
toybox_config="${build_dir}/toybox.config"
mkdir -p "${build_dir}"
KCONFIG_ALLCONFIG="${project_dir}/tools/toybox-miniconfig" \
KCONFIG_CONFIG="${toybox_config}" \
CFLAGS="-O2 -fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0" \
    make -C "${toybox_source}" allnoconfig toybox

test "$("${toybox_source}/toybox" | tr ' ' '\n' | sed '/^$/d' | sort | tr '\n' ' ')" \
    = "cat date echo pwd uname "
test "$("${toybox_source}/toybox" echo 'Toybox smoke: OK')" = "Toybox smoke: OK"
"${toybox_source}/toybox" date -u +%Y-%m-%d >/dev/null
"${toybox_source}/toybox" uname -s >/dev/null

toybox_symbols="${build_dir}/toybox-undefined-symbols.txt"
nm -u "${toybox_source}/generated/unstripped/toybox" \
    | awk '{ print $NF }' | sed 's/@.*//' | sort -u > "${toybox_symbols}"
if grep -Eq '^(fork|vfork|execv|execve|execvp|posix_spawn)$' "${toybox_symbols}"; then
    echo "the selected Toybox applets unexpectedly require process creation" >&2
    exit 1
fi
echo "Toybox selected applets build: OK"
if grep -qx ioctl "${toybox_symbols}"; then
    echo "Toybox install: deferred (the common runtime still references unsupported ioctl)"
else
    echo "Toybox install: ABI audit no longer reports ioctl; reassess target linking"
fi
echo "Toybox ABI report: ${toybox_symbols}"
