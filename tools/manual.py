#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""suom1saundi's manuals, kept up to date from the code.

    python3 tools/manual.py           rewrite the generated parts of the three manuals and the HTML pages
    python3 tools/manual.py --check   only check: exit 1 if anything is out of date (the workflow runs this)

The manuals are docs/HANDBUCH.md (Deutsch), docs/MANUAL.md (English) and docs/TLHINGAN.md (tlhIngan Hol). Their prose is
written by hand. Everything between <!-- GEN:name --> and <!-- /GEN:name --> is written by this script from
app/core/suomi.c (tracks, knob names, factory values, ranges, scales, pages), VERSION and docs/CHANGES.tsv.
Then each manual becomes an HTML page (docs/handbuch.html, manual.html, tlhingan.html) in suom1saundi's own trashy
style, and docs/index.html links them with the emulator. Standard library only.
"""
import html
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CORE = ROOT / "app" / "core" / "suomi.c"
DOCS = ROOT / "docs"
LANGS = {  # code: (markdown, html page, html lang, title)
    "de": ("HANDBUCH.md", "handbuch.html", "de", "suom1saundi – Handbuch"),
    "en": ("MANUAL.md", "manual.html", "en", "suom1saundi – Manual"),
    "tlh": ("TLHINGAN.md", "tlhingan.html", "tlh", "suom1saundi – lo'meH paq"),
}

# ------------------------------------------------------------------------------------------------- the code ---
def c_strings(src, name):
    """the string list of a C array initialiser, by the array's name (0 for a NULL entry)"""
    m = re.search(r"\b" + re.escape(name) + r"\s*\[[^\]]*\](?:\s*\[[^\]]*\])?\s*=\s*\{(.*?)\};", src, re.S)
    if not m:
        raise SystemExit(f"manual: {name} not found in {CORE.name}")
    return [None if t == "0" else t.strip('"') for t in re.findall(r'"[^"]*"|\b0\b', m.group(1))]


def read_core():
    src = CORE.read_text(encoding="utf-8")
    tracks = c_strings(src, "TRACK_NAME")
    names = c_strings(src, "SOUND_NAME")
    sound = [names[i * 8:(i + 1) * 8] for i in range(len(tracks))]
    m = re.search(r"DEF\[SS_TRACKS\]\[14\]\s*=\s*\{(.*?)\};", src, re.S)
    rows = re.findall(r"\{([\d,\s]+)\}", m.group(1))
    defs = [[int(v) for v in r.split(",")] for r in rows]
    glob = {k: int(v) for k, v in re.findall(r"R\.prj\.(\w+)\s*=\s*(\d+);", src)}
    lim = {k: (int(a), int(b)) for k, a, b in re.findall(r"LIM\(p->(\w+),\s*(-?\d+),\s*(-?\d+)\)", src)}
    return {
        "tracks": tracks, "sound": sound, "defs": defs, "glob": glob, "lim": lim,
        "pages": c_strings(src, "PAGE_NAME"), "scales": c_strings(src, "SCALE"),
        "waves": c_strings(src, "WAVE"), "arps": c_strings(src, "ARP"),
    }


# ------------------------------------------------------------------------------------- what each knob does ---
# (de, en, tlh) for each track's eight knobs, by the knob's name on the screen. Add new knobs here in all three.
DESC = {
    "POTKU": {
        "TUNE": ("Grundton der Kick", "the kick's pitch", "POTKU wab Dung bIng"),
        "DECAY": ("Ausklang", "how long it rings", "wab tIq"),
        "SWEEP": ("wie tief die Tonhöhe herunterfällt: der Psy-„Tock“", "how far the pitch falls: the psy 'tok'", "wab pum: psy 'tok'"),
        "DRIVE": ("Sättigung", "saturation", "HoS 'ej jach"),
        "SNAP": ("wie schnell der Sweep fällt", "how fast the sweep falls", "pum nom"),
        "CLICK": ("Klick am Anfang", "the click at the start", "wab tagh 'och"),
        "HOLD": ("volle Lautstärke halten, bevor sie fällt (bis 86 ms)", "full level held before it falls (up to 86 ms)", "HoS naQ yI'uch (86 ms)"),
        "BODY": ("zweite Harmonische: voller, kastiger", "the second harmonic: fuller, boxier", "wab 'ugh law'"),
    },
    "BASSO": {
        "CUTOFF": ("Grenzfrequenz des Tiefpasses (18 dB)", "the low-pass's cutoff (18 dB)", "tlhegh Hab: Dung bIng"),
        "RESO": ("Resonanz", "resonance", "jach"),
        "ENVMOD": ("wie weit die Hüllkurve den Filter öffnet", "how far the envelope opens the filter", "tlhegh poSmoH"),
        "DECAY": ("Ausklang von Lautstärke und Filter", "decay of level and filter", "wab tIq"),
        "WAVE": ("Säge (0) bis Rechteck (127)", "saw (0) to square (127)", "wab jIr: 0 .. 127"),
        "SUB": ("Sinus eine Oktave tiefer", "a sine an octave below", "wab bIng"),
        "GLIDE": ("Gleitzeit bei Slide", "glide time on a slide", "SLIDE nom"),
        "DRIVE": ("Verzerrung vor dem Filter", "distortion before the filter", "HoS 'ej jach"),
    },
    "HATTU": {
        "DECAY": ("Ausklang", "how long it rings", "wab tIq"),
        "TONE": ("Tonhöhe des Metalls", "pitch of the metal", "baS wab Dung bIng"),
        "METAL": ("Rauschen (0) bis FM-Metall (127)", "noise (0) to FM metal (127)", "wab veQ .. baS"),
        "OPEN": ("wie weit ein Akzent die Hat öffnet", "how far an accent opens the hat", "ACC poSmoH"),
        "CHIP": ("Mischung mit dem 8-Bit-Rauschkanal (LFSR)", "blend with the 8-bit noise channel (LFSR)", "8-BIT veQ wab"),
        "SHAKE": ("weicher Anstieg: Shaker statt Hat", "a soft attack: a shaker instead of a hat", "QIt tagh"),
        "SPREAD": ("jeder Schlag woanders im Stereobild", "each hit somewhere else in the stereo field", "poS nIH pIm"),
        "CUT": ("Hochpass: höher ist dünner", "high-pass: higher is thinner", "wab Dung neH"),
    },
    "HAPPO": {
        "CUTOFF": ("Grenzfrequenz: die Acid-Linie", "cutoff: the acid line", "tlhegh Hab: Dung bIng"),
        "RESO": ("Resonanz, bis kurz vor dem Pfeifen", "resonance, up to just before it whistles", "jach"),
        "ENVMOD": ("Filterhüllkurve", "filter envelope", "tlhegh poSmoH"),
        "DECAY": ("Ausklang; Akzente klingen länger", "decay; accents ring longer", "wab tIq"),
        "WAVE": ("Säge bis Rechteck", "saw to square", "wab jIr"),
        "ACCENT": ("wie stark ein Akzent Filter und Lautstärke hochreißt", "how hard an accent kicks filter and level", "ACC HoS"),
        "GLIDE": ("Gleitzeit bei Slide", "glide time on a slide", "SLIDE nom"),
        "DRIVE": ("Verzerrung", "distortion", "HoS 'ej jach"),
    },
    "PIIP": {
        "WAVE": ("Soundchip-Welle: Puls 12/25/50 %, 4-Bit-Dreieck, Säge, gestimmtes Rauschen", "sound-chip wave: pulse 12/25/50 %, 4-bit triangle, saw, pitched noise", "8-BIT wab jIr"),
        "DECAY": ("Ausklang", "how long it rings", "wab tIq"),
        "ARP": ("Akkord als schnelles Arpeggio: OFF OCT MIN MAJ SUS4 5+8 DIM", "a chord as a fast arpeggio: OFF OCT MIN MAJ SUS4 5+8 DIM", "wab law' nom"),
        "VIBRATO": ("Vibrato-Tiefe", "vibrato depth", "wab ghIr"),
        "SPEED": ("Tempo von Arpeggio und Vibrato", "speed of arpeggio and vibrato", "nom"),
        "PWM": ("Pulsbreiten-Modulation", "pulse-width modulation", "PWM"),
        "BEND": ("Tonhöhe beim Anschlag: von unten (−) oder oben (+)", "pitch at the start: from below (−) or above (+)", "tagh: bIng (−) Dung (+)"),
        "DETUNE": ("zweiter, verstimmter Oszillator", "a second, detuned oscillator", "cha' wab"),
    },
    "OUTO": {
        "DECAY": ("Ausklang", "how long it rings", "wab tIq"),
        "FM": ("FM-Index: wie schräg", "FM index: how weird", "FM HoS"),
        "RATIO": ("Frequenzverhältnis der Operatoren", "the operators' frequency ratio", "FM mI'"),
        "ZAP": ("Laser: Tonhöhe fällt von oben (+) oder steigt von unten (−)", "laser: pitch falls from above (+) or rises from below (−)", "pu' wab: Dung (+) bIng (−)"),
        "ZTIME": ("Dauer des Zaps", "length of the zap", "pu' tIq"),
        "FEEDBK": ("Feedback des Modulators: Knurren", "modulator feedback: growl", "ghogh jach"),
        "WOBBLE": ("Tonhöhen-LFO: Blubbern", "pitch LFO: bubbling", "wab ghIr"),
        "RANDOM": ("jeder Schlag ein bisschen anders", "each hit a little different", "Hoch wab pIm"),
    },
}
TRACK_DESC = {
    "POTKU": ("Psy-Kick: Sinus mit tiefem, schnellem Pitch-Sweep, Klick, Soft-Clipper", "psy kick: a sine with a deep, fast pitch sweep, click, soft clipper", "POTKU: psy wab 'ugh"),
    "BASSO": ("Filterbass mit Sub, galoppierend zwischen den Kicks, Oktavsprünge und Slides", "filter bass with sub, galloping between the kicks, octave jumps and slides", "BASSO: wab bIng"),
    "HATTU": ("Hats und Shaker: FM-Metall, Rauschen und ein 8-Bit-Rauschkanal", "hats and shakers: FM metal, noise and an 8-bit noise channel", "HATTU: baS wab"),
    "HAPPO": ("Acid-Linie im Stil der 303: Säge/Rechteck, 18-dB-Resonanzfilter, Akzent, Slide", "a 303-style acid line: saw/square, 18 dB resonant filter, accent, slide", "HAPPO: acid wab"),
    "PIIP": ("8-Bit-Melodie wie ein Soundchip: Pulswellen, Arpeggios, Vibrato, Bend", "an 8-bit melody like a sound chip: pulse waves, arpeggios, vibrato, bend", "PIIP: 8-BIT QoQ"),
    "OUTO": ("„seltsam“: FM-Zaps, Laser, Blubbern, Quietschen; jeder Schlag anders", "'weird': FM zaps, lasers, bubbles, squeaks; every hit different", "OUTO: wab Huj"),
}
GLOBAL = [  # (field, screen name, page, de, en, tlh)
    ("bpm", "TEMPO", "HOME", "Tempo in BPM", "tempo in BPM", "nom (BPM)"),
    ("swing", "SWING", "HOME", "Swing in %", "swing in %", "SWING"),
    ("seko", "SEKO", "HOME", "Zufall: Steps spielen mal anders, Klänge wandern", "chaos: steps play differently now and then, sounds wander", "Hoch pIm"),
    ("duck", "DUCK", "HOME / FX²", "Tiefe des Sidechain-Duckings", "sidechain ducking depth", "SIDECHAIN HoS"),
    ("root", "ROOT", "GLO", "Grundton (MIDI-Note)", "root note (MIDI note)", "wab wa'DIch"),
    ("scale", "SCALE", "GLO", "Skala (siehe unten)", "scale (see below)", "wab ghom"),
    ("clock_out", "CLOCK", "GLO", "MIDI-Clock senden", "send MIDI clock", "MIDI CLOCK"),
    ("drive", "DRIVE", "ENV³", "Sättigung der Summe", "saturation of the sum", "Hoch HoS"),
    ("dly_ret", "D.RET", "ENV³", "Rückweg des Delays in die Summe", "the delay's return into the sum", "DELAY chegh"),
    ("rev_ret", "R.RET", "ENV³", "Rückweg des Halls", "the reverb's return", "REVERB chegh"),
    ("width", "WIDTH", "ENV³", "Stereobreite: 0 mono, 64 normal, 127 breit", "stereo width: 0 mono, 64 as is, 127 wide", "poS nIH tIn"),
    ("dly_time", "D.TIME", "FX", "Delay-Zeit 1/16, 1/8, 3/16", "delay time 1/16, 1/8, 3/16", "DELAY poH"),
    ("dly_fb", "D.FDBK", "FX", "Delay-Feedback", "delay feedback", "DELAY HoS"),
    ("rev_size", "R.SIZE", "FX", "Hallgröße", "reverb size", "REVERB tIn"),
    ("rev_damp", "R.DAMP", "FX", "Hall-Dämpfung", "reverb damping", "REVERB Hab"),
    ("sc_rel", "RELEASE", "FX²", "wie schnell das Ducking loslässt", "how fast the ducking lets go", "SIDECHAIN nom"),
]
SCALE_DESC = {
    "MINOR": ("Moll", "natural minor", "QoQ 'IQ"),
    "PHRYG": ("Phrygisch: dunkel", "phrygian: dark", "QoQ Hurgh"),
    "PHRDOM": ("Phrygisch-dominant: die Psy-Skala (Werk)", "phrygian dominant: the psy scale (factory)", "psy wab ghom"),
    "HARMIN": ("Harmonisch Moll", "harmonic minor", "QoQ 'IQ"),
    "DORIAN": ("Dorisch", "dorian", "DORIAN"),
    "MAJOR": ("Dur: die fröhliche Seite des Suomisaundi", "major: the happy side of suomisaundi", "QoQ Quch"),
    "PENTA": ("Moll-Pentatonik", "minor pentatonic", "vagh wab"),
    "WHOLE": ("Ganzton: outo", "whole tone: weird", "wab Huj"),
}
H = {  # table headings
    "de": {"knob": "Regler", "name": "Name", "page": "Seite", "fac": "Werk", "range": "Bereich", "what": "Was er tut",
           "ver": "Version", "date": "Datum", "chg": "Änderungen", "scale": "Skala", "track": "Spur", "mix": "Mix ab Werk",
           "lvl": "Pegel", "pan": "Pan", "dly": "Delay", "rev": "Hall", "free": "(frei)"},
    "en": {"knob": "Knob", "name": "Name", "page": "Page", "fac": "Factory", "range": "Range", "what": "What it does",
           "ver": "Version", "date": "Date", "chg": "Changes", "scale": "Scale", "track": "Track", "mix": "Factory mix",
           "lvl": "Level", "pan": "Pan", "dly": "Delay", "rev": "Reverb", "free": "(free)"},
    "tlh": {"knob": "SeH", "name": "pong", "page": "nav", "fac": "wa'DIch", "range": "chuq", "what": "Qu'",
            "ver": "mI'", "date": "jaj", "chg": "choH", "scale": "wab ghom", "track": "tlhegh", "mix": "wab ghom wa'DIch",
            "lvl": "HoS", "pan": "poS/nIH", "dly": "DELAY", "rev": "REVERB", "free": "(pagh)"},
}
L = {"de": 0, "en": 1, "tlh": 2}


def table(head, rows):
    out = ["| " + " | ".join(head) + " |", "|" + "|".join("---" for _ in head) + "|"]
    out += ["| " + " | ".join(str(c) for c in r) + " |" for r in rows]
    return "\n".join(out)


def gen_sound(core, lang):
    h, i, out = H[lang], L[lang], []
    for t, tname in enumerate(core["tracks"]):
        out.append(f"### {tname}\n\n{TRACK_DESC[tname][i]}\n")
        rows = []
        for k, pname in enumerate(core["sound"][t]):
            page = "EDIT" if k < 4 else "EDIT²"
            knob = f"K{k % 4 + 1}"
            if not pname:
                rows.append((page, knob, h["free"], "", ""))
                continue
            d = DESC.get(tname, {}).get(pname)
            if d is None:
                raise SystemExit(f"manual: no description for {tname} {pname}: add it to DESC in tools/manual.py")
            rows.append((page, knob, f"**{pname}**", core["defs"][t][k], d[i]))
        out.append(table([h["page"], h["knob"], h["name"], h["fac"], h["what"]], rows))
        out.append("")
    rows = [(f"**{n}**", d[8], "C" if d[9] == 64 else ("L" if d[9] < 64 else "R") + str(abs(d[9] - 64)), d[10], d[11])
            for n, d in zip(core["tracks"], core["defs"])]
    out.append(f"### {h['mix']}\n")
    out.append(table([h["track"], h["lvl"], h["pan"], h["dly"], h["rev"]], rows))
    return "\n".join(out).strip()


def gen_global(core, lang):
    h, i, rows = H[lang], L[lang], []
    names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
    for f, scr, page, *d in GLOBAL:
        v = core["glob"].get(f, 0)
        lo, hi = core["lim"].get(f, (0, 127))
        if f == "root":
            v = f"{names[v % 12]}{v // 12 - 1}"
        elif f == "scale":
            v = core["scales"][v]
        elif f == "dly_time":
            v = ["1/16", "1/8", "3/16"][v]
        elif f == "clock_out":
            v = "ON" if v else "OFF"
        rows.append((f"**{scr}**", page, v, f"{lo} … {hi}", d[i]))
    return table([h["name"], h["page"], h["fac"], h["range"], h["what"]], rows)


def gen_scales(core, lang):
    h, i = H[lang], L[lang]
    return table([h["scale"], h["what"]], [(f"**{s}**", SCALE_DESC[s][i]) for s in core["scales"]])


def gen_version(lang):
    h, i = H[lang], L[lang]
    rows = []
    for line in (DOCS / "CHANGES.tsv").read_text(encoding="utf-8").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        ver, date, *txt = line.split("\t")
        rows.append((f"**{ver}**", date, txt[i] if i < len(txt) else txt[0]))
    cur = (ROOT / "VERSION").read_text(encoding="utf-8").strip()
    label = {"de": "Aktuelle Version", "en": "Current version", "tlh": "DaH mI'"}[lang]
    return f"{label}: **{cur}**\n\n" + table([h["ver"], h["date"], h["chg"]], rows)


def gen_pages(core, lang):
    h = H[lang]
    pg = core["pages"]
    rows = [("HOME", pg[0], "TEMPO", "SWING", "SEKO", "DUCK"),
            ("EDIT", pg[1], "p1", "p2", "p3", "p4"),
            ("EDIT ×2", pg[9], "p5", "p6", "p7", "p8"),
            ("EDIT ×3", pg[12], "RATE", "SHAPE", "DEST", "DEPTH"),
            ("ENV", pg[2], "LEVEL", "PAN", "DELAY", "REVERB"),
            ("ENV ×2", pg[10], "TONE", "BITS", "RATE", "DUCK"),
            ("ENV ×3", pg[13], "DRIVE", "D.RET", "R.RET", "WIDTH"),
            ("FX", pg[3], "D.TIME", "D.FDBK", "R.SIZE", "R.DAMP"),
            ("FX ×2", pg[6], "SOURCE", "DEPTH", "RELEASE", "AMOUNT"),
            ("GLO", pg[4], "ROOT", "SCALE", "CLOCK", "CHAIN"),
            ("GLO ×2", pg[11], "XPOSE", "CHORD", "", ""),
            ("GLO ×3", pg[14], "MAXI", "KEYS CH", "PADS CH", "CC CH"),
            ("SEQ", pg[5], "HITS", "ROTATE", "CHANCE", "ACCENT"),
            ("SEQ ×2", pg[7], "SOURCE", "MODE", "", ""),
            ("SEQ ×3", pg[8], "STEP n", "TRACK", "", "")]
    btn = {"de": "Taste", "en": "Button", "tlh": "leQ"}[lang]
    return table([btn, h["page"], "K1", "K2", "K3", "K4"], [(f"**{a}**", b, c, d, e, f) for a, b, c, d, e, f in rows])


def gen_smk(core, lang):
    """the SMK-37's factory map and what each control does, from smk_preset and smk_input in the code"""
    src = CORE.read_text(encoding="utf-8")
    m = re.search(r"static void smk_preset\(ss_smk_t \*m\)\s*\{(.*?)\n\}", src, re.S).group(1)
    val = {k: int(v) for k, v in re.findall(r"m->(\w+) = (\d+);", m)}
    kb = int(re.search(r"knob_cc\[i\] = \(uint8_t\)\((\d+) \+ i\)", m).group(1))
    fb = int(re.search(r"fader_cc\[i\] = \(uint8_t\)\((\d+) \+ i\)", m).group(1))
    nm = c_strings(src, "NM")
    i = L[lang]
    T = {"ctl": ("Bedienelement", "Control", "SeH"), "msg": ("MIDI (Werk)", "MIDI (factory)", "MIDI"),
         "does": ("Funktion auf dem FM-1", "What it does on the FM-1", "Qu'"),
         "keys": ("Tasten", "keys", "leQmey"), "pads": ("Pads", "pads", "PADS"),
         "ch": ("Kanal", "channel", "CH"), "note": ("Note", "note", "NOTE"),
         "play": ("spielen die gewählte Spur; C4 ist ihre Grundnote; mit REC und PLAY aufnehmen",
                  "play the chosen track; C4 is its base note; REC while playing records", "tlhegh QoQ"),
         "snd": ("Klang p", "sound p", "wab p"),
         "lvl": ("Pegel", "level", "HoS"), "wheel": ("Mod-Rad", "mod wheel", "MOD"),
         "bend": ("Pitchbend", "pitch bend", "BEND"), "bendd": ("biegt die gewählte Spur ±2 Halbtöne", "bends the chosen track ±2 semitones", "wab ghIr"),
         "step": ("Step", "step", "Step"), "steps": ("schalten die Steps der gewählten Spur an und aus (kräftig: mit Akzent)",
                                                     "switch the chosen track's steps on and off (hard: with an accent)", "Steps chu' pagh"),
         "pat": ("Pattern zurück / vor (beim Spielen zum nächsten Takt)", "pattern back / on (while playing, at the next bar)", "nav ret / nav veb"),
         "oct": ("verschieben die Tasten um Oktaven; das macht das SMK-37 selbst, der FM-1 hört nur andere Noten",
                 "move the keys by octaves; the SMK-37 does that itself, the FM-1 just hears other notes", "leQmey Dung bIng"),
         "kb": ("Knobs 9–16", "knobs 9–16", "K9–K16"), "fb": ("Fader 5–8", "faders 5–8", "F5–F8"),
         "under": ("Tasten unter dem Display", "buttons under the display", "jIH bIng leQmey")}
    t = lambda k: T[k][i]
    trk = core["tracks"]
    rows = [(f"**{t('keys')}**", f"{t('ch')} {val['key_ch'] + 1}", t("play"))]
    for k in range(8):
        rows.append((f"**K{k + 1}**", f"CC {kb + k}", f"{t('snd')}{k + 1}: " + " / ".join(x[k] or "–" for x in core["sound"])))
    for k in range(8):
        rows.append((f"**K{k + 9}**", f"CC {kb + 8 + k}", nm[k]))
    for k in range(8):
        rows.append((f"**F{k + 1}**", f"CC {fb + k}", f"{t('lvl')} {trk[k]}" if k < len(trk) else ("D.RET" if k == 6 else "R.RET")))
    rows.append((f"**{t('wheel')}**", f"CC 1, {t('ch')} {val['key_ch'] + 1}", "SEKO"))
    rows.append((f"**{t('bend')}**", f"{t('ch')} {val['key_ch'] + 1}", t("bendd")))
    pads = [f"{trk[p]}" for p in range(6)] + ["NEW", "MUTATE"] + [f"MUTE {trk[p]}" for p in range(6)] + ["NEW GROOVE", "UNDO"]
    for p, what in enumerate(pads):
        rows.append((f"**Pad {p + 1}**", f"{t('note')} {val['pad_note'] + p}, {t('ch')} {val['pad_ch'] + 1}", what))
    rows.append((f"**PAD BANK** (KNOB BANK + FADER BANK)", f"{t('note')} {val['pad2_note']}–{val['pad2_note'] + 15}, {t('ch')} {val['pad_ch'] + 1}",
                 f"Pad 17–32: {t('steps')}"))
    rows.append(("**◀ ▶**", f"CC {val['left_cc']}, {val['right_cc']}", t("pat")))
    rows.append(("**KNOB BANK**", "–", t("kb")))
    rows.append(("**FADER BANK**", "–", t("fb")))
    rows.append(("**OCTAVE− / OCTAVE+**", "–", t("oct")))
    knobs_ch = f"CC: {t('ch')} {val['cc_ch'] + 1} · {t('under')}: KNOB BANK, FADER BANK, ◀, ▶, OCTAVE−, OCTAVE+"
    return f"{knobs_ch}\n\n" + table([t("ctl"), t("msg"), t("does")], rows)


GEN = {"smk": gen_smk, "sound": gen_sound, "global": gen_global, "scales": gen_scales, "pages": gen_pages}


def fill(md, core, lang):
    def sub(m):
        name = m.group(1)
        body = gen_version(lang) if name == "version" else GEN[name](core, lang)
        return f"<!-- GEN:{name} -->\n<!-- (written by tools/manual.py from the code: do not edit by hand) -->\n{body}\n<!-- /GEN:{name} -->"
    return re.sub(r"<!-- GEN:(\w+) -->.*?<!-- /GEN:\1 -->", sub, md, flags=re.S)


# ------------------------------------------------------------------------------------------ markdown -> HTML ---
def inline(t):
    t = html.escape(t, quote=False)
    t = re.sub(r"`([^`]+)`", r"<code>\1</code>", t)
    t = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", t)
    t = re.sub(r"(?<![*\w])\*([^*]+)\*(?!\w)", r"<i>\1</i>", t)
    t = re.sub(r"\[([^\]]+)\]\(([^)\s]+)\)", lambda m: f'<a href="{m.group(2)}">{m.group(1)}</a>', t)
    return t


def slug(t):
    return re.sub(r"[^a-z0-9]+", "-", t.lower()).strip("-") or "x"


def md_html(md):
    out, toc, lines, i = [], [], md.splitlines(), 0
    while i < len(lines):
        ln = lines[i]
        if ln.startswith("<!--"):
            while "-->" not in lines[i]:
                i += 1
            i += 1
            continue
        if not ln.strip():
            i += 1
            continue
        m = re.match(r"(#{1,4}) (.*)", ln)
        if m:
            n, txt = len(m.group(1)), m.group(2)
            sid = slug(txt)
            if n == 2:
                toc.append((sid, txt))
            out.append(f'<h{n} id="{sid}">{inline(txt)}</h{n}>')
            i += 1
            continue
        if ln.startswith("|"):
            rows = []
            while i < len(lines) and lines[i].startswith("|"):
                rows.append([c.strip() for c in lines[i].strip().strip("|").split("|")])
                i += 1
            body = "".join("<tr>" + "".join(f"<td>{inline(c)}</td>" for c in r) + "</tr>" for r in rows[2:])
            out.append('<div class="tw"><table><thead><tr>' + "".join(f"<th>{inline(c)}</th>" for c in rows[0]) +
                       f"</tr></thead><tbody>{body}</tbody></table></div>")
            continue
        if re.match(r"\s*([-*]|\d+\.) ", ln):
            tag = "ol" if re.match(r"\s*\d+\.", ln) else "ul"
            items = []
            while i < len(lines) and (re.match(r"\s*([-*]|\d+\.) ", lines[i]) or (lines[i].startswith("  ") and items)):
                if re.match(r"\s*([-*]|\d+\.) ", lines[i]):
                    items.append(re.sub(r"\s*([-*]|\d+\.) ", "", lines[i], count=1))
                else:
                    items[-1] += " " + lines[i].strip()
                i += 1
            out.append(f"<{tag}>" + "".join(f"<li>{inline(x)}</li>" for x in items) + f"</{tag}>")
            continue
        if ln.startswith(">"):
            q = []
            while i < len(lines) and lines[i].startswith(">"):
                q.append(lines[i][1:].strip())
                i += 1
            out.append(f'<aside class="sticker">{inline(" ".join(q))}</aside>')
            continue
        if ln.startswith("```"):
            i += 1
            code = []
            while i < len(lines) and not lines[i].startswith("```"):
                code.append(lines[i])
                i += 1
            i += 1
            out.append("<pre>" + html.escape("\n".join(code)) + "</pre>")
            continue
        para = []
        while i < len(lines) and lines[i].strip() and not re.match(r"(#|\||>|```|\s*([-*]|\d+\.) )", lines[i]):
            para.append(lines[i].strip())
            i += 1
        out.append(f"<p>{inline(' '.join(para))}</p>")
    return "\n".join(out), toc


CSS = r"""
:root{--hot:#ff1fd1;--lime:#b6ff00;--cyan:#00f0ff;--yel:#fff200;--ink:#12001a;--paper:#fffdf2}
*{box-sizing:border-box}
html{background:var(--ink)}
body{margin:0;color:var(--ink);font:17px/1.55 "Comic Sans MS","Comic Neue","Chalkboard SE","Marker Felt",cursive;
 background:
  repeating-conic-gradient(from 45deg,#2b0040 0 25%,#12001a 0 50%) 0 0/48px 48px fixed}
a{color:#c800ff;font-weight:700}
a:hover{background:var(--yel);color:#000}
.ticker{position:sticky;top:0;z-index:9;overflow:hidden;white-space:nowrap;background:var(--yel);color:#000;
 border-bottom:4px dashed #000;font:700 15px/2 Bangers,Impact,"Arial Black",sans-serif;letter-spacing:.08em}
.ticker span{display:inline-block;padding-left:100%;animation:tick 26s linear infinite}
@keyframes tick{to{transform:translateX(-100%)}}
header.top{text-align:center;padding:38px 14px 18px;color:#fff}
.logo{font:900 clamp(44px,11vw,120px)/.9 Bangers,Impact,"Arial Black",sans-serif;margin:0;letter-spacing:-.02em;
 background:linear-gradient(90deg,var(--hot),var(--yel),var(--lime),var(--cyan),var(--hot));background-size:300% 100%;
 -webkit-background-clip:text;background-clip:text;color:transparent;animation:rb 6s linear infinite;
 filter:drop-shadow(5px 5px 0 #000) drop-shadow(-3px -3px 0 #ff00aa);transform:rotate(-3deg)}
@keyframes rb{to{background-position:300% 0}}
.tag{display:inline-block;margin-top:14px;background:var(--hot);color:#fff;padding:6px 14px;transform:rotate(2deg);
 border:3px solid #fff;box-shadow:6px 6px 0 var(--lime);font-weight:700}
.langs{margin:18px 0 0;display:flex;gap:10px;justify-content:center;flex-wrap:wrap}
.langs a{background:#000;color:var(--lime);border:3px solid var(--lime);padding:6px 12px;text-decoration:none;
 box-shadow:4px 4px 0 var(--hot)}
.langs a.cur{background:var(--lime);color:#000}
main{max-width:980px;margin:0 auto 60px;padding:0 12px}
.paper{background:var(--paper);border:5px solid #000;box-shadow:12px 12px 0 var(--hot),-8px -8px 0 var(--cyan);
 padding:20px clamp(14px,4vw,42px) 40px;position:relative}
.paper:before{content:"";position:absolute;inset:-5px -5px auto -5px;height:16px;
 background:repeating-linear-gradient(45deg,#000 0 14px,var(--yel) 14px 28px)}
.toc{background:#000;color:#fff;padding:12px 16px;border:3px dotted var(--cyan);transform:rotate(-.6deg);margin:26px 0}
.toc b{color:var(--yel);font-family:Bangers,Impact,sans-serif;letter-spacing:.1em}
.toc ol{margin:6px 0 0;columns:2 230px}
.toc a{color:var(--cyan)}
h1{font:900 42px/1.1 Bangers,Impact,"Arial Black",sans-serif;margin:22px 0 10px}
h2{font:900 30px/1.15 Bangers,Impact,"Arial Black",sans-serif;margin:46px 0 14px;color:#fff;background:#000;
 display:inline-block;padding:4px 14px;transform:rotate(-1.2deg);box-shadow:6px 6px 0 var(--hot)}
h2:nth-of-type(3n+2){box-shadow:6px 6px 0 var(--lime);transform:rotate(1deg)}
h2:nth-of-type(3n){box-shadow:6px 6px 0 var(--cyan)}
h2:before{content:"★ ";color:var(--yel)}
h3{font:900 22px/1.2 Bangers,Impact,"Arial Black",sans-serif;margin:28px 0 6px;color:#c800ff;text-shadow:2px 2px 0 var(--lime)}
p,li{max-width:72ch}
code{background:#000;color:var(--lime);padding:1px 6px;font:15px ui-monospace,Menlo,Consolas,monospace}
pre{background:#000;color:var(--lime);padding:14px;overflow:auto;border-left:8px solid var(--hot);font:14px/1.45 ui-monospace,Menlo,monospace}
.tw{overflow-x:auto;margin:10px 0 18px}
table{border-collapse:collapse;min-width:100%;font-size:15.5px;background:#fff}
th{background:#000;color:var(--yel);font:700 15px Bangers,Impact,"Arial Black",sans-serif;letter-spacing:.05em;text-align:left;padding:7px 10px}
td{border:2px solid #000;padding:6px 10px;vertical-align:top}
tbody tr:nth-child(odd){background:#fff3fd}
tbody tr:hover{background:var(--yel)}
.sticker{display:block;background:var(--lime);border:3px solid #000;padding:12px 16px;margin:18px 0;max-width:62ch;
 transform:rotate(-1.4deg);box-shadow:7px 7px 0 #000;font-weight:700}
.sticker:nth-of-type(even){background:var(--cyan);transform:rotate(1.2deg)}
.blink{animation:bl 1s steps(1) infinite;color:var(--hot)}
@keyframes bl{50%{opacity:0}}
.construct{margin:30px auto 0;text-align:center;font:700 14px Bangers,Impact,sans-serif;color:#000;letter-spacing:.12em;
 background:repeating-linear-gradient(-45deg,var(--yel) 0 18px,#000 18px 36px);padding:12px}
.construct span{background:var(--yel);padding:4px 10px}
footer{color:#fff;text-align:center;font-size:14px;padding:0 12px 40px}
footer .counter{display:inline-block;background:#000;color:var(--lime);font:700 18px ui-monospace,monospace;
 border:2px inset #777;padding:2px 8px;letter-spacing:.2em}
.emu{display:inline-block;margin:22px 0 4px;background:var(--hot);color:#fff;font:900 24px Bangers,Impact,sans-serif;padding:12px 22px;
 border:4px solid #000;box-shadow:8px 8px 0 #000;text-decoration:none;transform:rotate(-2deg)}
.emu:hover{background:var(--lime);color:#000;transform:rotate(2deg) scale(1.05)}
@media (prefers-reduced-motion:reduce){*{animation:none!important}}
"""

TICK = {
    "de": "★ SUOMISAUNDI AUF DEINEM FM-1 ★ PSY-KICK ★ ACID ★ 8-BIT ★ SEKO = VERRÜCKT ★ DIESES HANDBUCH SCHREIBT SICH SELBST NEU ★ JETZT IM EMULATOR SPIELEN ★",
    "en": "★ SUOMISAUNDI ON YOUR FM-1 ★ PSY KICK ★ ACID ★ 8-BIT ★ SEKO = CRAZY ★ THIS MANUAL REWRITES ITSELF ★ PLAY IT NOW IN THE EMULATOR ★",
    "tlh": "★ nuqneH! ★ suom1saundi QoQ jan ★ POTKU ★ HAPPO ★ PIIP ★ SEKO: Hoch pIm ★ reH paq'vam choHlu' ★ DaH yIQoy! ★ Qapla'! ★",
}
UI = {  # (tagline, toc, emulator, under construction, footer)
    "de": ("Das Handbuch zur Suomisaundi-Firmware für den M-VAVE FM-1", "INHALT", "▶ IM EMULATOR SPIELEN",
           "IMMER IM UMBAU · WIRD BEI JEDEM PUSH AUS DEM CODE AKTUALISIERT", "Besucher seit 1997"),
    "en": ("The manual of the suomisaundi firmware for the M-VAVE FM-1", "CONTENTS", "▶ PLAY IN THE EMULATOR",
           "FOREVER UNDER CONSTRUCTION · REBUILT FROM THE CODE ON EVERY PUSH", "visitors since 1997"),
    "tlh": ("M-VAVE FM-1 QoQ jan lo'meH paq", "'ay'mey", "▶ DaH yIQoy",
            "reH choHlu' · ngoq vo' paq'vam chenmoHlu'", "SuchwI'pu' (1997 DIS)"),
}


def page(lang, body, toc, title):
    tag, toc_l, emu, cons, foot = UI[lang]
    cur = (ROOT / "VERSION").read_text(encoding="utf-8").strip()
    langs = "".join(f'<a href="{LANGS[c][1]}"{" class=cur" if c == lang else ""}>{n}</a>'
                    for c, n in (("de", "Deutsch"), ("en", "English"), ("tlh", "tlhIngan Hol")))
    tocs = "".join(f'<li><a href="#{a}">{inline(t)}</a></li>' for a, t in toc)
    return f"""<!DOCTYPE html>
<html lang="{LANGS[lang][2]}">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(title)}</title>
<!-- written by tools/manual.py from docs/{LANGS[lang][0]}: do not edit by hand -->
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Bangers&family=Comic+Neue:wght@400;700&display=swap">
<style>{CSS}</style>
</head>
<body>
<div class="ticker" aria-hidden="true"><span>{TICK[lang]}</span></div>
<header class="top">
  <p class="logo">suom1saundi</p>
  <div class="tag">{tag} · v{cur} <span class="blink">NEW!</span></div>
  <nav class="langs">{langs}<a href="index.html">⌂</a></nav>
  <a class="emu" href="emulator.html">{emu}</a>
</header>
<main>
<div class="paper">
<nav class="toc"><b>{toc_l}</b><ol>{tocs}</ol></nav>
{body}
<div class="construct"><span>🚧 {cons} 🚧</span></div>
</div>
</main>
<footer><p>{foot}: <span class="counter">0004711</span></p>
<p>MIT · <a href="https://github.com/g0rst/suom1saundi" style="color:#b6ff00">github.com/g0rst/suom1saundi</a></p></footer>
</body>
</html>
"""


def index_page():
    cur = (ROOT / "VERSION").read_text(encoding="utf-8").strip()
    return f"""<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>suom1saundi – suomisaundi für den M-VAVE FM-1</title>
<!-- written by tools/manual.py: do not edit by hand -->
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Bangers&family=Comic+Neue:wght@400;700&display=swap">
<style>{CSS}
.grid{{display:grid;gap:22px;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));margin:26px 0}}
.card{{background:#fff;border:4px solid #000;padding:16px;box-shadow:8px 8px 0 var(--lime);text-decoration:none;color:#000;display:block}}
.card:nth-child(2){{box-shadow:8px 8px 0 var(--hot);transform:rotate(1deg)}}
.card:nth-child(3){{box-shadow:8px 8px 0 var(--cyan);transform:rotate(-1deg)}}
.card b{{font:900 26px Bangers,Impact,sans-serif;display:block}}
.card:hover{{background:var(--yel)}}
</style>
</head>
<body>
<div class="ticker" aria-hidden="true"><span>{TICK["de"]} {TICK["en"]} {TICK["tlh"]}</span></div>
<header class="top">
  <p class="logo">suom1saundi</p>
  <div class="tag">suomisaundi · M-VAVE FM-1 · v{cur} <span class="blink">NEW!</span></div><br>
  <a class="emu" href="emulator.html">▶ EMULATOR / PLAY NOW / DaH yIQoy</a>
</header>
<main><div class="paper">
<h1>Finnischer Freestyle-Psy im Taschenformat</h1>
<p>Sechs Stimmen (POTKU, BASSO, HATTU, HAPPO, PIIP, OUTO), ein polymetrischer Sequencer mit Slides, Chance und <b>SEKO</b>,
ein 8-Bit-Crusher pro Spur. Der Emulator oben läuft mit genau demselben C-Kern wie die Firmware.</p>
<div class="grid">
  <a class="card" href="handbuch.html"><b>HANDBUCH</b>Deutsch</a>
  <a class="card" href="manual.html"><b>MANUAL</b>English</a>
  <a class="card" href="tlhingan.html"><b>lo'meH paq</b>tlhIngan Hol</a>
</div>
<aside class="sticker">Noch nicht auf echter Hardware getestet. Not yet tested on real hardware. QoQ jan naQ wIwaHbe'.</aside>
<div class="construct"><span>🚧 UNDER CONSTRUCTION SEIT 1997 🚧</span></div>
</div></main>
<footer><p><a href="https://github.com/g0rst/suom1saundi" style="color:#b6ff00">github.com/g0rst/suom1saundi</a> · MIT</p></footer>
</body>
</html>
"""


def main():
    check = "--check" in sys.argv[1:]
    core = read_core()
    stale = []
    want = {}
    for lang, (md_name, html_name, _, title) in LANGS.items():
        md_path = DOCS / md_name
        md = md_path.read_text(encoding="utf-8")
        new_md = fill(md, core, lang)
        want[md_path] = new_md
        body, toc = md_html(new_md)
        want[DOCS / html_name] = page(lang, body, toc, title)
    want[DOCS / "index.html"] = index_page()
    for path, text in want.items():
        old = path.read_text(encoding="utf-8") if path.exists() else None
        if old != text:
            stale.append(path.relative_to(ROOT))
            if not check:
                path.write_text(text, encoding="utf-8")
    if check and stale:
        print("manual: out of date (run python3 tools/manual.py):", *stale, sep="\n  ")
        return 1
    print("manual: " + (f"wrote {len(stale)} file(s)" if stale else "up to date"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
