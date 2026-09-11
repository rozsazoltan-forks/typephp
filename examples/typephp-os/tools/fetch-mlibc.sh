#!/usr/bin/env sh

set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
version=7.0.0
commit=7c2a178142625cc9852e59a1a090468c61a62d3b
archive_sha256=56d62366a6a15bc3fd8741bebcb311ca543553f7b734b1e780aeae33691bd621
target="${project_dir}/thirdparty/mlibc"
marker="${target}/.typephp-source-${version}"

if [ -f "${marker}" ] && [ -f "${target}/meson.build" ]; then
    exit 0
fi

temporary=$(mktemp -d "${TMPDIR:-/tmp}/typephp-os-mlibc.XXXXXX")
trap 'rm -rf "${temporary}"' EXIT HUP INT TERM
archive="${temporary}/mlibc.tar.gz"
staging="${temporary}/installed"

echo "Downloading mlibc v${version} (${commit})"
curl -L --fail --retry 3 --silent --show-error -o "${archive}" \
    "https://github.com/managarm/mlibc/archive/refs/tags/v${version}.tar.gz"
printf '%s  %s\n' "${archive_sha256}" "${archive}" | sha256sum -c -
mkdir -p "${staging}"
tar -xzf "${archive}" -C "${staging}" --strip-components=1
test -f "${staging}/LICENSE"

mkdir -p "${project_dir}/thirdparty"
rm -rf "${target}"
mv "${staging}" "${target}"
printf '%s\n' "${commit}" > "${marker}"
echo "Installed mlibc v${version} in ${target}"
