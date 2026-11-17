# suom1saundi – Manual

suom1saundi is firmware for the **M-VAVE FM-1** that turns the little FM synth into a groovebox for **suomisaundi**:
Finnish freestyle psytrance. The genre is bouncy, playful and silly, with rolling basses, chirping acid lines, chiptune
melodies and noises nobody expects. The six voices are built for exactly that, and a knob called **SEKO** (Finnish
for "crazy") makes sure no bar sounds quite like the last.

> This manual updates itself: the tables of knobs, factory values, pages, scales and versions are written by
> `tools/manual.py` straight from the firmware's code on every push.

## Status and warning

- The core (`app/core/suomi.c`) runs in the host tests, in fm1-base's simulator and in the browser emulator.
- A `.fwsc` package for the device needs JieLi's toolchain and the AC79 SDK and has **not been built yet**.
- The firmware has **not been tested on a real FM-1**. Custom firmware can brick a device. Go to the official V15
  first, back up your data and know how to get back (fm1-base's installer).

## Try it without the device

The [emulator](emulator.html) runs in the browser with exactly the same C core as the firmware, as WebAssembly. The
controls sit as on the FM-1. Press **Ton einschalten** (sound on) first, then **PLAY**. The space bar is PLAY. Saving
goes to the browser.

## Quick start

1. Press **PLAY**: a generated groove in F♯ phrygian dominant at 145 BPM plays out of the box.
2. One of the first six **black keys** picks a track. The 16 **white keys** are its steps.
3. **SEL** rolls the track anew, **ARP** changes it just a little, the black key **NEW** rolls the whole groove,
   **UNDO** brings the previous one back.
4. **HOME** and turn up **KNOB3** (SEKO): the groove starts to improvise.
5. **EDIT** and turn the four knobs: the track's sound. **EDIT** again: the other four sound knobs.
6. A short press on **SAVE** stores everything.

## The six tracks

| Key | Track | Finnish for | Role |
|---|---|---|---|
| black 1 | **POTKU** | kick | the kick, four on the floor |
| black 2 | **BASSO** | bass | filter bass between the kicks, with octave jumps |
| black 3 | **HATTU** | hat | hats and shakers, with 8-bit noise if you like |
| black 4 | **HAPPO** | acid | the acid line |
| black 5 | **PIIP** | beep | an 8-bit melody like an old sound chip |
| black 6 | **OUTO** | weird | FM zaps, lasers, bubbles |

## Controls

| Control | Function |
|---|---|
| **PLAY** | start and stop |
| **16 white keys** | the steps of the chosen track; a tap toggles |
| **hold a white key + K1** | the step's note (−24 … +24) |
| **hold a white key + K2** | chance: 100, 75, 50, 25 % |
| **hold a white key + K3** | off, accent (ACC), slide (SLD), both (A+S) |
| **hold a white key + K4** | ratchet: 1 to 4 hits within the step |
| **hold a white key + SELECT** | microtiming: earlier or later |
| **hold a white key + SEL** | clear the step's locks and shift |
| **black keys 1–6** | pick a track (heard when the groove is stopped) |
| **black keys 7–11** | CLR (clear the track), ◀ ▶ (shift the track), NEW (whole groove anew), UNDO |
| **SELECT**, **OCT− / OCT+** | pick a track |
| **ALGORITHM** | the track's loop length, 1 to 16 steps |
| **PRESETS** | pattern A to D; while playing, from the next bar |
| **SEL** | roll the track anew |
| **ARP** | change the track a little (mutate) |
| **LFO** | tap: mute the track; hold: solo |
| **REC** | the keys play the track chromatically; recorded while the groove runs |
| **SAVE** | tap: save; hold + PRESETS: copy the pattern; hold + SELECT: copy the track |
| **MASTER** | volume |

## Pages

The buttons above the knobs choose what **KNOB1** to **KNOB4** do. A second press on the same button opens its
second page. The page's name is shown low on the screen.

<!-- GEN:pages -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| Button | Page | K1 | K2 | K3 | K4 |
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

p1 to p8 are the chosen track's eight sound knobs (below).

## Sounds

Each track has eight sound knobs: four on **EDIT**, four on **EDIT²** (press EDIT twice). Factory values run from 0 to
127.

<!-- GEN:sound -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
### POTKU

psy kick: a sine with a deep, fast pitch sweep, click, soft clipper

| Page | Knob | Name | Factory | What it does |
|---|---|---|---|---|
| EDIT | K1 | **TUNE** | 48 | the kick's pitch |
| EDIT | K2 | **DECAY** | 58 | how long it rings |
| EDIT | K3 | **SWEEP** | 62 | how far the pitch falls: the psy 'tok' |
| EDIT | K4 | **DRIVE** | 40 | saturation |
| EDIT² | K1 | **SNAP** | 40 | how fast the sweep falls |
| EDIT² | K2 | **CLICK** | 50 | the click at the start |
| EDIT² | K3 | **HOLD** | 20 | full level held before it falls (up to 86 ms) |
| EDIT² | K4 | (free) |  |  |

### BASSO

filter bass with sub, galloping between the kicks, octave jumps and slides

| Page | Knob | Name | Factory | What it does |
|---|---|---|---|---|
| EDIT | K1 | **CUTOFF** | 52 | the low-pass's cutoff (18 dB) |
| EDIT | K2 | **RESO** | 40 | resonance |
| EDIT | K3 | **ENVMOD** | 60 | how far the envelope opens the filter |
| EDIT | K4 | **DECAY** | 34 | decay of level and filter |
| EDIT² | K1 | **WAVE** | 0 | saw (0) to square (127) |
| EDIT² | K2 | **SUB** | 40 | a sine an octave below |
| EDIT² | K3 | **GLIDE** | 50 | glide time on a slide |
| EDIT² | K4 | **DRIVE** | 40 | distortion before the filter |

### HATTU

hats and shakers: FM metal, noise and an 8-bit noise channel

| Page | Knob | Name | Factory | What it does |
|---|---|---|---|---|
| EDIT | K1 | **DECAY** | 40 | how long it rings |
| EDIT | K2 | **TONE** | 72 | pitch of the metal |
| EDIT | K3 | **METAL** | 70 | noise (0) to FM metal (127) |
| EDIT | K4 | **OPEN** | 64 | how far an accent opens the hat |
| EDIT² | K1 | **CHIP** | 0 | blend with the 8-bit noise channel (LFSR) |
| EDIT² | K2 | **SHAKE** | 0 | a soft attack: a shaker instead of a hat |
| EDIT² | K3 | (free) |  |  |
| EDIT² | K4 | (free) |  |  |

### HAPPO

a 303-style acid line: saw/square, 18 dB resonant filter, accent, slide

| Page | Knob | Name | Factory | What it does |
|---|---|---|---|---|
| EDIT | K1 | **CUTOFF** | 58 | cutoff: the acid line |
| EDIT | K2 | **RESO** | 96 | resonance, up to just before it whistles |
| EDIT | K3 | **ENVMOD** | 74 | filter envelope |
| EDIT | K4 | **DECAY** | 44 | decay; accents ring longer |
| EDIT² | K1 | **WAVE** | 0 | saw to square |
| EDIT² | K2 | **ACCENT** | 90 | how hard an accent kicks filter and level |
| EDIT² | K3 | **GLIDE** | 60 | glide time on a slide |
| EDIT² | K4 | **DRIVE** | 60 | distortion |

### PIIP

an 8-bit melody like a sound chip: pulse waves, arpeggios, vibrato, bend

| Page | Knob | Name | Factory | What it does |
|---|---|---|---|---|
| EDIT | K1 | **WAVE** | 30 | sound-chip wave: pulse 12/25/50 %, 4-bit triangle, saw, pitched noise |
| EDIT | K2 | **DECAY** | 46 | how long it rings |
| EDIT | K3 | **ARP** | 40 | a chord as a fast arpeggio: OFF OCT MIN MAJ SUS4 5+8 DIM |
| EDIT | K4 | **VIBRATO** | 30 | vibrato depth |
| EDIT² | K1 | **SPEED** | 64 | speed of arpeggio and vibrato |
| EDIT² | K2 | **PWM** | 30 | pulse-width modulation |
| EDIT² | K3 | **BEND** | 64 | pitch at the start: from below (−) or above (+) |
| EDIT² | K4 | **DETUNE** | 0 | a second, detuned oscillator |

### OUTO

'weird': FM zaps, lasers, bubbles, squeaks; every hit different

| Page | Knob | Name | Factory | What it does |
|---|---|---|---|---|
| EDIT | K1 | **DECAY** | 60 | how long it rings |
| EDIT | K2 | **FM** | 80 | FM index: how weird |
| EDIT | K3 | **RATIO** | 48 | the operators' frequency ratio |
| EDIT | K4 | **ZAP** | 100 | laser: pitch falls from above (+) or rises from below (−) |
| EDIT² | K1 | **ZTIME** | 50 | length of the zap |
| EDIT² | K2 | **FEEDBK** | 40 | modulator feedback: growl |
| EDIT² | K3 | **WOBBLE** | 0 | pitch LFO: bubbling |
| EDIT² | K4 | **RANDOM** | 50 | each hit a little different |

### Factory mix

| Track | Level | Pan | Delay | Reverb |
|---|---|---|---|---|
| **POTKU** | 122 | C | 0 | 0 |
| **BASSO** | 108 | C | 0 | 0 |
| **HATTU** | 84 | R12 | 14 | 14 |
| **HAPPO** | 84 | L12 | 60 | 20 |
| **PIIP** | 76 | R16 | 70 | 40 |
| **OUTO** | 80 | C | 60 | 50 |
<!-- /GEN:sound -->

## The sequencer

- **Polymetre:** each track has its own loop length (ALGORITHM). HAPPO on 14 against the kick's 16 repeats only after
  seven bars.
- **Chance** per step, **ratchets** of up to four hits, **accents**.
- **Slides** on BASSO, HAPPO and PIIP: the sound glides to the new note without a new attack. The glide time is GLIDE
  (fixed on PIIP). A slide is only heard while the previous note still sounds.
- **Parameter locks:** on EDIT, hold a white key and turn K1 to K4: the value holds for that step alone. The generator
  gives OUTO its own locks on every step, which is why each zap sounds different.
- **EUCLID** (SEQ): hits spread as evenly as the loop allows, with rotation, chance and an accent on every n-th hit.
- **LINK** (SEQ ×2): the track plays depending on another: WITH only when the source hits, GAPS only in its gaps,
  AFTER a step later. A muted source still counts.
- **TIMING** (SEQ ×3): SELECT walks from hit to hit, K1 shifts the hit, K2 the whole track.

## SEKO

SEKO on the HOME page is the suomisaundi knob. At 0 the sequencer plays exactly what is programmed. The further up, the
more often a step plays **something else**: a scale degree up or down, an octave off, with a slide or as a double hit;
hats double or swap their accent. Filters and decays wander slowly too, and the level sways a little. The kick always
stays where it is.

## 8-bit

ENV twice opens the chosen track's **8-BIT** page:

- **BITS** rounds the signal to fewer and fewer levels (OFF, then coarser up to 127).
- **RATE** holds each value for several samples: the sample rate drops, the sound turns metallic and full of aliasing.

On top of that, PIIP (pulse waves, 4-bit triangle, pitched noise) and HATTU's CHIP knob (a shift register like a sound
chip's) already sound 8-bit by themselves. The idea comes from memetek-fm1.

## Effects and sidechain

- **ENV**: level, pan and delay and reverb sends per track.
- **FX**: delay time (1/16, 1/8, 3/16; the right side two thirds of it), feedback, reverb size, damping.
- **FX ×2** (SIDECHAIN): source (POTKU from the factory), depth, release and how much the chosen track is ducked. A
  muted source still ducks (ghost kick).
- **GLO ×2**: DRIVE saturates the sum, XPOSE transposes the pattern.

## Patterns, chain and key

Four patterns, A to D. **PRESETS** switches them, while playing at the next bar. **CHAIN** (GLO) plays A-B, A-C or
A-D in turn, four bars each. From the factory C and D are transposed by +3 and −2 semitones (XPOSE), so the chain
changes key. Hold SAVE and turn PRESETS to copy the whole pattern, hold SAVE and turn SELECT to copy the track alone;
letting go copies, UNDO in the destination takes it back.

## Global values

<!-- GEN:global -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| Name | Page | Factory | Range | What it does |
|---|---|---|---|---|
| **TEMPO** | HOME | 145 | 120 … 180 | tempo in BPM |
| **SWING** | HOME | 50 | 50 … 75 | swing in % |
| **SEKO** | HOME | 40 | 0 … 127 | chaos: steps play differently now and then, sounds wander |
| **DUCK** | HOME / FX² | 100 | 0 … 127 | sidechain ducking depth |
| **ROOT** | GLO | F#1 | 24 … 47 | root note (MIDI note) |
| **SCALE** | GLO | PHRDOM | 0 … 127 | scale (see below) |
| **CLOCK** | GLO | ON | 0 … 1 | send MIDI clock |
| **DRIVE** | GLO² | 30 | 0 … 127 | saturation of the sum |
| **D.TIME** | FX | 3/16 | 0 … 2 | delay time 1/16, 1/8, 3/16 |
| **D.FDBK** | FX | 60 | 0 … 127 | delay feedback |
| **R.SIZE** | FX | 60 | 0 … 127 | reverb size |
| **R.DAMP** | FX | 60 | 0 … 127 | reverb damping |
| **RELEASE** | FX² | 50 | 0 … 127 | how fast the ducking lets go |
<!-- /GEN:global -->

## Scales

The generator, SEKO and the tracks' notes follow the scale on GLO.

<!-- GEN:scales -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| Scale | What it does |
|---|---|
| **MINOR** | natural minor |
| **PHRYG** | phrygian: dark |
| **PHRDOM** | phrygian dominant: the psy scale (factory) |
| **HARMIN** | harmonic minor |
| **DORIAN** | dorian |
| **MAJOR** | major: the happy side of suomisaundi |
| **PENTA** | minor pentatonic |
| **WHOLE** | whole tone: weird |
<!-- /GEN:scales -->

## MIDI

- **In:** channels 1 to 6 play the tracks (note 60 = the step's base note). Channel 10: 36 POTKU, 37–39 OUTO, 42 and
  44 HATTU, 46 HATTU open. MIDI start and stop start and stop the groove.
- **Out:** MIDI clock, start and stop over USB when CLOCK on GLO is on.

## Building the firmware

```
git clone --recursive https://github.com/g0rst/suom1saundi
cd suom1saundi
sh tests/run_tests.sh        # the core's host tests
./build.sh --host            # fm1-base's simulator, then tests/run_tests.sh again
sh sim/build.sh              # the browser emulator: docs/emulator.html
./build.sh                   # the .fwsc package (needs JieLi's toolchain and the AC79 SDK)
python3 tools/manual.py      # update the manuals from the code
```

## Where it comes from

The sequencer, controls and screen come from [ro:MINImal](https://github.com/g0rst/ro-MINImal), the 8-bit ideas from
[memetek-fm1](https://github.com/g0rst/memetek-fm1), acid and filter ideas from [neur0psy](https://github.com/g0rst/neur0psy).
The platform is [fm1-base](https://github.com/charlesvestal/fm1-base) on felucca-bsp (MPL-2.0). suom1saundi itself is
MIT. Not affiliated with M-VAVE.

## Versions

<!-- GEN:version -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
Current version: **0.1**

| Version | Date | Changes |
|---|---|---|
| **0.1** | 2026-11-17 | First version: six voices, sequencer with slides and SEKO, 8-bit crusher, browser emulator, manual in three languages |
<!-- /GEN:version -->
