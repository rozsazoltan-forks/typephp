#!/usr/bin/env sh

set -eu

tools_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

"${tools_dir}/fetch-openlibm.sh"
"${tools_dir}/fetch-compiler-rt-builtins.sh"
"${tools_dir}/fetch-toybox.sh"
