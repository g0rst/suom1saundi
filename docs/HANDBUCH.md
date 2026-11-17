# suom1saundi – Handbuch

suom1saundi ist eine Firmware für den **M-VAVE FM-1**, die aus dem kleinen FM-Synth eine Groovebox für
**Suomisaundi** macht: finnischen Freestyle-Psytrance. Das Genre ist hüpfend, verspielt und albern, mit rollenden
Bässen, zwitschernden Acid-Linien, Chiptune-Melodien und Geräuschen, die niemand erwartet. Genau dafür sind die sechs
Stimmen gebaut, und ein Regler namens **SEKO** (finnisch für „verrückt“) sorgt dafür, dass kein Takt genau wie der
letzte klingt.

Es gibt zwei Arten, suom1saundi zu spielen: **MINI**, der FM-1 allein, und **MAXI**, der FM-1 mit einem
**M-VAVE SMK-37** (Pro oder Elite) als Steuerzentrale mit Tasten, 16 Pads, 8 Endlos-Knobs und Fadern.

> Dieses Handbuch aktualisiert sich selbst: Die Tabellen mit Reglern, Werkswerten, Seiten, Skalen, der
> SMK-37-Belegung und den Versionen schreibt `tools/manual.py` bei jedem Push direkt aus dem Code der Firmware.

## Stand und Warnung

- Der Kern (`app/core/suomi.c`) läuft in den Host-Tests, im Simulator von fm1-base und im Browser-Emulator.
- Ein `.fwsc`-Paket für das Gerät braucht JieLis Toolchain und das AC79-SDK und wurde **noch nicht gebaut**.
- Auf einem echten FM-1 ist die Firmware **noch nicht getestet**, mit einem echten SMK-37 auch nicht. Custom-Firmware
  kann ein Gerät unbrauchbar machen. Vorher auf die offizielle V15 gehen, Daten sichern und wissen, wie man
  zurückkommt (Installer von fm1-base).
- Projekte aus Version 0.1 werden nicht geladen (das Speicherformat hat sich geändert); der FM-1 startet dann mit
  einem frischen Groove.

## Ausprobieren ohne Gerät

Der [Emulator](emulator.html) läuft im Browser mit genau demselben C-Kern wie die Firmware, als WebAssembly. Die
Bedienelemente sitzen wie am FM-1. Erst **Ton einschalten**, dann **PLAY**. Die Leertaste ist PLAY. Gespeichert wird
im Browser.

Unter dem FM-1 liegt ein **virtuelles SMK-37** mit Knobs, Fadern, Pads, Mod-Rad und zwei Oktaven Tasten (am
Computer auch A W S E D F T G Y H U J K). Es sendet die Werks-Belegung. Ein echtes SMK-37 (oder ein anderer
Controller) am Computer spielt den Emulator über **Web MIDI** (Chrome oder Edge).

## Schnellstart

1. **PLAY** drücken: ab Werk läuft ein generierter Groove in Fis phrygisch-dominant bei 145 BPM.
2. Eine der ersten sechs **schwarzen Tasten** wählt eine Spur. Die 16 **weißen Tasten** sind ihre Steps.
3. **SEL** würfelt die gewählte Spur neu, **ARP** verändert sie nur ein bisschen, die schwarze Taste **NEW** würfelt
   den ganzen Groove neu, mit neuem Akkord. **UNDO** holt den vorherigen zurück.
4. **HOME** und **KNOB3** (SEKO) aufdrehen: Der Groove fängt an zu improvisieren.
5. **EDIT** und an den vier Knobs drehen: der Klang der Spur. Nochmal **EDIT**: die anderen vier Klangregler.
   Ein drittes Mal: der LFO der Spur.
6. **ENV**: der Mixer mit allen sechs Spuren.
7. **SAVE** kurz drücken speichert alles.

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
| **SEL** | Spur neu würfeln (auf der Seite SMK-37: Belegung lernen) |
| **ARP** | Spur ein bisschen verändern (auf der Seite SMK-37: Werks-Belegung) |
| **LFO** | kurz: Spur stumm; gehalten: Solo |
| **REC** | Tasten spielen die Spur chromatisch; bei laufendem Groove wird aufgenommen |
| **SAVE** | kurz: speichern; halten + PRESETS: Pattern kopieren; halten + SELECT: Spur kopieren |
| **MASTER** | Lautstärke |

## Seiten

Die Tasten über den Knobs wählen, was **KNOB1** bis **KNOB4** tun. Jeder weitere Druck auf dieselbe Taste öffnet
ihre nächste Seite. Die Seite steht unten im Display.

<!-- GEN:pages -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
| Taste | Seite | K1 | K2 | K3 | K4 |
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

p1 bis p8 sind die acht Klangregler der gewählten Spur (siehe unten).

## Klänge

Jede Spur hat acht Klangregler: vier auf **EDIT**, vier auf **EDIT²** (EDIT zweimal drücken). Werkswerte gehen von
0 bis 127. Im MAXI-Modus liegen alle acht zugleich auf K1 bis K8 des SMK-37, und das Display zeigt alle acht.

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
| EDIT² | K4 | **BODY** | 10 | zweite Harmonische: voller, kastiger |

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
| EDIT² | K3 | **SPREAD** | 20 | jeder Schlag woanders im Stereobild |
| EDIT² | K4 | **CUT** | 50 | Hochpass: höher ist dünner |

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

### MOD: der LFO jeder Spur

**EDIT ×3** öffnet den LFO der gewählten Spur. Er läuft im Takt des Grooves und bewegt einen Klangregler, Schlag für
Schlag: eine Acid-Linie, deren Filter sich über zwei Takte öffnet, eine Kick, die jeden zweiten Takt härter wird,
Zaps, die immer weiter zerfasern.

- **RATE**: eine Schwingung pro 1/16, 1/8, 3/16, 1/4, 3/8, 1/2, 3/4, 1, 2 oder 4 Takte
- **SHAPE**: SIN, TRI, SAW (fallend), SQR, RND (jede Schwingung ein neuer Zufallswert)
- **DEST**: welcher der acht Klangregler
- **DEPTH**: 0 in der Mitte; nach rechts mehr, nach links umgekehrt

## Der Generator

**SEL** (eine Spur) und **NEW** (der ganze Groove) setzen nicht nur Schläge, sondern spielen zusammenpassende
Tonfolgen:

- Jedes Pattern hat einen **Akkord** (GLO ×2, CHORD: eine Stufe der Skala, I bis VII). BASSO, HAPPO, PIIP und OUTO
  bauen ihre Töne aus diesem Akkord und der Skala, deshalb passen sie zusammen.
- **BASSO** galoppiert auf dem Grundton des Akkords, springt in die Oktave, zur Quinte und manchmal auf den Ton
  darunter; jede dritte Linie endet auf der Quinte.
- **HAPPO** spielt ein Motiv aus acht Tönen über einer Rhythmuszelle: Pedaltöne mit Sprüngen, Akkordbrechungen,
  Läufe durch die Skala, das phrygische Quietschen. Beim zweiten Durchgang rückt die zweite Hälfte manchmal eine Terz
  höher. Große Sprünge gleiten (Slide), Oktavsprünge bekommen Akzente.
- **PIIP** spielt Frage und Antwort: acht Töne, die zum Grundton zurückkehren. Wo HAPPO spielt, lässt PIIP oft eine
  Lücke, so antworten sich die beiden.
- **OUTO** wirft seine Zaps auf Akkordtöne, eine Oktave höher oder tiefer.
- Die Patterns A bis D sind ab Werk eine **Akkordfolge** (etwa I – VI – VII – I). Mit **CHAIN** (GLO) wandert der
  Groove durch sie. **NEW** wählt für das Pattern einen neuen Akkord.
- **ARP** (Mutate) setzt geänderte Noten ebenfalls auf Akkordtöne.

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
Ausklang langsam, und die Lautstärke schwankt leicht. Die Kick bleibt immer, wo sie ist. Im MAXI-Modus liegt SEKO auf
dem Mod-Rad des SMK-37.

## Der Mixer

**ENV** öffnet den Mixer. Statt der Steps zeigt das Display dann alle sechs Spuren als Kanalzüge: Pegelanzeige,
Fader, die Anteile an Delay und Hall, das Panorama, M für stumm und S für Solo. Die Knobs gelten für die gewählte Spur
(Spur wählen wie immer: schwarze Tasten, SELECT, OCT).

- **MIX** (ENV): LEVEL, PAN, DELAY, REVERB.
- **CHANNEL** (ENV ×2): **TONE**, ein sanfter Filter pro Kanal (FLAT in der Mitte, nach links Tiefpass, nach rechts
  Hochpass), **BITS** und **RATE** (der 8-Bit-Crusher) und **DUCK**, wie stark die Spur vor der Kick zurückweicht.
  Der Kanalzug zeigt LP, HP und 8 an.
- **MASTER** (ENV ×3): **DRIVE** sättigt die Summe, **D.RET** und **R.RET** sind die Rückwege von Delay und Hall,
  **WIDTH** die Stereobreite (0 mono, 64 normal, 127 breit). Rechts am Rand zeigt das Display die Summe links und
  rechts.
- **FX** und **FX ×2** stellen Delay, Hall und Sidechain ein.
- Im MAXI-Modus sind die Fader des SMK-37 der Mixer: F1 bis F6 die Pegel der Spuren, F7 und F8 die Rückwege von
  Delay und Hall.

## 8-Bit

ENV zweimal öffnet die Seite **CHANNEL** der gewählten Spur:

- **BITS** rundet das Signal auf immer weniger Stufen (OFF, dann gröber bis 127).
- **RATE** hält jeden Wert mehrere Samples lang: die Abtastrate sinkt, der Klang wird metallisch und aliasing-reich.

Dazu klingen PIIP (Pulswellen, 4-Bit-Dreieck, gestimmtes Rauschen) und der CHIP-Regler von HATTU (ein Schieberegister
wie im Soundchip) schon von sich aus nach 8 Bit. Die Idee dafür kommt aus memetek-fm1.

## Effekte und Sidechain

- **FX**: Delay-Zeit (1/16, 1/8, 3/16; rechts zwei Drittel davon), Feedback, Hallgröße, Dämpfung.
- **FX ×2** (SIDECHAIN): Quelle (ab Werk POTKU), Tiefe, Release und wie stark die gewählte Spur geduckt wird. Eine
  stumme Quelle duckt weiter (Ghost-Kick).

## MINI und MAXI

**MINI** ist der FM-1 allein: alles geht über seine 4 Knobs, Seiten und Tasten, wie oben beschrieben.

**MAXI** ist der FM-1 mit einem M-VAVE SMK-37 (Pro oder Elite). Das SMK-37 wird zur Steuerzentrale: Seine Tasten
spielen die gewählte Spur, die Pads spielen die Drums und schalten Spuren, die 8 Knobs (mit KNOB BANK 16) liegen auf
den Klangreglern, die Fader sind der Mixer. Sobald der FM-1 eine Nachricht des SMK-37 bekommt, schaltet er auf MAXI:
Unter dem Logo erscheint ein grüner Strich, und auf den Seiten EDIT und EDIT² zeigt das Display alle acht Klangregler
zugleich; der zuletzt am SMK-37 gedrehte leuchtet.

### Verbinden

- **Kabel (empfohlen):** der 3,5-mm-MIDI-Ausgang des SMK-37 an die MIDI-Buchse des FM-1 (über die passenden
  Adapter). Dafür braucht die Firmware keinen zusätzlichen Code.
- **Über einen Computer:** SMK-37 und FM-1 per USB am Computer, MIDI vom SMK-37 zum FM-1 weiterleiten.
- Bluetooth ist absichtlich nicht eingebaut: Ein Bluetooth-Stack belegt einen großen Teil von Flash und RAM des FM-1.

### Belegung

Ab Werk erwartet der FM-1 die Belegung unten. Am einfachsten stellt man das SMK-37 in **MIDI SUITE** (M-VAVEs
Editor) genauso ein. Oder der FM-1 lernt die Belegung, die das SMK-37 gerade sendet:

1. **GLO** dreimal drücken: Seite **SMK-37**.
2. **SEL**: Das Display führt durch das Lernen. Eine Taste spielen (das C, das die Grundnote sein soll), Pad 1
   schlagen, PAD BANK einschalten und Pad 17 schlagen, dann K1 bis K16 nacheinander drehen, F1 bis F8 bewegen und
   zuletzt ◀ und ▶ drücken (für K9 bis K16 vorher KNOB BANK, für F5 bis F8 FADER BANK drücken).
3. **SEL** überspringt einen Regler, **SAVE** beendet das Lernen (der Rest bleibt frei), **HOME** bricht ab.

Endlos-Knobs, die Schritte senden statt Positionen, erkennt der FM-1 beim Lernen selbst. **ARP** auf der Seite SMK-37
stellt die Werks-Belegung wieder her. Mit K1 auf dieser Seite schaltet man MAXI aus; K2 bis K4 stellen die Kanäle von
Tasten, Pads und Reglern von Hand ein.

<!-- GEN:smk -->
<!-- (written by tools/manual.py from the code: do not edit by hand) -->
CC: Kanal 1 · Tasten unter dem Display: KNOB BANK, FADER BANK, ◀, ▶, OCTAVE−, OCTAVE+

| Bedienelement | MIDI (Werk) | Funktion auf dem FM-1 |
|---|---|---|
| **Tasten** | Kanal 1 | spielen die gewählte Spur; C4 ist ihre Grundnote; mit REC und PLAY aufnehmen |
| **K1** | CC 20 | Klang p1: TUNE / CUTOFF / DECAY / CUTOFF / WAVE / DECAY |
| **K2** | CC 21 | Klang p2: DECAY / RESO / TONE / RESO / DECAY / FM |
| **K3** | CC 22 | Klang p3: SWEEP / ENVMOD / METAL / ENVMOD / ARP / RATIO |
| **K4** | CC 23 | Klang p4: DRIVE / DECAY / OPEN / DECAY / VIBRATO / ZAP |
| **K5** | CC 24 | Klang p5: SNAP / WAVE / CHIP / WAVE / SPEED / ZTIME |
| **K6** | CC 25 | Klang p6: CLICK / SUB / SHAKE / ACCENT / PWM / FEEDBK |
| **K7** | CC 26 | Klang p7: HOLD / GLIDE / SPREAD / GLIDE / BEND / WOBBLE |
| **K8** | CC 27 | Klang p8: BODY / DRIVE / CUT / DRIVE / DETUNE / RANDOM |
| **K9** | CC 28 | LFO RATE |
| **K10** | CC 29 | LFO SHAPE |
| **K11** | CC 30 | LFO DEST |
| **K12** | CC 31 | LFO DEPTH |
| **K13** | CC 32 | TONE |
| **K14** | CC 33 | BITS |
| **K15** | CC 34 | RATE |
| **K16** | CC 35 | DUCK |
| **F1** | CC 41 | Pegel POTKU |
| **F2** | CC 42 | Pegel BASSO |
| **F3** | CC 43 | Pegel HATTU |
| **F4** | CC 44 | Pegel HAPPO |
| **F5** | CC 45 | Pegel PIIP |
| **F6** | CC 46 | Pegel OUTO |
| **F7** | CC 47 | D.RET |
| **F8** | CC 48 | R.RET |
| **Mod-Rad** | CC 1, Kanal 1 | SEKO |
| **Pitchbend** | Kanal 1 | biegt die gewählte Spur ±2 Halbtöne |
| **Pad 1** | Note 36, Kanal 10 | POTKU |
| **Pad 2** | Note 37, Kanal 10 | BASSO |
| **Pad 3** | Note 38, Kanal 10 | HATTU |
| **Pad 4** | Note 39, Kanal 10 | HAPPO |
| **Pad 5** | Note 40, Kanal 10 | PIIP |
| **Pad 6** | Note 41, Kanal 10 | OUTO |
| **Pad 7** | Note 42, Kanal 10 | NEW |
| **Pad 8** | Note 43, Kanal 10 | MUTATE |
| **Pad 9** | Note 44, Kanal 10 | MUTE POTKU |
| **Pad 10** | Note 45, Kanal 10 | MUTE BASSO |
| **Pad 11** | Note 46, Kanal 10 | MUTE HATTU |
| **Pad 12** | Note 47, Kanal 10 | MUTE HAPPO |
| **Pad 13** | Note 48, Kanal 10 | MUTE PIIP |
| **Pad 14** | Note 49, Kanal 10 | MUTE OUTO |
| **Pad 15** | Note 50, Kanal 10 | NEW GROOVE |
| **Pad 16** | Note 51, Kanal 10 | UNDO |
| **PAD BANK** (KNOB BANK + FADER BANK) | Note 52–67, Kanal 10 | Pad 17–32: schalten die Steps der gewählten Spur an und aus (kräftig: mit Akzent) |
| **◀ ▶** | CC 102, 103 | Pattern zurück / vor (beim Spielen zum nächsten Takt) |
| **KNOB BANK** | – | Knobs 9–16 |
| **FADER BANK** | – | Fader 5–8 |
| **OCTAVE− / OCTAVE+** | – | verschieben die Tasten um Oktaven; das macht das SMK-37 selbst, der FM-1 hört nur andere Noten |
<!-- /GEN:smk -->

K1 bis K4 schreiben Parameter-Locks, solange am FM-1 eine weiße Taste gehalten wird. Die Pads 1 bis 6 klingen sofort
mit Anschlag; mit **REC** und laufendem Groove werden sie, wie die Tasten, in den Step geschrieben, auf den sie fallen.

### Die sechs Tasten unter dem Display

- **KNOB BANK** schaltet die Knobs auf K9 bis K16 (LFO und Kanal), **FADER BANK** die Fader auf F5 bis F8.
- **PAD BANK:** KNOB BANK und FADER BANK zusammen drücken. Die Pads werden zu Pad 17 bis 32, und die sind am FM-1 die
  **16 Steps der gewählten Spur**: ein Schlag schaltet einen Step an oder aus, ein kräftiger Schlag setzt ihn mit
  Akzent. Liegt der Step hinter dem Ende des Loops, wird der Loop bis dorthin verlängert. So programmiert man vom
  SMK-37 aus wie an einer Drum-Machine: mit den Pads 1 bis 6 (PAD BANK aus) die Spur wählen, mit PAD BANK die Steps
  setzen.
- **◀ und ▶** wechseln das Pattern (A bis D), beim Spielen zum nächsten Takt. In MIDI SUITE auf CC 102 und 103
  stellen, oder lernen lassen.
- **OCTAVE− und OCTAVE+** verschieben die Tasten um Oktaven. Das erledigt das SMK-37 selbst; der FM-1 bekommt einfach
  höhere oder tiefere Noten, auf der gewählten Spur und beim Aufnehmen mit REC.

Im Emulator hat das virtuelle SMK-37 dieselben sechs Tasten unter einem kleinen Display, das die Bänke und die Oktave
zeigt. Mit der Maus schaltet Shift-Klick auf eine BANK-Taste die PAD BANK.

### Was das SMK-37 übernimmt

Arpeggiator, Akkord-Modus, Skalen-Korrektur und Note Repeat hat das SMK-37 selbst. suom1saundi baut sie deshalb nicht
nach: Was das SMK-37 daraus spielt, kommt als Noten an, klingt auf der gewählten Spur und wird mit REC aufgenommen.

## Patterns, Chain und Tonart

Vier Patterns A bis D. **PRESETS** wechselt sie, beim Spielen zum nächsten Takt. **CHAIN** (GLO) spielt A-B, A-C oder
A-D reihum, je vier Takte. Jedes Pattern hat seinen Akkord (GLO ×2, CHORD) und kann mit **XPOSE** transponiert werden.
SAVE halten und PRESETS drehen kopiert das ganze Pattern, SAVE halten und SELECT drehen nur die Spur; loslassen
kopiert, UNDO im Ziel macht es rückgängig.

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
| **DRIVE** | ENV³ | 30 | 0 … 127 | Sättigung der Summe |
| **D.RET** | ENV³ | 0 | 0 … 127 | Rückweg des Delays in die Summe |
| **R.RET** | ENV³ | 100 | 0 … 127 | Rückweg des Halls |
| **WIDTH** | ENV³ | 64 | 0 … 127 | Stereobreite: 0 mono, 64 normal, 127 breit |
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

- **In** (TRS-Buchse und USB): Mit MAXI an spielen die Noten auf dem Tasten-Kanal (ab Werk 1) die gewählte Spur und
  die Pad-Noten (ab Werk Kanal 10, 36 bis 51) die Pads; CCs und Pitchbend wie in der Belegung oben. Ohne MAXI und auf
  den übrigen Kanälen: Kanäle 1 bis 6 spielen die Spuren (Note 60 = Grundnote des Steps), Kanal 10: 36 POTKU, 37–39
  OUTO, 42 und 44 HATTU, 46 HATTU offen. MIDI-Start und -Stopp starten und stoppen den Groove.
- **Out:** MIDI-Clock, Start und Stopp über USB, wenn CLOCK auf GLO an ist.

## Platz auf dem Chip

Der FM-1 hat wenig Flash und RAM. suom1saundi geht deshalb sparsam damit um:

- Nur das, was in der Audio-Schleife läuft (Stimmen, Sequencer, Effekte), wird auf Geschwindigkeit übersetzt. Alles
  andere (Generator, Display, Bedienung, Speichern) wird auf Größe übersetzt. Der Code ist so trotz Mixer, LFOs und
  MAXI kleiner als in Version 0.1.
- Ein Step belegt 7 statt 8 Byte; ein Projekt mit vier Patterns ist dadurch kleiner geworden.
- MAXI braucht keinen eigenen Treiber: Das SMK-37 spricht über die MIDI-Buchse, die der FM-1 ohnehin hat, und bringt
  Arpeggiator, Akkorde und Skalen selbst mit.

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
Aktuelle Version: **0.3**

| Version | Datum | Änderungen |
|---|---|---|
| **0.1** | 2026-11-17 | Erste Version: sechs Stimmen, Sequencer mit Slides und SEKO, 8-Bit-Crusher, Browser-Emulator, Handbuch in drei Sprachen |
| **0.2** | 2026-11-17 | Generator mit Akkorden und Motiven, neuer Mixer (Kanalzüge, TONE, Master), LFO pro Spur, BODY/SPREAD/CUT, MAXI-Modus mit dem SMK-37 (Belegung, Lernen, Pads, Fader), virtuelles SMK-37 und Web MIDI im Emulator, kleinerer Code |
| **0.3** | 2026-11-17 | SMK-37: die sechs Tasten unter dem Display; PAD BANK macht die Pads 17–32 zu den Steps der Spur, ◀ ▶ wechseln das Pattern, OCTAVE im Emulator; Lernen dafür erweitert; Projektformat Version 3 |
<!-- /GEN:version -->
