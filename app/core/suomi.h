/* SPDX-License-Identifier: MIT
 * suom1saundi -- a suomisaundi (Finnish freeform psytrance) groove synthesizer for the M-VAVE FM-1.
 *
 * The core is portable C99 with no libc and no platform calls: the same file runs on the FM-1 (app/app.c, on
 * fm1-base), in the browser emulator (sim/, WebAssembly) and in the host tests (tests/).
 *
 * Threads: ss_render and ss_midi_note run in the audio context; everything else in the main loop.
 */
#ifndef SS_CORE_H
#define SS_CORE_H
#include <stdint.h>

#define SS_SR 44100
#define SS_TRACKS 6
#define SS_STEPS 16
#define SS_PATTERNS 4
#define SS_W 240
#define SS_H 240
#define SS_STRIP_H 48                 /* the screen is redrawn in 5 strips of 48 rows */
#define SS_STRIPS (SS_H / SS_STRIP_H)
#define SS_NKEYS 27                   /* F3 .. G5 */

/* the tracks, by their Finnish names: kick, bass, hat, acid, beep (the sound chip), weird (the FM zap) */
enum { SS_POTKU, SS_BASSO, SS_HATTU, SS_HAPPO, SS_PIIP, SS_OUTO };

/* panel controls, in fm1-base's order (plat.h); app.c asserts they match */
enum { SS_B_FX, SS_B_SEL, SS_B_ENV, SS_B_LFO, SS_B_EDIT, SS_B_GLO, SS_B_HOME, SS_B_SAVE,
       SS_B_ARP, SS_B_SEQ, SS_B_PLAY, SS_B_REC, SS_B_OCTDN, SS_B_OCTUP, SS_NB };
enum { SS_E_SELECT, SS_E_ALGO, SS_E_PRESET, SS_E_K1, SS_E_K2, SS_E_K3, SS_E_K4, SS_NE };

enum { SS_PG_HOME, SS_PG_SOUND, SS_PG_MIX, SS_PG_FX, SS_PG_GLOBAL, SS_PG_EUCLID, SS_PG_SC, SS_PG_LINK, SS_PG_TIMING,
       SS_PG_SOUND2, SS_PG_CHAN, SS_PG_GLOBAL2, SS_PG_MOD, SS_PG_MASTER, SS_PG_SMK, SS_NPAGES };

/* step flags */
#define SS_ST_ON 0x80u
#define SS_ST_ACC 0x40u
#define SS_ST_SLIDE 0x08u               /* glide to this note, no new attack (BASSO, HAPPO, PIIP) */
#define SS_ST_PROB(f) (((f) >> 4) & 3u)   /* 0: 100 %, 1: 75 %, 2: 50 %, 3: 25 % */
#define SS_ST_RATCH(f) (((f) & 3u) + 1u)  /* 1..4 hits within the step */

typedef struct {
    uint8_t flags;
    int8_t note;                        /* semitones from the track's base note */
    int8_t shift;                       /* microtiming, -8..8: sixteenths of a step early or late */
    uint8_t lock[4];                    /* a sound parameter for this step alone: 0 none, else value + 1 */
} ss_step_t;                            /* 7 bytes: 16 x 6 x 4 of them are most of the project */

typedef struct {
    ss_step_t step[SS_STEPS];
    uint8_t len;                        /* 1..16: each track loops on its own length (polymetre) */
    uint8_t mute;
    uint8_t p[8];                       /* the sound's parameters, 0..127: pages SOUND and SOUND 2 */
    uint8_t level, pan, dly, rev;       /* 0..127 */
    uint8_t bits, rate;                 /* the 8-bit part: 0 off .. 127 fewest bits, lowest sample rate */
    uint8_t tone;                       /* the channel's filter: 64 flat, below a low-pass, above a high-pass */
    uint8_t lfo_rate, lfo_shape;        /* the MOD page: 0..9 (1/16 .. 4 bars), 0..4 (SIN TRI SAW SQR RND) */
    uint8_t lfo_dest, lfo_depth;        /* the sound parameter it moves at each hit (0..7), 64: none, below: inverted */
    uint8_t eu_hits, eu_rot;            /* the EUCLID page: hits spread over the loop, rotated */
    uint8_t eu_prob;                    /* 0..3: 25, 50, 75, 100 % for all of them */
    uint8_t eu_acc;                     /* 0: none; n: every n-th hit accented */
    uint8_t sc;                         /* 0..127: how much the sidechain ducks this track */
    uint8_t dep_src;                    /* the LINK page: 0 none, else track + 1 whose hits gate this track's steps */
    int8_t tshift;                      /* microtiming of the whole track, -8..8, added to each step's shift */
    uint8_t dep_mode;                   /* 0 WITH: only on its hits; 1 GAPS: only where it is silent; 2 AFTER: a step later */
} ss_track_t;

typedef struct {
    ss_track_t tr[SS_TRACKS];
    int8_t xpose;                       /* -12..12 semitones: with CHAIN, the key changes */
    uint8_t chord;                      /* 0..6: the chord's root as a degree of the scale; the generator builds on it */
    uint8_t rsv[2];
} ss_pattern_t;

/* MAXI: an M-VAVE SMK-37 (Pro or Elite) plays and turns the FM-1 over MIDI (the TRS jack, USB). What each of its
 * controls sends is set in its MIDI SUITE; the FM-1 takes the factory map below, or learns another (GLO x3, SEL) */
#define SS_SMK_KNOBS 16                 /* K1..K8 and, with KNOB BANK, K9..K16 */
#define SS_SMK_FADERS 8                 /* F1..F4 and, with FADER BANK, F5..F8 */
#define SS_SMK_PADS 16
typedef struct {
    uint8_t on;                         /* 1: MAXI */
    uint8_t key_ch, key_base;           /* the keys: channel 0..15, the note that plays a track's base note */
    uint8_t pad_ch, pad_note;           /* the pads: channel, the first pad's note (16 notes on from it) */
    uint8_t cc_ch;                      /* knobs and faders: channel */
    uint8_t knob_cc[SS_SMK_KNOBS];      /* 255: none */
    uint8_t fader_cc[SS_SMK_FADERS];
    uint16_t knob_rel;                  /* bit k: knob k sends steps, not positions */
    uint8_t rel_kind;                   /* steps as 1 / 127 (0), or as 65 / 63 (1) */
    uint8_t rsv;
} ss_smk_t;

typedef struct {
    uint32_t magic;                     /* SS_MAGIC */
    uint16_t version, size;
    uint8_t bpm;                        /* 120..180 */
    uint8_t swing;                      /* 50..75 % */
    uint8_t seko;                       /* 0..127: how often a step plays as another, how far the sounds wander */
    uint8_t duck;                       /* 0..127: the sidechain's depth */
    uint8_t root;                       /* MIDI note of the bass's root, 24..47 */
    uint8_t scale;                      /* 0..7 */
    uint8_t dly_time;                   /* 0: 1/16, 1: 1/8, 2: 3/16 */
    uint8_t dly_fb, rev_size, rev_damp; /* 0..127 */
    uint8_t clock_out;                  /* MIDI clock and start / stop out */
    uint8_t cur_pat;
    uint8_t chain;                      /* 0: off; 1..3: patterns A-B, A-C, A-D in turn, four bars each */
    uint8_t sc_src;                     /* the track whose hits duck the others (muted, it still does) */
    uint8_t sc_rel;                     /* 0..127: how fast the ducking lets go */
    uint8_t drive;                      /* 0..127: into the soft clipper on the sum */
    uint8_t dly_ret, rev_ret;           /* the MASTER page: the effects' returns, 0..127 */
    uint8_t width;                      /* stereo width: 0 mono, 64 as it is, 127 wide */
    uint8_t rsv;
    ss_smk_t smk;
    ss_pattern_t pat[SS_PATTERNS];
    uint32_t sum;                       /* over everything before it */
} ss_project_t;

#define SS_MAGIC 0x314F5553u          /* "SUO1" */
#define SS_VERSION 2u

/* ---- life cycle ---- */
void ss_init(uint32_t seed);          /* tables, defaults and a first groove generated from seed */
void ss_panic(void);                  /* silence now */

/* ---- audio context ---- */
/* n stereo frames of signed 24-bit samples (Q23 in int32), at gain_q12 (0..4096) */
void ss_render(int32_t *lr, uint32_t n, uint32_t gain_q12);
/* a MIDI note (channel 0..15). MAXI on: the SMK-37's keys play the chosen track, its pads are pads. Otherwise, and on
 * the other channels: channels 1..6 play the tracks (note 60 = the step's base note), channel 10 a drum map */
void ss_midi_note(uint32_t channel, uint32_t note, uint32_t velocity);
void ss_midi_transport(int start);    /* MIDI start / stop in */
void ss_midi_cc(uint32_t channel, uint32_t cc, uint32_t value);   /* channel 0..15 */
void ss_midi_bend(uint32_t channel, int value);   /* -8192..8191 */
int ss_midi_rt_pop(void);             /* a real-time byte to send (0xF8, 0xFA, 0xFC), or -1 */

/* ---- main loop ---- */
/* the panel as it is now: held buttons (bit SS_B_*), held keys (bit k), detents since the last call */
void ss_input(uint32_t buttons, uint32_t keys, const int32_t enc[SS_NE], uint32_t now_ms);
uint32_t ss_dirty(void);              /* bit s: strip s must be drawn again (cleared by the call) */
void ss_draw(uint16_t *px, int strip);   /* strip's 240 x 48 pixels, RGB565 */
void ss_lights(uint32_t *buttons, uint32_t *keys, uint32_t *dim_buttons, uint32_t *dim_keys);
void ss_message(const char *a, const char *b);

/* ---- the project, for the platform's storage ---- */
uint32_t ss_project_get(void *dst, uint32_t max);       /* bytes written (0: too small) */
int ss_project_set(const void *src, uint32_t len);      /* 0 = loaded; else nothing changed */
int ss_save_wanted(void);             /* 1 once after SAVE: the platform writes the project */

/* ---- for tests and the simulator ---- */
int ss_state(int what);
enum { SS_Q_PLAYING, SS_Q_TRACK, SS_Q_PAGE, SS_Q_BPM, SS_Q_GSTEP, SS_Q_PATTERN, SS_Q_KEYS_MODE,
       SS_Q_LEN, SS_Q_STEPS_ON, SS_Q_MUTE, SS_Q_UNSAVED, SS_Q_POS, SS_Q_SOLO, SS_Q_CURSOR, SS_Q_MAXI, SS_Q_LEARN,
       SS_Q_HITS = 100 };            /* + track: hits sounded since PLAY */
ss_project_t *ss_project(void);

#endif
