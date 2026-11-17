# suom1saundi – Hinweise für die Arbeit an diesem Repository

suom1saundi ist eine Suomisaundi-Groovebox-Firmware für den M-VAVE FM-1 auf fm1-base. Der Instrumenten-Kern ist
`app/core/suomi.c` (portables C99 ohne libc): dieselbe Datei läuft auf dem Gerät (`app/app.c`), im Browser-Emulator
(`sim/`, WebAssembly) und in den Host-Tests (`tests/`).

- **Die drei Handbücher bei jeder sichtbaren Änderung mitpflegen:** `docs/HANDBUCH.md` (Deutsch), `docs/MANUAL.md`
  (Englisch) und `docs/TLHINGAN.md` (tlhIngan Hol). Danach `python3 tools/manual.py` ausführen: Es schreibt die
  Tabellen zwischen `<!-- GEN:… -->` aus dem Code neu und erzeugt `docs/handbuch.html`, `manual.html`, `tlhingan.html`
  und `index.html`. Neue Klangregler brauchen eine Beschreibung in `DESC` in `tools/manual.py`, in allen drei Sprachen.
- Neue Version: `VERSION` erhöhen und eine Zeile in `docs/CHANGES.tsv` (Deutsch, Englisch, tlhIngan Hol).
- Nach Änderungen am Kern: `sh tests/run_tests.sh` und `sh sim/build.sh` (der Emulator `docs/emulator.html`).
- Die README-Tabellen sind eine Kurzfassung des Handbuchs: zusammen pflegen.
- Ein Push auf `main` baut Tests, Emulator und Handbücher neu, committet Änderungen zurück und veröffentlicht `docs/`
  auf https://g0rst.github.io/suom1saundi/ (`.github/workflows/pages.yml`).
