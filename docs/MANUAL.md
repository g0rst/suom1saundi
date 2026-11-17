# suom1saundi – Manual

suom1saundi is firmware for the **M-VAVE FM-1** that turns the little FM synth into a groovebox for **suomisaundi**:
Finnish freestyle psytrance. The genre is bouncy, playful and silly, with rolling basses, chirping acid lines, chiptune
melodies and noises nobody expects. The six voices are built for exactly that, and a knob called **SEKO** (Finnish
for "crazy") makes sure no bar sounds quite like the last.

There are two ways to play it: **MINI**, the FM-1 alone, and **MAXI**, the FM-1 with an **M-VAVE SMK-37** (Pro or
Elite) as its control centre, with keys, 16 pads, 8 endless knobs and faders.

> This manual updates itself: the tables of knobs, factory values, pages, scales, the SMK-37 map and versions are
> written by `tools/manual.py` straight from the firmware's code on every push.

## Status and warning

- The core (`app/core/suomi.c`) runs in the host tests, in fm1-base's simulator and in the browser emulator.
- A `.fwsc` package for the device needs JieLi's toolchain and the AC79 SDK and has **not been built yet**.
- The firmware has **not been tested on a real FM-1**, nor with a real SMK-37. Custom firmware can brick a device.
  Go to the official V15 first, back up your data and know how to get back (fm1-base's installer).
- Projects saved by version 0.1 are not loaded (the format changed); the FM-1 then starts with a fresh groove.

## Try it without the device

The [emulator](emulator.html) runs in the browser with exactly the same C core as the firmware, as WebAssembly. The
controls sit as on the FM-1. Press **Ton einschalten** (sound on) first, then **PLAY**. The space bar is PLAY. Saving
goes to the browser.

Below the FM-1 sits a **virtual SMK-37** with knobs, faders, pads, a mod wheel and two octaves of keys (also A W S E
D F T G Y H U J K on the computer). It sends the factory map. A real SMK-37 (or any controller) on the computer plays
the emulator through **Web MIDI** (Chrome or Edge).

## Quick start

1. Press **PLAY**: a generated groove in F♯ phrygian dominant at 145 BPM plays out of the box.
2. One of the first six **black keys** picks a track. The 16 **white keys** are its steps.
3. **SEL** rolls the track anew, **ARP** changes it just a little, the black key **NEW** rolls the whole groove with a
   new chord, **UNDO** brings the previous one back.
4. **HOME** and turn up **KNOB3** (SEKO): the groove starts to improvise.
5. **EDIT** and turn the four knobs: the track's sound. **EDIT** again: the other four sound knobs. A third time: the
   track's LFO.
6. **ENV**: the mixer with all six tracks.
7. A short press on **SAVE** stores everything.

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
| **SEL** | roll the track anew (on the SMK-37 page: learn its map) |
| **ARP** | change the track a little (on the SMK-37 page: the factory map) |
| **LFO** | tap: mute the track; hold: solo |
| **REC** | the keys play the track chromatically; recorded while the groove runs |
| **SAVE** | tap: save; hold + PRESETS: copy the pattern; hold + SELECT: copy the track |
| **MASTER** | volume |

## Pages

The buttons above the knobs choose what **KNOB1** to **KNOB4** do. Each further press on the same button opens its
next page. The page's name is shown low on the screen.

<!-- GEN:pages -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| Button | Page | K1 | K2 | K3 | K4 |
|---|---|---|---|---|---|
| **HOME** | GROOVE | TEMPO | SWING | SEKO | DUCK |
| **EDIT** | SOUND | p1 | p2 | p3 | p4 |
| **EDIT ×2** | SOUND 2 | p5 | p6 | p7 | p8 |
| **EDIT ×3** | MOD | RATE | SHAPE | DEST | DEPTH |
| **ENV** | MIX | LEVEL | PAN | DELAY | REVERB |
| **ENV ×2** | CHANNEL | TONE | BITS | RATE | DUCK |
| **ENV ×3** | MASTER | DRIVE | D.RET | R.RET | WIDTH |
| **FX** | FX | D.TIME | D.FDBK | R.SIZE | R.DAMP |
| **FX ×2** | SIDECHAIN | SOURCE | DEPTH | RELEASE | AMOUNT |
| **GLO** | GLOBAL | ROOT | SCALE | CLOCK | CHAIN |
| **GLO ×2** | GLOBAL 2 | XPOSE | CHORD |  |  |
| **GLO ×3** | SMK-37 | MAXI | KEYS CH | PADS CH | CC CH |
| **SEQ** | EUCLID | HITS | ROTATE | CHANCE | ACCENT |
| **SEQ ×2** | LINK | SOURCE | MODE |  |  |
| **SEQ ×3** | TIMING | STEP n | TRACK |  |  |
<!-- /GEN:pages -->

p1 to p8 are the chosen track's eight sound knobs (below).

## Sounds

Each track has eight sound knobs: four on **EDIT**, four on **EDIT²** (press EDIT twice). Factory values run from 0 to
127. In MAXI all eight sit on the SMK-37's K1 to K8 at once, and the screen shows all eight.

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
| EDIT² | K4 | **BODY** | 10 | the second harmonic: fuller, boxier |

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
| EDIT² | K3 | **SPREAD** | 20 | each hit somewhere else in the stereo field |
| EDIT² | K4 | **CUT** | 50 | high-pass: higher is thinner |

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

### MOD: each track's LFO

**EDIT ×3** opens the chosen track's LFO. It runs in time with the groove and moves one sound knob, hit by hit: an
acid line whose filter opens over two bars, a kick that hits harder every other bar, zaps that fray further and
further.

- **RATE**: one cycle per 1/16, 1/8, 3/16, 1/4, 3/8, 1/2, 3/4, 1, 2 or 4 bars
- **SHAPE**: SIN, TRI, SAW (falling), SQR, RND (a new random value each cycle)
- **DEST**: which of the eight sound knobs
- **DEPTH**: 0 in the middle; right for more, left inverted

## The generator

**SEL** (one track) and **NEW** (the whole groove) don't just place hits, they write note lines that fit together:

- Each pattern has a **chord** (GLO ×2, CHORD: a degree of the scale, I to VII). BASSO, HAPPO, PIIP and OUTO build
  their notes from that chord and the scale, so they agree.
- **BASSO** gallops on the chord's root, jumps to the octave, the fifth and sometimes the note below; one line in three
  ends on the fifth.
- **HAPPO** plays a motif of eight notes over a rhythm cell: pedal notes with jumps, broken chords, runs through the
  scale, the phrygian squeak. The second time round its second half sometimes moves up a third. Big jumps slide, octave
  jumps get accents.
- **PIIP** plays question and answer: eight notes that come home to the root. Where HAPPO plays, PIIP often leaves a
  gap, so the two answer each other.
- **OUTO** throws its zaps onto chord tones, an octave up or down.
- From the factory, patterns A to D are a **chord progression** (such as I – VI – VII – I). With **CHAIN** (GLO) the
  groove walks through it. **NEW** picks a new chord for the pattern.
- **ARP** (mutate) also puts the notes it changes on chord tones.

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
stays where it is. In MAXI, SEKO sits on the SMK-37's mod wheel.

## The mixer

**ENV** opens the mixer. Instead of the steps the screen then shows all six tracks as channel strips: a level meter,
the fader, the delay and reverb sends, the pan, M for muted and S for solo. The knobs work on the chosen track (pick it
as ever: black keys, SELECT, OCT).

- **MIX** (ENV): LEVEL, PAN, DELAY, REVERB.
- **CHANNEL** (ENV ×2): **TONE**, a gentle filter per channel (FLAT in the middle, left a low-pass, right a high-pass),
  **BITS** and **RATE** (the 8-bit crusher) and **DUCK**, how far the track gives way to the kick. The strips show LP,
  HP and 8.
- **MASTER** (ENV ×3): **DRIVE** saturates the sum, **D.RET** and **R.RET** are the delay's and reverb's returns,
  **WIDTH** the stereo width (0 mono, 64 as is, 127 wide). At the right edge the screen shows the sum, left and right.
- **FX** and **FX ×2** set the delay, the reverb and the sidechain.
- In MAXI the SMK-37's faders are the mixer: F1 to F6 the tracks' levels, F7 and F8 the delay's and reverb's returns.

## 8-bit

ENV twice opens the chosen track's **CHANNEL** page:

- **BITS** rounds the signal to fewer and fewer levels (OFF, then coarser up to 127).
- **RATE** holds each value for several samples: the sample rate drops, the sound turns metallic and full of aliasing.

On top of that, PIIP (pulse waves, 4-bit triangle, pitched noise) and HATTU's CHIP knob (a shift register like a sound
chip's) already sound 8-bit by themselves. The idea comes from memetek-fm1.

## Effects and sidechain

- **FX**: delay time (1/16, 1/8, 3/16; the right side two thirds of it), feedback, reverb size, damping.
- **FX ×2** (SIDECHAIN): source (POTKU from the factory), depth, release and how much the chosen track is ducked. A
  muted source still ducks (ghost kick).

## MINI and MAXI

**MINI** is the FM-1 alone: everything goes through its 4 knobs, pages and keys, as above.

**MAXI** is the FM-1 with an M-VAVE SMK-37 (Pro or Elite). The SMK-37 becomes the control centre: its keys play the
chosen track, the pads play the drums and switch tracks, the 8 knobs (16 with KNOB BANK) sit on the sound knobs, the
faders are the mixer. As soon as the FM-1 gets a message from the SMK-37 it switches to MAXI: a green line appears
under the logo, and on the EDIT and EDIT² pages the screen shows all eight sound knobs at once; the one last turned on
the SMK-37 is lit.

### Connecting

- **Cable (recommended):** the SMK-37's 3.5 mm MIDI out into the FM-1's MIDI jack (with the right adapters). The
  firmware needs no extra code for it.
- **Through a computer:** SMK-37 and FM-1 on the computer by USB, MIDI routed from the SMK-37 to the FM-1.
- Bluetooth is left out on purpose: a Bluetooth stack takes a large part of the FM-1's flash and RAM.

### The map

From the factory the FM-1 expects the map below. The easiest way is to set the SMK-37 the same in **MIDI SUITE**
(M-VAVE's editor). Or the FM-1 learns the map the SMK-37 sends now:

1. Press **GLO** three times: the **SMK-37** page.
2. **SEL**: the screen guides you. Play a key (the C that should be the base note), hit pad 1, switch PAD BANK on and
   hit pad 17, then turn K1 to K16 one after the other, move F1 to F8 and press ◀ and ▶ last (for K9 to K16 press
   KNOB BANK first, for F5 to F8 FADER BANK).
3. **SEL** skips a control, **SAVE** ends the learning (the rest stays unassigned), **HOME** cancels.

Endless knobs that send steps instead of positions are recognised while learning. **ARP** on the SMK-37 page brings
back the factory map. K1 on that page turns MAXI off; K2 to K4 set the channels of keys, pads and knobs by hand.

<!-- GEN:smk -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
CC: channel 1 · buttons under the display: KNOB BANK, FADER BANK, ◀, ▶, OCTAVE−, OCTAVE+

| Control | MIDI (factory) | What it does on the FM-1 |
|---|---|---|
| **keys** | channel 1 | play the chosen track; C4 is its base note; REC while playing records |
| **K1** | CC 20 | sound p1: TUNE / CUTOFF / DECAY / CUTOFF / WAVE / DECAY |
| **K2** | CC 21 | sound p2: DECAY / RESO / TONE / RESO / DECAY / FM |
| **K3** | CC 22 | sound p3: SWEEP / ENVMOD / METAL / ENVMOD / ARP / RATIO |
| **K4** | CC 23 | sound p4: DRIVE / DECAY / OPEN / DECAY / VIBRATO / ZAP |
| **K5** | CC 24 | sound p5: SNAP / WAVE / CHIP / WAVE / SPEED / ZTIME |
| **K6** | CC 25 | sound p6: CLICK / SUB / SHAKE / ACCENT / PWM / FEEDBK |
| **K7** | CC 26 | sound p7: HOLD / GLIDE / SPREAD / GLIDE / BEND / WOBBLE |
| **K8** | CC 27 | sound p8: BODY / DRIVE / CUT / DRIVE / DETUNE / RANDOM |
| **K9** | CC 28 | LFO RATE |
| **K10** | CC 29 | LFO SHAPE |
| **K11** | CC 30 | LFO DEST |
| **K12** | CC 31 | LFO DEPTH |
| **K13** | CC 32 | TONE |
| **K14** | CC 33 | BITS |
| **K15** | CC 34 | RATE |
| **K16** | CC 35 | DUCK |
| **F1** | CC 41 | level POTKU |
| **F2** | CC 42 | level BASSO |
| **F3** | CC 43 | level HATTU |
| **F4** | CC 44 | level HAPPO |
| **F5** | CC 45 | level PIIP |
| **F6** | CC 46 | level OUTO |
| **F7** | CC 47 | D.RET |
| **F8** | CC 48 | R.RET |
| **mod wheel** | CC 1, channel 1 | SEKO |
| **pitch bend** | channel 1 | bends the chosen track ±2 semitones |
| **Pad 1** | note 36, channel 10 | POTKU |
| **Pad 2** | note 37, channel 10 | BASSO |
| **Pad 3** | note 38, channel 10 | HATTU |
| **Pad 4** | note 39, channel 10 | HAPPO |
| **Pad 5** | note 40, channel 10 | PIIP |
| **Pad 6** | note 41, channel 10 | OUTO |
| **Pad 7** | note 42, channel 10 | NEW |
| **Pad 8** | note 43, channel 10 | MUTATE |
| **Pad 9** | note 44, channel 10 | MUTE POTKU |
| **Pad 10** | note 45, channel 10 | MUTE BASSO |
| **Pad 11** | note 46, channel 10 | MUTE HATTU |
| **Pad 12** | note 47, channel 10 | MUTE HAPPO |
| **Pad 13** | note 48, channel 10 | MUTE PIIP |
| **Pad 14** | note 49, channel 10 | MUTE OUTO |
| **Pad 15** | note 50, channel 10 | NEW GROOVE |
| **Pad 16** | note 51, channel 10 | UNDO |
| **PAD BANK** (KNOB BANK + FADER BANK) | note 52–67, channel 10 | Pad 17–32: switch the chosen track's steps on and off (hard: with an accent) |
| **◀ ▶** | CC 102, 103 | pattern back / on (while playing, at the next bar) |
| **KNOB BANK** | – | knobs 9–16 |
| **FADER BANK** | – | faders 5–8 |
| **OCTAVE− / OCTAVE+** | – | move the keys by octaves; the SMK-37 does that itself, the FM-1 just hears other notes |
<!-- /GEN:smk -->

K1 to K4 write parameter locks while a white key is held on the FM-1. Pads 1 to 6 sound at once, with velocity; with
**REC** and the groove running they are written, like the keys, into the step they fall on.

### The six buttons under the display

- **KNOB BANK** switches the knobs to K9 to K16 (LFO and channel), **FADER BANK** the faders to F5 to F8.
- **PAD BANK:** press KNOB BANK and FADER BANK together. The pads become pads 17 to 32, and on the FM-1 those are the
  **chosen track's 16 steps**: a hit switches a step on or off, a hard hit sets it with an accent. A step past the
  loop's end makes the loop that long. So you program from the SMK-37 as on a drum machine: pick the track with pads 1
  to 6 (PAD BANK off), set its steps with PAD BANK.
- **◀ and ▶** switch the pattern (A to D), while playing at the next bar. Set them to CC 102 and 103 in MIDI SUITE, or
  let the FM-1 learn them.
- **OCTAVE− and OCTAVE+** move the keys by octaves. The SMK-37 does that itself; the FM-1 simply gets higher or lower
  notes, on the chosen track and when recording with REC.

In the emulator the virtual SMK-37 has the same six buttons under a small display showing the banks and the octave.
With a mouse, shift-click on a BANK button toggles PAD BANK.

### What the SMK-37 takes over

The SMK-37 has its own arpeggiator, chord mode, scale correction and note repeat. suom1saundi does not build them
again: what the SMK-37 plays with them arrives as notes, sounds on the chosen track and is recorded with REC.

## Patterns, chain and key

Four patterns, A to D. **PRESETS** switches them, while playing at the next bar. **CHAIN** (GLO) plays A-B, A-C or
A-D in turn, four bars each. Each pattern has its chord (GLO ×2, CHORD) and can be transposed with **XPOSE**. Hold SAVE
and turn PRESETS to copy the whole pattern, hold SAVE and turn SELECT to copy the track alone; letting go copies, UNDO
in the destination takes it back.

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
| **DRIVE** | ENV³ | 30 | 0 … 127 | saturation of the sum |
| **D.RET** | ENV³ | 0 | 0 … 127 | the delay's return into the sum |
| **R.RET** | ENV³ | 100 | 0 … 127 | the reverb's return |
| **WIDTH** | ENV³ | 64 | 0 … 127 | stereo width: 0 mono, 64 as is, 127 wide |
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

- **In** (TRS jack and USB): with MAXI on, notes on the keys' channel (1 from the factory) play the chosen track and
  the pad notes (channel 10, 36 to 51 from the factory) are the pads; CCs and pitch bend as in the map above. Without
  MAXI and on the other channels: channels 1 to 6 play the tracks (note 60 = the step's base note), channel 10: 36
  POTKU, 37–39 OUTO, 42 and 44 HATTU, 46 HATTU open. MIDI start and stop start and stop the groove.
- **Out:** MIDI clock, start and stop over USB when CLOCK on GLO is on.

## Room on the chip

The FM-1 has little flash and RAM, so suom1saundi is frugal with them:

- Only what runs in the audio loop (voices, sequencer, effects) is compiled for speed. Everything else (generator,
  screen, controls, saving) is compiled for size. With the mixer, the LFOs and MAXI added, the code is still smaller
  than version 0.1's.
- A step takes 7 bytes instead of 8, so a project of four patterns got smaller.
- MAXI needs no driver of its own: the SMK-37 talks through the MIDI jack the FM-1 has anyway, and brings its own
  arpeggiator, chords and scales.

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
Current version: **0.3**

| Version | Date | Changes |
|---|---|---|
| **0.1** | 2026-11-17 | First version: six voices, sequencer with slides and SEKO, 8-bit crusher, browser emulator, manual in three languages |
| **0.2** | 2026-11-17 | Generator with chords and motifs, new mixer (channel strips, TONE, master), an LFO per track, BODY/SPREAD/CUT, MAXI mode with the SMK-37 (map, learning, pads, faders), virtual SMK-37 and Web MIDI in the emulator, smaller code |
| **0.3** | 2026-11-17 | SMK-37: the six buttons under the display; PAD BANK makes pads 17–32 the track's steps, ◀ ▶ switch the pattern, OCTAVE in the emulator; learning extended for them; project format version 3 |
<!-- /GEN:version -->
