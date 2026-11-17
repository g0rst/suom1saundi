# suom1saundi

Firmware für den **M-VAVE FM-1**: eine Groovebox für **Suomisaundi**, finnischen Freestyle-Psytrance. Psy-Kick,
galoppierender Filterbass, Acid-Linie, 8-Bit-Melodie wie ein alter Soundchip und FM-Zaps, auf einem polymetrischen
Sequencer mit Slides, Chance und **SEKO** – dem Regler, der den Groove jedes Mal ein bisschen anders spielen lässt.
Der Generator schreibt zusammenpassende Tonfolgen über einem Akkord pro Pattern. **MINI** ist der FM-1 allein,
**MAXI** der FM-1 mit einem **M-VAVE SMK-37** als Steuerzentrale (Tasten, Pads, 16 Knobs, 8 Fader). Und als eigene
Firmware läuft suom1saundi **direkt auf dem SMK-37** (Zweig `smk37`, noch ungetestet).

![Display](docs/screen.png)

- **Im Browser spielen:** [Emulator](https://g0rst.github.io/suom1saundi/emulator.html) (oder `docs/emulator.html`
  direkt öffnen). Derselbe C-Kern wie in der Firmware, als WebAssembly.
  Für die SMK-37-Version: [SMK-37-Emulator](https://g0rst.github.io/suom1saundi/emulator-smk37.html)
  (`docs/emulator-smk37.html`) mit 37 Tasten, 16 Pads, 8 Reglern, 4 Fadern und den Rädern.
- **Handbuch:** [Deutsch](https://g0rst.github.io/suom1saundi/handbuch.html) ·
  [English](https://g0rst.github.io/suom1saundi/manual.html) ·
  [tlhIngan Hol](https://g0rst.github.io/suom1saundi/tlhingan.html)
  (Quellen: [docs/HANDBUCH.md](docs/HANDBUCH.md), [docs/MANUAL.md](docs/MANUAL.md), [docs/TLHINGAN.md](docs/TLHINGAN.md)).
  Die Tabellen darin schreibt `tools/manual.py` bei jedem Push aus dem Code neu.

## Stand

| Teil | Zustand |
| --- | --- |
| Kern (`app/core/suomi.c`): Klang, Sequencer, Generator, Display | läuft, Host-Tests grün (`tests/run_tests.sh`) |
| Browser-Emulator (`docs/emulator.html`) | läuft (WebAssembly aus demselben Kern) |
| FM-1-Anbindung (`app/app.c`) im Host-Simulator von fm1-base | läuft, Testskript `app/tests/basic.txt` grün (inkl. Speichern und Neustart) |
| Firmware-Paket `.fwsc` | baut GitHub aus dem Quellcode (`.github/workflows/release.yml`) |
| Lauf auf einem echten FM-1 | **noch nicht getestet** |
| SMK-37-Firmware (`./build.sh --board smk37`, `smk37-port.yaml`) | baut in der Cloud für Pro, Elite, MKE; **noch nicht getestet** |

**Vor dem Flashen:** Custom-Firmware kann einen FM-1 unbrauchbar machen. Vorher auf die offizielle V15 gehen, Daten
sichern und wissen, wie man zurückkommt (Installer von fm1-base, `base/README.md`).

## Stimmen

| Spur | Synthese | EDIT (K1–K4) | EDIT² (K1–K4) |
| --- | --- | --- | --- |
| **POTKU** | Sinus mit tiefem Pitch-Sweep, Klick, Soft-Clipper | TUNE, DECAY, SWEEP, DRIVE | SNAP, CLICK, HOLD, BODY |
| **BASSO** | Säge/Rechteck + Sub, 18-dB-Resonanzfilter mit Hüllkurve, Slides | CUTOFF, RESO, ENVMOD, DECAY | WAVE, SUB, GLIDE, DRIVE |
| **HATTU** | FM-Metall gegen Rauschen, 8-Bit-Rauschkanal (LFSR) | DECAY, TONE, METAL, OPEN | CHIP, SHAKE, SPREAD, CUT |
| **HAPPO** | Acid-Linie im 303-Stil: Akzent, Slide | CUTOFF, RESO, ENVMOD, DECAY | WAVE, ACCENT, GLIDE, DRIVE |
| **PIIP** | Soundchip: Puls 12/25/50 %, 4-Bit-Dreieck, Säge, Rauschen; Arpeggio, Vibrato | WAVE, DECAY, ARP, VIBRATO | SPEED, PWM, BEND, DETUNE |
| **OUTO** | FM-Paar mit Feedback und Zap-Hüllkurve: Laser, Blubbern | DECAY, FM, RATIO, ZAP | ZTIME, FEEDBK, WOBBLE, RANDOM |

Pro Spur dazu ein **LFO** im Takt (EDIT ×3) und ein Kanalzug im **Mixer** (ENV): Pegel, Pan, Delay, Hall, TONE-Filter,
**8-Bit-Crusher** (BITS, RATE), Ducking, mit Pegelanzeigen aller Spuren. Master: Drive, Effekt-Returns, Stereobreite.
44,1 kHz, Blöcke zu 256 Frames.

## Bedienung (Kurzfassung)

| Bedienelement | Funktion |
| --- | --- |
| PLAY | Start und Stopp |
| 16 weiße Tasten | Steps der Spur; halten + K1/K2/K3/K4: Note, Chance, Akzent/Slide, Ratchet |
| schwarze Tasten 1–6 | Spur: POTKU, BASSO, HATTU, HAPPO, PIIP, OUTO |
| schwarze Tasten 7–11 | CLR, ◀, ▶, NEW (ganzer Groove neu), UNDO |
| HOME | Tempo, Swing, **SEKO**, Duck |
| EDIT, ×2, ×3 | die acht Klangregler der Spur (Step halten: Parameter-Lock); MOD (LFO) |
| ENV, ×2, ×3 | Mixer; Kanal (TONE, BITS, RATE, DUCK); Master (DRIVE, Returns, WIDTH) |
| FX, FX² | Delay und Hall; Sidechain |
| GLO, ×2, ×3 | Grundton, Skala, MIDI-Clock, Chain; XPOSE, Akkord; SMK-37 (MAXI, Kanäle, SEL lernt) |
| SEQ, ×2, ×3 | Euklid, Link, Timing |
| ALGORITHM / PRESETS | Looplänge / Pattern A–D |
| SEL / ARP / LFO | Spur neu / leicht verändern / stumm (halten: Solo) |
| REC | Tasten spielen die Spur |
| SAVE | speichern; halten + PRESETS/SELECT: Pattern/Spur kopieren |

**MAXI mit dem SMK-37:** MIDI-Kabel vom SMK-37 an die MIDI-Buchse des FM-1. Ab Werk: Tasten Kanal 1, Pads Kanal 10
ab Note 36, Knobs CC 20–35, Fader CC 41–48 (in MIDI SUITE so einstellen, oder GLO ×3 und SEL: der FM-1 lernt die
Belegung). K1–K8 Klangregler, K9–K16 LFO und Kanal, F1–F6 Pegel, F7/F8 Delay- und Hall-Return, Pads 1–6 spielen die
Spuren, 7/8 NEW und MUTATE, 9–14 Mute, 15 neuer Groove, 16 UNDO, Mod-Rad SEKO, Pitchbend ±2 Halbtöne. Arpeggiator,
Akkorde und Skalen liefert das SMK-37 selbst.

**Direkt auf dem SMK-37:** Tasten spielen die Spur (Klavier), SEQ REC kurz wechselt zu STEPS (F3–G5 wie am FM-1,
C3–D#3 Pattern A–D, E3 CHAIN), SEQ REC halten nimmt auf; Pads = die 16 Steps der Spur, Pad halten + K1–K4 bearbeitet
den Step; K1–K8 in drei Bänken (SOUND, LFO+CHAN, FM-1); Fader Bank A Pegel POTKU–HAPPO, Bank B PIIP, OUTO, Delay,
Hall; Pitch-Rad ±2 Halbtöne, Mod-Rad SEKO. Vor dem Aufspielen mit M-Flasher eine volle Sicherung machen.

Alles Weitere im [Handbuch](docs/HANDBUCH.md).

## Bauen

```
git clone --recursive https://github.com/g0rst/suom1saundi
sh tests/run_tests.sh        # Host-Tests des Kerns (schreibt groove.wav und Screenshots nach build/tests)
./build.sh --host            # Simulator von fm1-base; danach tests/run_tests.sh nochmal
sh sim/build.sh              # Browser-Emulator: docs/emulator.html
./build.sh                   # .fwsc-Paket (braucht JieLis Toolchain und AC79-SDK: base/README.md)
./build.sh --board smk37     # dasselbe für das SMK-37 (--model elite / mke); wendet smk37/fm1-base.patch an
python3 tools/manual.py      # Handbücher aus dem Code; --check prüft nur
```

GitHub Pages einmalig einschalten: **Settings → Pages → Source: GitHub Actions**. Danach veröffentlicht jeder Push auf
`main` Emulator und Handbücher.

## Herkunft und Lizenz

MIT, siehe [LICENSE](LICENSE). Sequencer, Bedienung und Display stammen aus
[ro:MINImal](https://github.com/g0rst/ro-MINImal), 8-Bit-Ideen aus [memetek-fm1](https://github.com/g0rst/memetek-fm1),
Acid- und Filter-Ideen aus [neur0psy](https://github.com/g0rst/neur0psy). Plattform:
[fm1-base](https://github.com/charlesvestal/fm1-base) auf felucca-bsp (MPL-2.0). Die SMK-37-Version enthält den
Tastatur-Treiber aus [genkigenki/fm1-x0x](https://github.com/genkigenki/fm1-x0x) (GPL-3.0, über
`smk37/fm1-base.patch`) und steht als Ganzes unter GPL-3.0. Nicht mit M-VAVE verbunden.
