#!/bin/sh
# SPDX-License-Identifier: MIT
# Builds the browser emulator: the core compiled to WebAssembly (clang with the wasm32 target and wasm-ld; no
# Emscripten, no libc), embedded in sim/page.html -> docs/emulator.html, one self-contained file.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/build/sim"
mkdir -p "$OUT" "$ROOT/docs"
${CLANG:-clang} --target=wasm32 -O2 -ffreestanding -nostdlib -fno-builtin -Wall \
    -Wl,--no-entry -Wl,--strip-all -Wl,-z,stack-size=65536 \
    -o "$OUT/suomi.wasm" "$ROOT/sim/wasm.c" "$ROOT/app/core/suomi.c"
python3 - "$ROOT/sim/page.html" "$OUT/suomi.wasm" "$ROOT/docs/emulator.html" <<'PY'
import base64, sys
page, wasm, dest = sys.argv[1:]
html = open(page, encoding="utf-8").read()
assert html.count("@@WASM@@") == 1
open(dest, "w", encoding="utf-8").write(html.replace("@@WASM@@", base64.b64encode(open(wasm, "rb").read()).decode()))
print(f"{dest}: {len(html)} bytes of page, {len(open(wasm, 'rb').read())} bytes of WebAssembly")
PY
