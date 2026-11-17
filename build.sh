#!/bin/sh
# SPDX-License-Identifier: MIT
# Builds suom1saundi with fm1-base's build tool (base/, a git submodule: git submodule update --init --recursive).
#   ./build.sh                 the firmware package app/build/suom1saundi.fwsc
#                              (needs JieLi's toolchain and AC79 SDK: base/README.md, base/tools/get_toolchain.sh)
#   ./build.sh --host          fm1-base's host simulator: app/build/host/suom1saundi_host
#   ./build.sh --release X.Y   a release package
# The browser test page is built by sim/build.sh, the core's own tests by tests/run_tests.sh.
set -e
cd "$(dirname "$0")"
[ -f base/tools/build.py ] || { echo "base/ is empty: run  git submodule update --init --recursive" >&2; exit 1; }
exec python3 base/tools/build.py app "$@"
