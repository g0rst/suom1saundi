#!/bin/sh
# SPDX-License-Identifier: MIT
# The core's host tests: builds tests/test_core.c with the system compiler and runs it.
# Writes groove.wav and the screens (PPM) into build/tests/.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/build/tests"
mkdir -p "$OUT"
${CC:-cc} -std=c99 -O2 -Wall -Wextra -Wno-type-limits -I"$ROOT/app/core" \
    "$ROOT/tests/test_core.c" "$ROOT/app/core/suomi.c" -lm -o "$OUT/test_core"
"$OUT/test_core" "$OUT"
# The firmware's app in fm1-base's host simulator, if it was built (./build.sh --host)
SIM="$ROOT/app/build/host/suom1saundi_host"
if [ -x "$SIM" ]; then
    "$SIM" "$ROOT/app/tests/basic.txt" "$ROOT/app/build/host" | tee "$OUT/sim.log" | tail -3
    ! grep -q "EXPECTATIONS FAILED" "$OUT/sim.log"
else
    echo "host simulator not built (./build.sh --host): skipped"
fi
