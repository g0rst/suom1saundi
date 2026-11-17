# suom1saundi – lo'meH paq

nuqneH! M-VAVE FM-1 QoQ jan ngoq 'oH suom1saundi'e'. suomisaundi QoQ DaQoymeH wIghunta'. Suomi QoQ 'oH suomisaundi'e':
Quch 'ej Huj. jav ghogh ghaj suom1saundi. **SEKO** SeH DajIrmoHchugh, QoQ choH reH.
cha' mIw: **MINI** (FM-1 nIteb) 'ej **MAXI** (FM-1 SMK-37 je).

> tlhIngan Hol mughlu'pu'; DIvI' Hol pong'e' jIH 'ang, pong rap lo'lu'. *(Fan-Übersetzung: Namen der Regler bleiben, wie
> das Display sie zeigt.)* reH choHlu' paq'vam: `tools/manual.py` ngoq vo' SeHmey mI'mey je ghItlh.

## ghuHmoH

- `app/core/suomi.c` Qap: tests, fm1-base, EMULATOR je.
- `.fwsc` wIchenmoHbe'. JieLi jan 'ej AC79 SDK DapoQ.
- QoQ jan naQ wIwaHbe'! QoQ jan DaQaw'laH. V15 yIlo' wa'DIch, De'lIj yIpol.

## EMULATOR

[EMULATOR](emulator.html) yIlo': ngoq rap Qap. **Ton einschalten** yIyuv, **PLAY** yIyuv. QoQ yIQoy!

## yItagh!

1. **PLAY** yIyuv: QoQ tagh. 145 BPM, F♯ PHRDOM.
2. jav leQmey qIj: tlhegh yIwIv. wa'maH jav leQmey chIS: tlhegh Steps.
3. **SEL** yIyuv: tlhegh chu'. **ARP**: tlhegh choH. **NEW**: Hoch chu'. **UNDO**: ret.
4. **HOME** yIyuv, **KNOB3** (SEKO) yIjIrmoH: QoQ Huj.
5. **EDIT** yIyuv, SeHmey yIjIrmoH: wab choH.
6. **SAVE** yIyuv: Hoch yIpol.

## jav tlhegh

| leQ | tlhegh | Suomi Hol | Qu' |
|---|---|---|---|
| qIj 1 | **POTKU** | pup | wab 'ugh, loS |
| qIj 2 | **BASSO** | wab bIng | POTKU retlh |
| qIj 3 | **HATTU** | mIv | baS wab, 8-BIT |
| qIj 4 | **HAPPO** | acid | acid tlhegh |
| qIj 5 | **PIIP** | piip | 8-BIT QoQ |
| qIj 6 | **OUTO** | Huj | FM pu' wab |

## SeHmey

| SeH | Qu' |
|---|---|
| **PLAY** | tagh, mev |
| **chIS 16** | Steps; yIyuv: chu' pagh |
| **chIS yI'uch + K1** | wab Dung bIng |
| **chIS yI'uch + K2** | CHANCE: 100, 75, 50, 25 % |
| **chIS yI'uch + K3** | OFF, ACC, SLD, A+S |
| **chIS yI'uch + K4** | RATCHET 1 .. 4 |
| **qIj 1–6** | tlhegh yIwIv |
| **qIj 7–11** | CLR, ◀ ▶, NEW, UNDO |
| **ALGORITHM** | tlhegh tIq: 1 .. 16 |
| **PRESETS** | A, B, C, D |
| **LFO** | yIyuv: tam; yI'uch: nIteb |
| **REC** | leQmey QoQ |
| **SAVE** | yIpol |
| **MASTER** | HoS |

## navmey

SeHmey Dung leQmey: K1 .. K4 Qu' yIwIv. leQ rap cha'logh yIyuv: cha'DIch nav.

<!-- GEN:pages -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| leQ | nav | K1 | K2 | K3 | K4 |
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

## wab

Hoch tlhegh: chorgh SeH. loS **EDIT**, loS **EDIT²**. 0 .. 127.

<!-- GEN:sound -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
### POTKU

POTKU: psy wab 'ugh

| nav | SeH | pong | wa'DIch | Qu' |
|---|---|---|---|---|
| EDIT | K1 | **TUNE** | 48 | POTKU wab Dung bIng |
| EDIT | K2 | **DECAY** | 58 | wab tIq |
| EDIT | K3 | **SWEEP** | 62 | wab pum: psy 'tok' |
| EDIT | K4 | **DRIVE** | 40 | HoS 'ej jach |
| EDIT² | K1 | **SNAP** | 40 | pum nom |
| EDIT² | K2 | **CLICK** | 50 | wab tagh 'och |
| EDIT² | K3 | **HOLD** | 20 | HoS naQ yI'uch (86 ms) |
| EDIT² | K4 | **BODY** | 10 | wab 'ugh law' |

### BASSO

BASSO: wab bIng

| nav | SeH | pong | wa'DIch | Qu' |
|---|---|---|---|---|
| EDIT | K1 | **CUTOFF** | 52 | tlhegh Hab: Dung bIng |
| EDIT | K2 | **RESO** | 40 | jach |
| EDIT | K3 | **ENVMOD** | 60 | tlhegh poSmoH |
| EDIT | K4 | **DECAY** | 34 | wab tIq |
| EDIT² | K1 | **WAVE** | 0 | wab jIr: 0 .. 127 |
| EDIT² | K2 | **SUB** | 40 | wab bIng |
| EDIT² | K3 | **GLIDE** | 50 | SLIDE nom |
| EDIT² | K4 | **DRIVE** | 40 | HoS 'ej jach |

### HATTU

HATTU: baS wab

| nav | SeH | pong | wa'DIch | Qu' |
|---|---|---|---|---|
| EDIT | K1 | **DECAY** | 40 | wab tIq |
| EDIT | K2 | **TONE** | 72 | baS wab Dung bIng |
| EDIT | K3 | **METAL** | 70 | wab veQ .. baS |
| EDIT | K4 | **OPEN** | 64 | ACC poSmoH |
| EDIT² | K1 | **CHIP** | 0 | 8-BIT veQ wab |
| EDIT² | K2 | **SHAKE** | 0 | QIt tagh |
| EDIT² | K3 | **SPREAD** | 20 | poS nIH pIm |
| EDIT² | K4 | **CUT** | 50 | wab Dung neH |

### HAPPO

HAPPO: acid wab

| nav | SeH | pong | wa'DIch | Qu' |
|---|---|---|---|---|
| EDIT | K1 | **CUTOFF** | 58 | tlhegh Hab: Dung bIng |
| EDIT | K2 | **RESO** | 96 | jach |
| EDIT | K3 | **ENVMOD** | 74 | tlhegh poSmoH |
| EDIT | K4 | **DECAY** | 44 | wab tIq |
| EDIT² | K1 | **WAVE** | 0 | wab jIr |
| EDIT² | K2 | **ACCENT** | 90 | ACC HoS |
| EDIT² | K3 | **GLIDE** | 60 | SLIDE nom |
| EDIT² | K4 | **DRIVE** | 60 | HoS 'ej jach |

### PIIP

PIIP: 8-BIT QoQ

| nav | SeH | pong | wa'DIch | Qu' |
|---|---|---|---|---|
| EDIT | K1 | **WAVE** | 30 | 8-BIT wab jIr |
| EDIT | K2 | **DECAY** | 46 | wab tIq |
| EDIT | K3 | **ARP** | 40 | wab law' nom |
| EDIT | K4 | **VIBRATO** | 30 | wab ghIr |
| EDIT² | K1 | **SPEED** | 64 | nom |
| EDIT² | K2 | **PWM** | 30 | PWM |
| EDIT² | K3 | **BEND** | 64 | tagh: bIng (−) Dung (+) |
| EDIT² | K4 | **DETUNE** | 0 | cha' wab |

### OUTO

OUTO: wab Huj

| nav | SeH | pong | wa'DIch | Qu' |
|---|---|---|---|---|
| EDIT | K1 | **DECAY** | 60 | wab tIq |
| EDIT | K2 | **FM** | 80 | FM HoS |
| EDIT | K3 | **RATIO** | 48 | FM mI' |
| EDIT | K4 | **ZAP** | 100 | pu' wab: Dung (+) bIng (−) |
| EDIT² | K1 | **ZTIME** | 50 | pu' tIq |
| EDIT² | K2 | **FEEDBK** | 40 | ghogh jach |
| EDIT² | K3 | **WOBBLE** | 0 | wab ghIr |
| EDIT² | K4 | **RANDOM** | 50 | Hoch wab pIm |

### wab ghom wa'DIch

| tlhegh | HoS | poS/nIH | DELAY | REVERB |
|---|---|---|---|---|
| **POTKU** | 122 | C | 0 | 0 |
| **BASSO** | 108 | C | 0 | 0 |
| **HATTU** | 84 | R12 | 14 | 14 |
| **HAPPO** | 84 | L12 | 60 | 20 |
| **PIIP** | 76 | R16 | 70 | 40 |
| **OUTO** | 80 | C | 60 | 50 |
<!-- /GEN:sound -->

### MOD

**EDIT** wejlogh yIyuv: LFO. wab SeH vIHmoH LFO. RATE, SHAPE, DEST, DEPTH.

## QoQ chenmoHwI'

**SEL** pagh **NEW** yIyuv: QoQ chu'. CHORD ghaj Hoch nav (GLO ×2). BASSO HAPPO PIIP OUTO je: CHORD rap lo'.
Qochbe' Hoch tlhegh. A B C D: CHORD tlhegh. **CHAIN** yIchu'!

## SEKO

SEKO 0: QoQ rap reH. SEKO Dun: Steps pIm, wab ghIr, SLIDE, cha' Steps. POTKU choHbe' reH.

## wab ghom SeH

**ENV** yIyuv: Hoch tlhegh 'ang jIH. **ENV ×2**: TONE, BITS, RATE, DUCK. **ENV ×3**: DRIVE, D.RET, R.RET, WIDTH.

## MINI 'ej MAXI

- MINI: FM-1 nIteb.
- MAXI: FM-1 SMK-37 je. SMK-37 leQmey, PADS, SeHmey: FM-1 SeH.
- MIDI tlhegh yIlo' (3,5 mm). Bluetooth wIlo'be': FM-1 qawHaq mach.
- **GLO** wejlogh yIyuv, **SEL** yIyuv: ghoj FM-1. **ARP**: wa'DIch SeHmey.

<!-- GEN:smk -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
CC: CH 1

| SeH | MIDI | Qu' |
|---|---|---|
| **leQmey** | CH 1 | tlhegh QoQ |
| **K1** | CC 20 | wab p1: TUNE / CUTOFF / DECAY / CUTOFF / WAVE / DECAY |
| **K2** | CC 21 | wab p2: DECAY / RESO / TONE / RESO / DECAY / FM |
| **K3** | CC 22 | wab p3: SWEEP / ENVMOD / METAL / ENVMOD / ARP / RATIO |
| **K4** | CC 23 | wab p4: DRIVE / DECAY / OPEN / DECAY / VIBRATO / ZAP |
| **K5** | CC 24 | wab p5: SNAP / WAVE / CHIP / WAVE / SPEED / ZTIME |
| **K6** | CC 25 | wab p6: CLICK / SUB / SHAKE / ACCENT / PWM / FEEDBK |
| **K7** | CC 26 | wab p7: HOLD / GLIDE / SPREAD / GLIDE / BEND / WOBBLE |
| **K8** | CC 27 | wab p8: BODY / DRIVE / CUT / DRIVE / DETUNE / RANDOM |
| **K9** | CC 28 | LFO RATE |
| **K10** | CC 29 | LFO SHAPE |
| **K11** | CC 30 | LFO DEST |
| **K12** | CC 31 | LFO DEPTH |
| **K13** | CC 32 | TONE |
| **K14** | CC 33 | BITS |
| **K15** | CC 34 | RATE |
| **K16** | CC 35 | DUCK |
| **F1** | CC 41 | HoS POTKU |
| **F2** | CC 42 | HoS BASSO |
| **F3** | CC 43 | HoS HATTU |
| **F4** | CC 44 | HoS HAPPO |
| **F5** | CC 45 | HoS PIIP |
| **F6** | CC 46 | HoS OUTO |
| **F7** | CC 47 | D.RET |
| **F8** | CC 48 | R.RET |
| **MOD** | CC 1, CH 1 | SEKO |
| **BEND** | CH 1 | wab ghIr |
| **Pad 1** | NOTE 36, CH 10 | POTKU |
| **Pad 2** | NOTE 37, CH 10 | BASSO |
| **Pad 3** | NOTE 38, CH 10 | HATTU |
| **Pad 4** | NOTE 39, CH 10 | HAPPO |
| **Pad 5** | NOTE 40, CH 10 | PIIP |
| **Pad 6** | NOTE 41, CH 10 | OUTO |
| **Pad 7** | NOTE 42, CH 10 | NEW |
| **Pad 8** | NOTE 43, CH 10 | MUTATE |
| **Pad 9** | NOTE 44, CH 10 | MUTE POTKU |
| **Pad 10** | NOTE 45, CH 10 | MUTE BASSO |
| **Pad 11** | NOTE 46, CH 10 | MUTE HATTU |
| **Pad 12** | NOTE 47, CH 10 | MUTE HAPPO |
| **Pad 13** | NOTE 48, CH 10 | MUTE PIIP |
| **Pad 14** | NOTE 49, CH 10 | MUTE OUTO |
| **Pad 15** | NOTE 50, CH 10 | NEW GROOVE |
| **Pad 16** | NOTE 51, CH 10 | UNDO |
<!-- /GEN:smk -->

## 8-BIT

**ENV** cha'logh yIyuv: **BITS** 'ej **RATE**. wab veQ, wab Dun!

## Hoch mI'

<!-- GEN:global -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| pong | nav | wa'DIch | chuq | Qu' |
|---|---|---|---|---|
| **TEMPO** | HOME | 145 | 120 … 180 | nom (BPM) |
| **SWING** | HOME | 50 | 50 … 75 | SWING |
| **SEKO** | HOME | 40 | 0 … 127 | Hoch pIm |
| **DUCK** | HOME / FX² | 100 | 0 … 127 | SIDECHAIN HoS |
| **ROOT** | GLO | F#1 | 24 … 47 | wab wa'DIch |
| **SCALE** | GLO | PHRDOM | 0 … 127 | wab ghom |
| **CLOCK** | GLO | ON | 0 … 1 | MIDI CLOCK |
| **DRIVE** | ENV³ | 30 | 0 … 127 | Hoch HoS |
| **D.RET** | ENV³ | 0 | 0 … 127 | DELAY chegh |
| **R.RET** | ENV³ | 100 | 0 … 127 | REVERB chegh |
| **WIDTH** | ENV³ | 64 | 0 … 127 | poS nIH tIn |
| **D.TIME** | FX | 3/16 | 0 … 2 | DELAY poH |
| **D.FDBK** | FX | 60 | 0 … 127 | DELAY HoS |
| **R.SIZE** | FX | 60 | 0 … 127 | REVERB tIn |
| **R.DAMP** | FX | 60 | 0 … 127 | REVERB Hab |
| **RELEASE** | FX² | 50 | 0 … 127 | SIDECHAIN nom |
<!-- /GEN:global -->

## wab ghommey

<!-- GEN:scales -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| wab ghom | Qu' |
|---|---|
| **MINOR** | QoQ 'IQ |
| **PHRYG** | QoQ Hurgh |
| **PHRDOM** | psy wab ghom |
| **HARMIN** | QoQ 'IQ |
| **DORIAN** | DORIAN |
| **MAJOR** | QoQ Quch |
| **PENTA** | vagh wab |
| **WHOLE** | wab Huj |
<!-- /GEN:scales -->

## MIDI

- MIDI 1 .. 6: tlhegh. MIDI 10: 36 POTKU, 37–39 OUTO, 42 44 46 HATTU.
- MIDI CLOCK: GLO, CLOCK ON.

## ngoq yIchenmoH

```
git clone --recursive https://github.com/g0rst/suom1saundi
sh tests/run_tests.sh
sh sim/build.sh
./build.sh
python3 tools/manual.py
```

## mI'mey

<!-- GEN:version -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
DaH mI': **0.2**

| mI' | jaj | choH |
|---|---|---|
| **0.1** | 2026-11-17 | wa'DIch: jav ghogh, SLIDE SEKO je, 8-BIT, EMULATOR, wej Hol lo'meH paq |
| **0.2** | 2026-11-17 | CHORD chu', wab ghom SeH chu', LFO, MAXI: SMK-37, EMULATOR SMK-37, ngoq mach |
<!-- /GEN:version -->

Qapla'!
