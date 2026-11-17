# suom1saundi – Handbuch

suom1saundi ist eine Firmware für den **M-VAVE FM-1**, die aus dem kleinen FM-Synth eine Groovebox für
**Suomisaundi** macht: finnischen Freestyle-Psytrance. Das Genre ist hüpfend, verspielt und albern, mit rollenden
Bässen, zwitschernden Acid-Linien, Chiptune-Melodien und Geräuschen, die niemand erwartet. Genau dafür sind die sechs
Stimmen gebaut, und ein Regler namens **SEKO** (finnisch für „verrückt“) sorgt dafür, dass kein Takt genau wie der
letzte klingt.

> Dieses Handbuch aktualisiert sich selbst: Die Tabellen mit Reglern, Werkswerten, Seiten, Skalen und Versionen
> schreibt `tools/manual.py` bei jedem Push direkt aus dem Code der Firmware.

## Stand und Warnung

- Der Kern (`app/core/suomi.c`) läuft in den Host-Tests, im Simulator von fm1-base und im Browser-Emulator.
- Ein `.fwsc`-Paket für das Gerät braucht JieLis Toolchain und das AC79-SDK und wurde **noch nicht gebaut**.
- Auf einem echten FM-1 ist die Firmware **noch nicht getestet**. Custom-Firmware kann ein Gerät unbrauchbar machen.
  Vorher auf die offizielle V15 gehen, Daten sichern und wissen, wie man zurückkommt (Installer von fm1-base).

## Ausprobieren ohne Gerät

Der [Emulator](emulator.html) läuft im Browser mit genau demselben C-Kern wie die Firmware, als WebAssembly. Die
Bedienelemente sitzen wie am FM-1. Erst **Ton einschalten**, dann **PLAY**. Die Leertaste ist PLAY. Gespeichert wird
im Browser.

## Schnellstart

1. **PLAY** drücken: ab Werk läuft ein generierter Groove in Fis phrygisch-dominant bei 145 BPM.
2. Eine der ersten sechs **schwarzen Tasten** wählt eine Spur. Die 16 **weißen Tasten** sind ihre Steps.
3. **SEL** würfelt die gewählte Spur neu, **ARP** verändert sie nur ein bisschen, die schwarze Taste **NEW** würfelt
   den ganzen Groove neu, **UNDO** holt den vorherigen zurück.
4. **HOME** und **KNOB3** (SEKO) aufdrehen: Der Groove fängt an zu improvisieren.
5. **EDIT** und an den vier Knobs drehen: der Klang der Spur. Nochmal **EDIT**: die anderen vier Klangregler.
6. **SAVE** kurz drücken speichert alles.

## Die sechs Spuren

| Taste | Spur | Finnisch | Rolle |
|---|---|---|---|
| schwarz 1 | **POTKU** | Tritt | die Kick, vier auf den Boden |
| schwarz 2 | **BASSO** | Bass | Filterbass zwischen den Kicks, mit Oktavsprüngen |
| schwarz 3 | **HATTU** | Hut | Hats und Shaker, auf Wunsch mit 8-Bit-Rauschen |
| schwarz 4 | **HAPPO** | Säure | die Acid-Linie |
| schwarz 5 | **PIIP** | Piep | 8-Bit-Melodie wie ein alter Soundchip |
| schwarz 6 | **OUTO** | seltsam | FM-Zaps, Laser, Blubbern |

## Bedienelemente

| Element | Funktion |
|---|---|
| **PLAY** | Start und Stopp |
| **16 weiße Tasten** | die Steps der gewählten Spur; tippen schaltet um |
| **weiße Taste halten + K1** | Note des Steps (−24 … +24) |
| **weiße Taste halten + K2** | Chance: 100, 75, 50, 25 % |
| **weiße Taste halten + K3** | aus, Akzent (ACC), Slide (SLD), beides (A+S) |
| **weiße Taste halten + K4** | Ratchet: 1 bis 4 Schläge im Step |
| **weiße Taste halten + SELECT** | Mikrotiming: früher oder später |
| **weiße Taste halten + SEL** | Locks und Verschiebung des Steps löschen |
| **schwarze Tasten 1–6** | Spur wählen (ohne laufenden Groove hört man sie) |
| **schwarze Tasten 7–11** | CLR (Spur leeren), ◀ ▶ (Spur verschieben), NEW (ganzer Groove neu), UNDO |
| **SELECT**, **OCT− / OCT+** | Spur wählen |
| **ALGORITHM** | Looplänge der Spur, 1 bis 16 Steps |
| **PRESETS** | Pattern A bis D; beim Spielen ab dem nächsten Takt |
| **SEL** | Spur neu würfeln |
| **ARP** | Spur ein bisschen verändern (Mutate) |
| **LFO** | kurz: Spur stumm; gehalten: Solo |
| **REC** | Tasten spielen die Spur chromatisch; bei laufendem Groove wird aufgenommen |
| **SAVE** | kurz: speichern; halten + PRESETS: Pattern kopieren; halten + SELECT: Spur kopieren |
| **MASTER** | Lautstärke |

## Seiten

Die Tasten über den Knobs wählen, was **KNOB1** bis **KNOB4** tun. Ein zweiter Druck auf dieselbe Taste öffnet ihre
zweite Seite. Die Seite steht unten im Display.

<!-- GEN:pages -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| Taste | Seite | K1 | K2 | K3 | K4 |
|---|---|---|---|---|---|
| **HOME** | GROOVE | TEMPO | SWING | SEKO | DUCK |
| **EDIT** | SOUND | p1 | p2 | p3 | p4 |
| **EDIT ×2** | SOUND 2 | p5 | p6 | p7 | p8 |
| **ENV** | MIX | LEVEL | PAN | DELAY | REVERB |
| **ENV ×2** | 8-BIT | BITS | RATE |  |  |
| **FX** | FX | D.TIME | D.FDBK | R.SIZE | R.DAMP |
| **FX ×2** | SIDECHAIN | SOURCE | DEPTH | RELEASE | AMOUNT |
| **GLO** | GLOBAL | ROOT | SCALE | CLOCK | CHAIN |
| **GLO ×2** | GLOBAL 2 | XPOSE | DRIVE |  |  |
| **SEQ** | EUCLID | HITS | ROTATE | CHANCE | ACCENT |
| **SEQ ×2** | LINK | SOURCE | MODE |  |  |
| **SEQ ×3** | TIMING | STEP n | TRACK |  |  |
<!-- /GEN:pages -->

p1 bis p8 sind die acht Klangregler der gewählten Spur (siehe unten).

## Klänge

Jede Spur hat acht Klangregler: vier auf **EDIT**, vier auf **EDIT²** (EDIT zweimal drücken). Werkswerte gehen von
0 bis 127.

<!-- GEN:sound -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
### POTKU

Psy-Kick: Sinus mit tiefem, schnellem Pitch-Sweep, Klick, Soft-Clipper

| Seite | Regler | Name | Werk | Was er tut |
|---|---|---|---|---|
| EDIT | K1 | **TUNE** | 48 | Grundton der Kick |
| EDIT | K2 | **DECAY** | 58 | Ausklang |
| EDIT | K3 | **SWEEP** | 62 | wie tief die Tonhöhe herunterfällt: der Psy-„Tock“ |
| EDIT | K4 | **DRIVE** | 40 | Sättigung |
| EDIT² | K1 | **SNAP** | 40 | wie schnell der Sweep fällt |
| EDIT² | K2 | **CLICK** | 50 | Klick am Anfang |
| EDIT² | K3 | **HOLD** | 20 | volle Lautstärke halten, bevor sie fällt (bis 86 ms) |
| EDIT² | K4 | (frei) |  |  |

### BASSO

Filterbass mit Sub, galoppierend zwischen den Kicks, Oktavsprünge und Slides

| Seite | Regler | Name | Werk | Was er tut |
|---|---|---|---|---|
| EDIT | K1 | **CUTOFF** | 52 | Grenzfrequenz des Tiefpasses (18 dB) |
| EDIT | K2 | **RESO** | 40 | Resonanz |
| EDIT | K3 | **ENVMOD** | 60 | wie weit die Hüllkurve den Filter öffnet |
| EDIT | K4 | **DECAY** | 34 | Ausklang von Lautstärke und Filter |
| EDIT² | K1 | **WAVE** | 0 | Säge (0) bis Rechteck (127) |
| EDIT² | K2 | **SUB** | 40 | Sinus eine Oktave tiefer |
| EDIT² | K3 | **GLIDE** | 50 | Gleitzeit bei Slide |
| EDIT² | K4 | **DRIVE** | 40 | Verzerrung vor dem Filter |

### HATTU

Hats und Shaker: FM-Metall, Rauschen und ein 8-Bit-Rauschkanal

| Seite | Regler | Name | Werk | Was er tut |
|---|---|---|---|---|
| EDIT | K1 | **DECAY** | 40 | Ausklang |
| EDIT | K2 | **TONE** | 72 | Tonhöhe des Metalls |
| EDIT | K3 | **METAL** | 70 | Rauschen (0) bis FM-Metall (127) |
| EDIT | K4 | **OPEN** | 64 | wie weit ein Akzent die Hat öffnet |
| EDIT² | K1 | **CHIP** | 0 | Mischung mit dem 8-Bit-Rauschkanal (LFSR) |
| EDIT² | K2 | **SHAKE** | 0 | weicher Anstieg: Shaker statt Hat |
| EDIT² | K3 | (frei) |  |  |
| EDIT² | K4 | (frei) |  |  |

### HAPPO

Acid-Linie im Stil der 303: Säge/Rechteck, 18-dB-Resonanzfilter, Akzent, Slide

| Seite | Regler | Name | Werk | Was er tut |
|---|---|---|---|---|
| EDIT | K1 | **CUTOFF** | 58 | Grenzfrequenz: die Acid-Linie |
| EDIT | K2 | **RESO** | 96 | Resonanz, bis kurz vor dem Pfeifen |
| EDIT | K3 | **ENVMOD** | 74 | Filterhüllkurve |
| EDIT | K4 | **DECAY** | 44 | Ausklang; Akzente klingen länger |
| EDIT² | K1 | **WAVE** | 0 | Säge bis Rechteck |
| EDIT² | K2 | **ACCENT** | 90 | wie stark ein Akzent Filter und Lautstärke hochreißt |
| EDIT² | K3 | **GLIDE** | 60 | Gleitzeit bei Slide |
| EDIT² | K4 | **DRIVE** | 60 | Verzerrung |

### PIIP

8-Bit-Melodie wie ein Soundchip: Pulswellen, Arpeggios, Vibrato, Bend

| Seite | Regler | Name | Werk | Was er tut |
|---|---|---|---|---|
| EDIT | K1 | **WAVE** | 30 | Soundchip-Welle: Puls 12/25/50 %, 4-Bit-Dreieck, Säge, gestimmtes Rauschen |
| EDIT | K2 | **DECAY** | 46 | Ausklang |
| EDIT | K3 | **ARP** | 40 | Akkord als schnelles Arpeggio: OFF OCT MIN MAJ SUS4 5+8 DIM |
| EDIT | K4 | **VIBRATO** | 30 | Vibrato-Tiefe |
| EDIT² | K1 | **SPEED** | 64 | Tempo von Arpeggio und Vibrato |
| EDIT² | K2 | **PWM** | 30 | Pulsbreiten-Modulation |
| EDIT² | K3 | **BEND** | 64 | Tonhöhe beim Anschlag: von unten (−) oder oben (+) |
| EDIT² | K4 | **DETUNE** | 0 | zweiter, verstimmter Oszillator |

### OUTO

„seltsam“: FM-Zaps, Laser, Blubbern, Quietschen; jeder Schlag anders

| Seite | Regler | Name | Werk | Was er tut |
|---|---|---|---|---|
| EDIT | K1 | **DECAY** | 60 | Ausklang |
| EDIT | K2 | **FM** | 80 | FM-Index: wie schräg |
| EDIT | K3 | **RATIO** | 48 | Frequenzverhältnis der Operatoren |
| EDIT | K4 | **ZAP** | 100 | Laser: Tonhöhe fällt von oben (+) oder steigt von unten (−) |
| EDIT² | K1 | **ZTIME** | 50 | Dauer des Zaps |
| EDIT² | K2 | **FEEDBK** | 40 | Feedback des Modulators: Knurren |
| EDIT² | K3 | **WOBBLE** | 0 | Tonhöhen-LFO: Blubbern |
| EDIT² | K4 | **RANDOM** | 50 | jeder Schlag ein bisschen anders |

### Mix ab Werk

| Spur | Pegel | Pan | Delay | Hall |
|---|---|---|---|---|
| **POTKU** | 122 | C | 0 | 0 |
| **BASSO** | 108 | C | 0 | 0 |
| **HATTU** | 84 | R12 | 14 | 14 |
| **HAPPO** | 84 | L12 | 60 | 20 |
| **PIIP** | 76 | R16 | 70 | 40 |
| **OUTO** | 80 | C | 60 | 50 |
<!-- /GEN:sound -->

## Der Sequencer

- **Polymetrie:** Jede Spur hat ihre eigene Looplänge (ALGORITHM). HAPPO auf 14 gegen die 16 der Kick wiederholt sich
  erst nach sieben Takten.
- **Chance** pro Step, **Ratchets** bis zu vier Schlägen, **Akzente**.
- **Slides** auf BASSO, HAPPO und PIIP: Der Ton gleitet zur neuen Note, ohne neu anzuschlagen. Die Gleitzeit ist
  GLIDE (bei PIIP fest). Ein Slide ist nur zu hören, wenn die vorherige Note noch klingt.
- **Parameter-Locks:** Auf EDIT eine weiße Taste halten und K1 bis K4 drehen: Der Wert gilt nur für diesen Step.
  OUTO bekommt vom Generator auf jedem Step eigene Locks, deshalb klingt jeder Zap anders.
- **EUCLID** (SEQ): Schläge so gleichmäßig wie möglich über den Loop verteilt, mit Rotation, Chance und Akzent auf
  jedem n-ten Schlag.
- **LINK** (SEQ ×2): Die Spur spielt abhängig von einer anderen: WITH nur wenn die Quelle schlägt, GAPS nur in ihren
  Lücken, AFTER einen Step danach. Eine stumme Quelle wirkt weiter.
- **TIMING** (SEQ ×3): SELECT springt von Schlag zu Schlag, K1 verschiebt den Schlag, K2 die ganze Spur.

## SEKO

SEKO auf der Seite HOME ist der Suomisaundi-Regler. Bei 0 spielt der Sequencer genau, was programmiert ist. Je weiter
er aufgedreht ist, desto öfter spielt ein Step **etwas anderes**: eine Stufe der Skala höher oder tiefer, eine Oktave
daneben, mit Slide oder als Doppelschlag; Hats verdoppeln sich oder wechseln den Akzent. Dazu wandern Filter und
Ausklang langsam, und die Lautstärke schwankt leicht. Die Kick bleibt immer, wo sie ist.

## 8-Bit

ENV zweimal öffnet die Seite **8-BIT** der gewählten Spur:

- **BITS** rundet das Signal auf immer weniger Stufen (OFF, dann gröber bis 127).
- **RATE** hält jeden Wert mehrere Samples lang: die Abtastrate sinkt, der Klang wird metallisch und aliasing-reich.

Dazu klingen PIIP (Pulswellen, 4-Bit-Dreieck, gestimmtes Rauschen) und der CHIP-Regler von HATTU (ein Schieberegister
wie im Soundchip) schon von sich aus nach 8 Bit. Die Idee dafür kommt aus memetek-fm1.

## Effekte und Sidechain

- **ENV**: Pegel, Panorama und Anteil an Delay und Hall pro Spur.
- **FX**: Delay-Zeit (1/16, 1/8, 3/16; rechts zwei Drittel davon), Feedback, Hallgröße, Dämpfung.
- **FX ×2** (SIDECHAIN): Quelle (ab Werk POTKU), Tiefe, Release und wie stark die gewählte Spur geduckt wird. Eine
  stumme Quelle duckt weiter (Ghost-Kick).
- **GLO ×2**: DRIVE sättigt die Summe, XPOSE transponiert das Pattern.

## Patterns, Chain und Tonart

Vier Patterns A bis D. **PRESETS** wechselt sie, beim Spielen zum nächsten Takt. **CHAIN** (GLO) spielt A-B, A-C oder
A-D reihum, je vier Takte. Ab Werk sind C und D um +3 und −2 Halbtöne transponiert (XPOSE), so wechselt die Kette die
Tonart. SAVE halten und PRESETS drehen kopiert das ganze Pattern, SAVE halten und SELECT drehen nur die Spur;
loslassen kopiert, UNDO im Ziel macht es rückgängig.

## Globale Werte

<!-- GEN:global -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| Name | Seite | Werk | Bereich | Was er tut |
|---|---|---|---|---|
| **TEMPO** | HOME | 145 | 120 … 180 | Tempo in BPM |
| **SWING** | HOME | 50 | 50 … 75 | Swing in % |
| **SEKO** | HOME | 40 | 0 … 127 | Zufall: Steps spielen mal anders, Klänge wandern |
| **DUCK** | HOME / FX² | 100 | 0 … 127 | Tiefe des Sidechain-Duckings |
| **ROOT** | GLO | F#1 | 24 … 47 | Grundton (MIDI-Note) |
| **SCALE** | GLO | PHRDOM | 0 … 127 | Skala (siehe unten) |
| **CLOCK** | GLO | ON | 0 … 1 | MIDI-Clock senden |
| **DRIVE** | GLO² | 30 | 0 … 127 | Sättigung der Summe |
| **D.TIME** | FX | 3/16 | 0 … 2 | Delay-Zeit 1/16, 1/8, 3/16 |
| **D.FDBK** | FX | 60 | 0 … 127 | Delay-Feedback |
| **R.SIZE** | FX | 60 | 0 … 127 | Hallgröße |
| **R.DAMP** | FX | 60 | 0 … 127 | Hall-Dämpfung |
| **RELEASE** | FX² | 50 | 0 … 127 | wie schnell das Ducking loslässt |
<!-- /GEN:global -->

## Skalen

Der Generator, SEKO und die Noten der Spuren folgen der Skala auf GLO.

<!-- GEN:scales -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| Skala | Was er tut |
|---|---|
| **MINOR** | Moll |
| **PHRYG** | Phrygisch: dunkel |
| **PHRDOM** | Phrygisch-dominant: die Psy-Skala (Werk) |
| **HARMIN** | Harmonisch Moll |
| **DORIAN** | Dorisch |
| **MAJOR** | Dur: die fröhliche Seite des Suomisaundi |
| **PENTA** | Moll-Pentatonik |
| **WHOLE** | Ganzton: outo |
<!-- /GEN:scales -->

## MIDI

- **In:** Kanäle 1 bis 6 spielen die Spuren (Note 60 = Grundnote des Steps). Kanal 10: 36 POTKU, 37–39 OUTO, 42 und 44
  HATTU, 46 HATTU offen. MIDI-Start und -Stopp starten und stoppen den Groove.
- **Out:** MIDI-Clock, Start und Stopp über USB, wenn CLOCK auf GLO an ist.

## Firmware bauen

```
git clone --recursive https://github.com/g0rst/suom1saundi
cd suom1saundi
sh tests/run_tests.sh        # Host-Tests des Kerns
./build.sh --host            # Simulator von fm1-base, danach nochmal tests/run_tests.sh
sh sim/build.sh              # Browser-Emulator: docs/emulator.html
./build.sh                   # .fwsc-Paket (braucht JieLis Toolchain und AC79-SDK)
python3 tools/manual.py      # Handbücher aus dem Code aktualisieren
```

## Herkunft

Sequencer, Bedienung und Display stammen aus [ro:MINImal](https://github.com/g0rst/ro-MINImal), die 8-Bit-Ideen aus
[memetek-fm1](https://github.com/g0rst/memetek-fm1), Acid und Filter-Ideen aus [neur0psy](https://github.com/g0rst/neur0psy).
Die Plattform ist [fm1-base](https://github.com/charlesvestal/fm1-base) auf felucca-bsp (MPL-2.0). suom1saundi selbst
steht unter MIT. Nicht mit M-VAVE verbunden.

## Versionen

<!-- GEN:version -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
Aktuelle Version: **0.1**

| Version | Datum | Änderungen |
|---|---|---|
| **0.1** | 2026-11-17 | Erste Version: sechs Stimmen, Sequencer mit Slides und SEKO, 8-Bit-Crusher, Browser-Emulator, Handbuch in drei Sprachen |
<!-- /GEN:version -->
