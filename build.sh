#!/bin/sh
# SPDX-License-Identifier: MIT
# Builds suom1saundi with fm1-base's build tool (base/, a git submodule: git submodule update --init --recursive).
#   ./build.sh                 the firmware package app/build/suom1saundi.fwsc
#                              (needs JieLi's toolchain and AC79 SDK: base/README.md, base/tools/get_toolchain.sh)
#   ./build.sh --host          fm1-base's host simulator: app/build/host/suom1saundi_host
#   ./build.sh --release X.Y   a release package
#   ./build.sh --board smk37 [--model pro|elite|mke] [...]
#                              the same for the M-VAVE SMK-37: app/build/smk37/suom1saundi-smk37.fwsc
#                              (smk37-port.yaml). fm1-base has no SMK-37 support of its own yet: the build copies
#                              base/ to app/build/.base-smk37/ and applies smk37/fm1-base.patch there, so base/ and
#                              the FM-1 build stay as they are.
# The browser test page is built by sim/build.sh, the core's own tests by tests/run_tests.sh.
set -e
cd "$(dirname "$0")"
[ -f base/tools/build.py ] || { echo "base/ is empty: run  git submodule update --init --recursive" >&2; exit 1; }
board="${BOARD:-fm1}"
prev=""
for a in "$@"; do
    case "$a" in --board=*) board="${a#--board=}" ;; esac
    [ "$prev" = "--board" ] && board="$a"
    prev="$a"
done
if [ "$board" = "fm1" ]; then
    exec python3 base/tools/build.py app "$@"
fi
[ "$board" = "smk37" ] || { echo "--board $board: fm1 or smk37" >&2; exit 1; }
# a fresh copy whenever base/ or the patch changed
stamp="$(git -C base rev-parse HEAD 2>/dev/null)-$(git -C base/bsp rev-parse HEAD 2>/dev/null)-$(cksum < smk37/fm1-base.patch)"
dst=app/build/.base-smk37
if [ ! -f "$dst/.stamp" ] || [ "$(cat "$dst/.stamp")" != "$stamp" ]; then
    rm -rf "$dst"
    mkdir -p "$dst"
    (cd base && tar cf - --exclude=.git .) | (cd "$dst" && tar xf -)
    patch -s -p1 -d "$dst" < smk37/fm1-base.patch
    printf '%s' "$stamp" > "$dst/.stamp"
fi
exec env BOARD=smk37 python3 "$dst/tools/build.py" app "$@"
