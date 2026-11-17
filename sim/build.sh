#!/bin/sh
# SPDX-License-Identifier: MIT
# Builds the browser emulator: the core compiled to WebAssembly (clang with the wasm32 target and wasm-ld; no
# Emscripten, no libc), embedded in sim/page.html -> docs/emulator.html and sim/page-smk37.html -> docs/emulator-smk37.html, each one
# self-contained file.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/build/sim"
mkdir -p "$OUT" "$ROOT/docs"
${CLANG:-clang} --target=wasm32 -O2 -ffreestanding -nostdlib -fno-builtin -Wall \
    -Wl,--no-entry -Wl,--strip-all -Wl,-z,stack-size=65536 \
    -o "$OUT/suomi.wasm" "$ROOT/sim/wasm.c" "$ROOT/app/core/suomi.c"
# the FM-1's page, and the SMK-37's (suom1saundi on the keyboard itself: its own panel, the same core)
python3 - "$OUT/suomi.wasm" "$ROOT/sim/page.html" "$ROOT/docs/emulator.html" \
    "$ROOT/sim/page-smk37.html" "$ROOT/docs/emulator-smk37.html" <<'PY'
import base64, sys
wasm, pairs = sys.argv[1], sys.argv[2:]
blob = open(wasm, "rb").read()
for page, dest in zip(pairs[::2], pairs[1::2]):
    html = open(page, encoding="utf-8").read()
    assert html.count("@@WASM@@") == 1
    open(dest, "w", encoding="utf-8").write(html.replace("@@WASM@@", base64.b64encode(blob).decode()))
    print(f"{dest}: {len(html)} bytes of page, {len(blob)} bytes of WebAssembly")
PY
