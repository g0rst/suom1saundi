/* SPDX-License-Identifier: MIT
 * suom1saundi core: six voices for suomisaundi (a psy kick, a filter bass, hats, an acid line, a sound chip's lead
 * and an FM zap), a polymetric probability sequencer with slides, a groove generator, SEKO (the pattern played
 * a little differently each time), an 8-bit crusher a track, delay, reverb, and the whole user interface drawn into
 * 240 x 48 strips. See suomi.h. The sequencer and the interface come from ro:MINImal (github.com/g0rst/ro-MINImal). */
#include "suomi.h"

#if defined SS_FM1_POOL && !defined PLAT_HOST   /* the FM-1: big buffers go in fm1-base's .pool region */
#define POOL __attribute__((section(".pool")))
#else
#define POOL
#endif

#define INV_SR (1.0f / (float)SS_SR)
#define DLY_LEN 16384u                  /* mono delay line, int16: 0.37 s */

/* ------------------------------------------------------------------ math --- */
static float sintab[1025];

static void sin_init(void)
{
    int i;
    for (i = 0; i <= 256; i++) {        /* a quarter by Taylor series, the rest by symmetry */
        float x = (float)i * (6.2831853f / 1024.0f), x2 = x * x;
        float s = x * (1.0f - x2 / 6.0f * (1.0f - x2 / 20.0f * (1.0f - x2 / 42.0f * (1.0f - x2 / 72.0f))));
        sintab[i] = s;
        sintab[512 - i] = s;
        sintab[512 + i] = -s;
        sintab[1024 - i] = -s;
    }
}

static inline float fsin(float turns)   /* sine of a phase in turns, any value */
{
    float x;
    int i;
    turns -= (float)(int)turns;
    if (turns < 0.0f)
        turns += 1.0f;
    x = turns * 1024.0f;
    i = (int)x;
    return sintab[i] + (sintab[i + 1] - sintab[i]) * (x - (float)i);
}

static float fexp2(float x)
{
    union { uint32_t u; float f; } b;
    int i;
    float f;
    if (x < -60.0f)
        return 0.0f;
    if (x > 60.0f)
        x = 60.0f;
    i = (int)x;
    if ((float)i > x)
        i--;
    f = x - (float)i;
    b.u = (uint32_t)(i + 127) << 23;
    return b.f * (1.0f + f * (0.6931472f + f * (0.2402265f + f * (0.0555041f + f * (0.0096181f + f * 0.0013333f)))));
}

static float note_hz(float note) { return 440.0f * fexp2((note - 69.0f) * (1.0f / 12.0f)); }
/* the per-sample factor of a decay with time constant tau seconds */
static float decay_coef(float tau) { return fexp2(-1.442695f * INV_SR / tau); }
/* 0..127 on an exponential scale from lo to lo * 2^octaves */
static float pmap(float p, float lo, float octaves) { return lo * fexp2(p * (1.0f / 127.0f) * octaves); }
static inline float clip1(float x)      /* soft: linear near 0, 1 at 1.5 and beyond */
{
    if (x > 1.5f)
        return 1.0f;
    if (x < -1.5f)
        return -1.0f;
    return x - x * x * x * (4.0f / 27.0f);
}

static inline uint32_t xs32(uint32_t *s)
{
    uint32_t x = *s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *s = x;
}
static inline float frand(uint32_t *s) { return (float)(xs32(s) >> 8) * (1.0f / 16777216.0f); }   /* 0..1 */
static inline float brand(uint32_t *s) { return frand(s) * 2.0f - 1.0f; }                           /* -1..1 */

/* ----------------------------------------------------------------- state --- */
typedef struct { float gl, gr, sd, sr; } bus_t;

/* BASSO and HAPPO: one oscillator (saw to square, band-limited) into a resonant low-pass whose cutoff an envelope
 * moves; an accent opens it further, a slide glides to the note without a new attack */
typedef struct {
    float ph, sph, f, ft, gc, amp, ac, att, fenv, fc16, cut, emod, k, wave, sub, drive, norm, vel;
    float a1, a2, a3, g1, ic1, ic2, lp;
    uint32_t ctl;
} acid_t;

static struct {
    ss_project_t prj;
    /* audio */
    uint32_t rng;                       /* audio context only */
    volatile uint8_t playing, want_play, want_stop;
    volatile uint8_t pending_pat;       /* 0xFF: none */
    float clk;                          /* samples to the next step */
    float mclk;                         /* samples to the next MIDI clock */
    uint32_t gstep;                     /* steps since start */
    volatile uint8_t pos[SS_TRACKS];    /* each track's step, for the screen */
    volatile uint32_t gstep_ui;
    float drift[SS_TRACKS][2];
    struct { int32_t delay, gap; int8_t note; uint8_t on, acc, rep, si, sl; float vel; } pend[SS_TRACKS][2];
    uint8_t early[SS_TRACKS];           /* this step was already scheduled from the one before: it plays early */
    uint8_t hit[SS_TRACKS], hit_prev[SS_TRACKS], hit_early[SS_TRACKS];   /* did the track's step fire: now,
                                         * a step ago, and the early one's answer */
    volatile uint32_t hits[SS_TRACKS];
    volatile uint8_t tq[16][3];         /* triggers from the main loop: track, note + 64, velocity */
    volatile uint8_t tq_w, tq_r;
    volatile uint8_t rt[16];            /* MIDI real-time bytes out */
    volatile uint8_t rt_w, rt_r;
    volatile uint8_t flash[SS_TRACKS];  /* a trigger happened: the screen shows it */
    bus_t bus[SS_TRACKS];
    struct { float ph, amp, ac, penv, pc, f0, pd, drive, norm, vel, click_g; int32_t click, hold; } kick;
    acid_t basso, happo;
    struct { float ph, mph, ph2, mph2, f, amp, ac, metal, lp, vel, chip, cph, nz, att, atc; uint32_t lfsr; } hat;
    struct { float ph, ph2, nph, f, ft, gc, amp, ac, att, vph, vinc, vdep, pwm, bend, benv, bc, det, vel, nz, mul[3];
             int32_t arp_n, arp_i, arp_cnt, arp_len; uint32_t lfsr; uint8_t wave; } piip;
    struct { float ph, mph, f, amp, ac, att, ienv, ic, idx, ratio, zap, zenv, zc, fb, last, wph, winc, wdep, pan, vel; } outo;
    struct { float held; uint32_t n, cnt; float q; } crush[SS_TRACKS];
    float duck_env;
    uint32_t dly_w;
    float dly_lp;
    uint32_t comb_i[4], ap_i[4];
    float comb_f[4];
    /* ui */
    uint32_t ui_rng;
    uint8_t track, page, keys_mode, unsaved, save_wanted;
    int8_t octave;
    volatile int8_t solo;               /* the one track heard, -1: all */
    uint32_t lfo_t;                     /* LFO went down (0: not held, or its hold already acted) */
    uint8_t cursor;                     /* the TIMING page: the step its first knob moves */
    uint8_t eu_armed;                   /* the next turn on the EUCLID page keeps an undo first */
    uint8_t save_held, copy_kind, copy_dst;   /* SAVE held: PRESETS (1) or SELECT (2) chose a pattern to copy to */
    uint32_t btn, keys, now;
    uint8_t step_edited;                /* a held step was edited: its release does not toggle it */
    uint32_t dirty;
    uint32_t seen_gstep;
    char msg[2][20];
    uint32_t msg_until;
    ss_pattern_t undo;
    uint8_t undo_pat, undo_ok;
} R;

static int16_t dly_buf[DLY_LEN] POOL;
static const uint16_t COMB_LEN[4] = {1116, 1277, 1422, 1557};
static const uint16_t AP_LEN[4] = {556, 441, 579, 464};   /* left: 0, 1; right: 2, 3 */
static float comb_buf[4][1557] POOL;
static float ap_buf[4][579] POOL;

/* the scales: their notes within an octave, and how many */
#define SS_NSCALES 8
static const uint8_t SCALES[SS_NSCALES][7] = {
    {0, 2, 3, 5, 7, 8, 10},             /* minor */
    {0, 1, 3, 5, 7, 8, 10},             /* phrygian */
    {0, 1, 4, 5, 7, 8, 10},             /* phrygian dominant: the psytrance scale */
    {0, 2, 3, 5, 7, 8, 11},             /* harmonic minor */
    {0, 2, 3, 5, 7, 9, 10},             /* dorian */
    {0, 2, 4, 5, 7, 9, 11},             /* major: the happy side of suomisaundi */
    {0, 3, 5, 7, 10, 0, 0},             /* minor pentatonic */
    {0, 2, 4, 6, 8, 10, 0},             /* whole tone: outo */
};
static const uint8_t SCALE_N[SS_NSCALES] = {7, 7, 7, 7, 7, 7, 5, 6};
static const float OUTO_RATIO[8] = {0.5f, 1.0f, 1.5f, 2.0f, 2.76f, 3.5f, 5.33f, 7.1f};
/* PIIP's arpeggios, a note a frame as on a sound chip: none, octave, minor, major, sus4, fifth and octave, diminished */
static const int8_t PIIP_ARP[7][3] = {{0, 0, 0}, {0, 12, 0}, {0, 3, 7}, {0, 4, 7}, {0, 5, 7}, {0, 7, 12}, {0, 3, 6}};
static const uint8_t PIIP_ARP_N[7] = {1, 2, 3, 3, 3, 3, 3};

#define PAT (&R.prj.pat[R.prj.cur_pat])

/* ---------------------------------------------------------------- voices --- */
/* a parameter with its drift: d in -1..1, scaled by SEKO, up to +-20 of 127 */
static float drifted(int t, int which, uint8_t p)
{
    float v = (float)p + R.drift[t][which] * (float)R.prj.seko * (20.0f / 127.0f);
    return v < 0.0f ? 0.0f : v > 127.0f ? 127.0f : v;
}

static void bus_update(int t)
{
    const ss_track_t *tr = &PAT->tr[t];
    float lev = (float)tr->level * (1.0f / 127.0f), pan = (float)tr->pan * (1.0f / 127.0f);
    float l = 2.0f * (1.0f - pan), r = 2.0f * pan;
    lev *= lev;
    R.bus[t].gl = lev * (l > 1.0f ? 1.0f : l);
    R.bus[t].gr = lev * (r > 1.0f ? 1.0f : r);
    R.bus[t].sd = lev * (float)tr->dly * (1.0f / 127.0f);
    R.bus[t].sr = lev * (float)tr->rev * (1.0f / 127.0f);
    /* the 8-bit part: fewer bits (BITS), fewer samples (RATE) */
    R.crush[t].q = tr->bits ? pmap((float)(127 - tr->bits), 1.5f, 6.0f) : 0.0f;
    R.crush[t].n = tr->rate ? 1u + (uint32_t)tr->rate * 40u / 127u : 0u;
}

/* the filter's coefficients from its envelope: every 16 samples */
static void acid_ctl(acid_t *a)
{
    float fc = a->cut * fexp2(a->emod * a->fenv), w, g;
    if (fc > 15000.0f)
        fc = 15000.0f;
    w = fc * (0.5f * INV_SR);
    g = fsin(w) / fsin(w + 0.25f);      /* tan(pi fc / sr) */
    a->a1 = 1.0f / (1.0f + g * (g + a->k));
    a->a2 = g * a->a1;
    a->a3 = g * a->a2;
    a->g1 = g / (1.0f + g);
    a->fenv *= a->fc16;
}

static inline float blep(float t, float dt)   /* the step of a saw, band-limited */
{
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

static inline float acid_run(acid_t *a)
{
    float dt, saw, p2, x, v1, v2, v3;
    if (a->amp <= 1e-4f) {
        a->ic1 = a->ic2 = a->lp = 0.0f;
        return 0.0f;
    }
    if ((a->ctl++ & 15u) == 0u)
        acid_ctl(a);
    a->f += (a->ft - a->f) * a->gc;
    dt = a->f * INV_SR;
    a->ph += dt;
    if (a->ph >= 1.0f)
        a->ph -= 1.0f;
    saw = 2.0f * a->ph - 1.0f - blep(a->ph, dt);
    x = saw;
    if (a->wave > 0.0f) {               /* a square is a saw less the same saw half a cycle on */
        p2 = a->ph + 0.5f;
        if (p2 >= 1.0f)
            p2 -= 1.0f;
        x += (-(2.0f * p2 - 1.0f - blep(p2, dt))) * a->wave;
    }
    if (a->sub > 0.0f) {                /* a sine an octave below */
        a->sph += dt * 0.5f;
        if (a->sph >= 1.0f)
            a->sph -= 1.0f;
        x += fsin(a->sph) * a->sub;
    }
    x = clip1(x * a->drive) * a->norm;
    v3 = x - a->ic2;
    v1 = a->a1 * a->ic1 + a->a2 * v3;
    v2 = a->ic2 + a->a2 * a->ic1 + a->a3 * v3;
    a->ic1 = 2.0f * v1 - a->ic1;
    a->ic2 = 2.0f * v2 - a->ic2;
    a->lp += (v2 - a->lp) * a->g1;      /* a third pole: 18 dB an octave */
    x = clip1(a->lp * 0.8f) * a->amp * a->att * a->vel;
    a->att += (1.0f - a->att) * 0.02f;
    a->amp *= a->ac;
    return x;
}

/* p: CUTOFF RESO ENVMOD DECAY | WAVE SUB-or-ACCENT GLIDE DRIVE */
static void acid_trig(acid_t *a, int t, const uint8_t *p, float hz, float vel, int acc, int slide, int is_bass)
{
    float g = 1.0f + (float)p[7] * (5.0f / 127.0f), accent = is_bass ? 0.5f : (float)p[5] * (1.0f / 127.0f);
    float dec = pmap(drifted(t, 1, p[3]), 0.03f, 5.0f);
    a->ft = hz;
    a->cut = pmap(drifted(t, 0, p[0]), is_bass ? 45.0f : 70.0f, 6.5f);
    a->k = 2.0f - (float)p[1] * (1.93f / 127.0f);
    a->emod = (float)p[2] * (5.0f / 127.0f) + (acc ? accent * 1.6f : 0.0f);
    a->wave = (float)p[4] * (1.0f / 127.0f);
    a->sub = is_bass ? (float)p[5] * (0.9f / 127.0f) : 0.0f;
    a->drive = g;
    a->norm = 1.0f / clip1(g);
    a->vel = vel * (acc ? 1.0f + accent * 0.35f : 1.0f);
    a->ac = decay_coef(acc && !is_bass ? dec * 1.5f : dec);
    a->fc16 = decay_coef(dec * 0.6f / 16.0f);
    if (slide && a->amp > 0.02f) {      /* a slide: the note glides, the envelopes go on */
        a->gc = 1.0f - decay_coef(pmap((float)p[6], 0.004f, 5.0f));
        a->amp = a->amp * 0.5f + 0.5f;
        return;
    }
    if (a->amp < 0.05f) {
        a->att = 0.0f;
        a->ph = a->sph = 0.0f;
    }
    a->f = hz;
    a->gc = 1.0f;
    a->amp = 1.0f;
    a->fenv = 1.0f;
    a->ctl = 0;
}

static float semis(float n) { return fexp2(n * (1.0f / 12.0f)); }

/* lock: the step's own values for the first four sound parameters (0: the track's), or NULL */
static void trig(int t, int note, float vel, int acc, int slide, const uint8_t *lock)
{
    const ss_track_t *tr = &PAT->tr[t];
    uint8_t p[8];
    int li;
    float root = (float)((int)R.prj.root + PAT->xpose);
    for (li = 0; li < 8; li++)
        p[li] = li < 4 && lock && lock[li] ? (uint8_t)(lock[li] - 1u) : tr->p[li];
    bus_update(t);
    R.flash[t] = 1;
    if (t == (int)R.prj.sc_src)
        R.duck_env = 1.0f;
    switch (t) {
    case SS_POTKU: {                    /* TUNE DECAY SWEEP DRIVE | SNAP CLICK HOLD */
        float g = 1.0f + (float)p[3] * (6.0f / 127.0f);
        R.kick.f0 = pmap((float)p[0], 36.0f, 1.0f) * semis((float)note);
        R.kick.ac = decay_coef(pmap(drifted(t, 0, p[1]), 0.04f, 3.2f));
        R.kick.pd = (float)p[2] * (40.0f / 127.0f);
        R.kick.pc = decay_coef(pmap((float)p[4], 0.004f, 3.5f));
        R.kick.penv = 1.0f;
        R.kick.drive = g;
        R.kick.norm = 1.0f / clip1(g);
        R.kick.amp = 1.0f;
        R.kick.ph = 0.0f;
        R.kick.click = 70;
        R.kick.click_g = (float)p[5] * (0.5f / 127.0f);
        R.kick.hold = (int32_t)p[6] * 30;           /* up to 86 ms at full level before it falls */
        R.kick.vel = vel;
        break;
    }
    case SS_BASSO:
        acid_trig(&R.basso, t, p, note_hz(root + (float)note), vel, acc, slide, 1);
        break;
    case SS_HATTU: {                    /* DECAY TONE METAL OPEN | CHIP SHAKE */
        float open = acc ? 1.0f + (float)p[3] * (7.0f / 127.0f) : 1.0f;
        R.hat.f = pmap((float)p[1], 3000.0f, 1.6f) * fexp2((float)note * (1.0f / 24.0f));
        R.hat.ac = decay_coef(pmap(drifted(t, 0, p[0]), 0.006f, 4.0f) * open);
        R.hat.metal = drifted(t, 1, p[2]) * (1.0f / 127.0f);
        R.hat.chip = (float)p[4] * (1.0f / 127.0f);
        R.hat.atc = p[5] ? 1.0f - decay_coef(pmap((float)p[5], 0.0005f, 6.0f)) : 1.0f;
        R.hat.att = p[5] ? 0.0f : 1.0f;
        R.hat.amp = 1.0f;
        R.hat.vel = vel;
        break;
    }
    case SS_HAPPO:
        acid_trig(&R.happo, t, p, note_hz(root + 12.0f + (float)note), vel, acc, slide, 0);
        break;
    case SS_PIIP: {                     /* WAVE DECAY ARP VIBRATO | SPEED PWM BEND DETUNE */
        int a = p[2] * 7 / 128, i;
        float hz = note_hz(root + 36.0f + (float)note), rate = pmap((float)p[4], 8.0f, 3.0f);
        R.piip.ft = hz;
        R.piip.wave = (uint8_t)(p[0] * 6 / 128);
        R.piip.ac = decay_coef(pmap(drifted(t, 0, p[1]), 0.02f, 5.5f));
        R.piip.arp_n = PIIP_ARP_N[a];
        for (i = 0; i < 3; i++)
            R.piip.mul[i] = semis((float)PIIP_ARP[a][i]);
        R.piip.arp_len = (int32_t)((float)SS_SR / rate);
        R.piip.vinc = rate * (0.25f * INV_SR);
        R.piip.vdep = drifted(t, 1, p[3]) * (0.06f / 127.0f);
        R.piip.pwm = (float)p[5] * (0.4f / 127.0f);
        R.piip.det = (float)p[7] * (0.02f / 127.0f);
        R.piip.vel = vel * (acc ? 1.0f : 0.8f);
        if (slide && R.piip.amp > 0.02f) {          /* a slide: portamento, the note goes on */
            R.piip.gc = 0.0012f;
            R.piip.amp = R.piip.amp * 0.5f + 0.5f;
            break;
        }
        R.piip.bend = ((float)p[6] - 64.0f) * (1.0f / 64.0f);   /* from below (-) or above (+), up to an octave */
        R.piip.bend = R.piip.bend >= 0.0f ? R.piip.bend : R.piip.bend * 0.5f;
        R.piip.benv = 1.0f;
        R.piip.bc = decay_coef(0.03f);
        R.piip.f = hz;
        R.piip.gc = 1.0f;
        R.piip.arp_i = R.piip.arp_cnt = 0;
        if (R.piip.amp < 0.05f)
            R.piip.att = 0.0f;
        R.piip.amp = 1.0f;
        break;
    }
    default: {                          /* OUTO: DECAY FM RATIO ZAP | ZTIME FEEDBK WOBBLE RANDOM */
        float rnd = (float)p[7] * (1.0f / 127.0f), zap = ((float)p[3] - 64.0f) * (1.0f / 63.0f);
        int ri = (int)(p[2] >> 4);
        float n = (float)note;
        if (rnd > 0.0f) {               /* never the same twice */
            ri = (ri + (int)(frand(&R.rng) * rnd * 7.99f)) & 7;
            zap += brand(&R.rng) * rnd;
            n += (float)(int)(brand(&R.rng) * rnd * 12.0f);
            R.outo.pan = 0.5f + brand(&R.rng) * rnd * 0.5f;
        } else {
            R.outo.pan = 0.5f;
        }
        zap = zap < -1.0f ? -1.0f : zap > 1.0f ? 1.0f : zap;
        R.outo.f = note_hz(root + 24.0f + n);
        R.outo.ac = decay_coef(pmap(drifted(t, 0, p[0]), 0.015f, 6.0f));
        R.outo.idx = drifted(t, 1, p[1]) * (1.1f / 127.0f);
        R.outo.ic = decay_coef(pmap((float)p[0], 0.01f, 6.0f));
        R.outo.ienv = 1.0f;
        R.outo.ratio = OUTO_RATIO[ri];
        R.outo.zap = zap >= 0.0f ? zap * zap * 7.0f : -zap * -0.9f;   /* falls from up to three octaves above,
                                                                      * or rises from more than three below */
        R.outo.zc = decay_coef(pmap((float)p[4], 0.004f, 7.0f));
        R.outo.zenv = 1.0f;
        R.outo.fb = (float)p[5] * (0.35f / 127.0f);
        R.outo.winc = p[6] ? pmap((float)p[6], 2.0f, 4.5f) * INV_SR : 0.0f;
        R.outo.wdep = (float)p[6] * (0.3f / 127.0f);
        if (R.outo.amp < 0.05f) {
            R.outo.att = 0.0f;
            R.outo.ph = R.outo.mph = 0.0f;
        }
        R.outo.amp = 1.0f;
        R.outo.vel = vel;
        break;
    }
    }
}

void ss_panic(void)
{
    uint32_t i, k;
    R.kick.amp = R.basso.amp = R.hat.amp = R.happo.amp = R.piip.amp = R.outo.amp = 0.0f;
    R.duck_env = 0.0f;
    for (i = 0; i < SS_TRACKS; i++) {
        R.pend[i][0].on = R.pend[i][1].on = R.early[i] = 0;
        R.crush[i].held = 0.0f;
        R.crush[i].cnt = 0;
    }
    for (i = 0; i < DLY_LEN; i++)
        dly_buf[i] = 0;
    for (k = 0; k < 4; k++) {
        for (i = 0; i < COMB_LEN[k]; i++)
            comb_buf[k][i] = 0.0f;
        for (i = 0; i < AP_LEN[k]; i++)
            ap_buf[k][i] = 0.0f;
        R.comb_f[k] = 0.0f;
    }
    R.dly_lp = 0.0f;
}

/* ------------------------------------------------------------- sequencer --- */
static void rt_push(uint8_t b)
{
    uint8_t w = R.rt_w;
    if (!R.prj.clock_out || (uint8_t)((w + 1u) & 15u) == R.rt_r)
        return;
    R.rt[w] = b;
    R.rt_w = (uint8_t)((w + 1u) & 15u);
}

int ss_midi_rt_pop(void)
{
    uint8_t r = R.rt_r, b;
    if (r == R.rt_w)
        return -1;
    b = R.rt[r];
    R.rt_r = (uint8_t)((r + 1u) & 15u);
    return b;
}

static int scale_shift(int note, int d);
static float step_samples(void) { return (float)SS_SR * 15.0f / (float)R.prj.bpm; }

/* a step's microtiming: its own shift and its track's, together at most half a sixteenth either way (an early
 * step is scheduled one step ahead, and at full swing the short sixteenth is only that long) */
static int eff_shift(const ss_track_t *tr, uint32_t pos)
{
    int v = tr->step[pos].shift + tr->tshift;
    return v < -8 ? -8 : v > 8 ? 8 : v;
}

/* one step of a track, to play base samples from now (plus its own shift); dur: that step's length.
 * Returns 1 if the step fired: a muted track's counts too (the sidechain and the LINK page follow it unheard) */
static int schedule(int t, uint32_t pos, float base, float dur)
{
    static const uint8_t PROB[4] = {100, 75, 50, 25};
    const ss_track_t *tr = &PAT->tr[t];
    ss_step_t st = tr->step[pos];
    float seko = (float)R.prj.seko * (1.0f / 127.0f), d;
    int k;
    if (!(st.flags & SS_ST_ON))
        return 0;
    if (xs32(&R.rng) % 100u >= PROB[SS_ST_PROB(st.flags)])
        return 0;
    if (tr->mute || (R.solo >= 0 && R.solo != t)) {
        if (t == (int)R.prj.sc_src)     /* the sidechain's source, muted: it still ducks (a ghost kick) */
            R.duck_env = 1.0f;
        return 1;
    }
    k = !R.pend[t][0].on ? 0 : !R.pend[t][1].on ? 1 : 0;
    R.pend[t][k].note = st.note;
    R.pend[t][k].si = (uint8_t)pos;
    R.pend[t][k].acc = (st.flags & SS_ST_ACC) != 0;
    R.pend[t][k].vel = (R.pend[t][k].acc ? 1.0f : 0.62f) * (1.0f + brand(&R.rng) * seko * 0.15f);
    d = base + (float)eff_shift(tr, pos) * step_samples() * (1.0f / 16.0f);
    R.pend[t][k].delay = d > 0.0f ? (int32_t)d : 0;
    R.pend[t][k].rep = (uint8_t)(SS_ST_RATCH(st.flags) - 1u);   /* a ratchet: more hits, evenly within the step */
    R.pend[t][k].gap = (int32_t)(dur / (float)SS_ST_RATCH(st.flags));
    R.pend[t][k].sl = (st.flags & SS_ST_SLIDE) != 0;
    if (t != SS_POTKU && frand(&R.rng) < seko * 0.3f) {   /* SEKO: this time the step is another */
        uint32_t m = xs32(&R.rng) & 7u;
        int nn = st.note;
        if (t == SS_HATTU) {
            if (m < 3u && !R.pend[t][k].rep) {
                R.pend[t][k].rep = 1;
                R.pend[t][k].gap = (int32_t)(dur * 0.5f);
            } else if (m == 3u) {
                R.pend[t][k].acc ^= 1u;
            }
        } else if (m < 4u) {            /* a degree or two of the scale up or down */
            nn = scale_shift(nn, (m & 1u) ? 1 + (int)(m >> 1) : -1 - (int)(m >> 1));
        } else if (m < 6u) {            /* an octave */
            nn += nn < 12 ? 12 : -12;
        } else if (m == 6u) {
            R.pend[t][k].sl = 1;
        } else if (!R.pend[t][k].rep) {
            R.pend[t][k].rep = 1;
            R.pend[t][k].gap = (int32_t)(dur * 0.5f);
        }
        R.pend[t][k].note = (int8_t)(nn < -48 ? -48 : nn > 48 ? 48 : nn);
    }
    R.pend[t][k].on = 1;
    R.hits[t]++;
    return 1;
}

static void seq_step(void)
{
    float sw = (float)R.prj.swing * 0.01f, s = step_samples();
    float d_long = s * 2.0f * sw, d_short = s * 2.0f * (1.0f - sw);   /* swing: each pair of 16ths long, short */
    float dur = (R.gstep & 1u) ? d_short : d_long, next = (R.gstep & 1u) ? d_long : d_short;
    uint32_t done = 0;
    int t, pass;
    if (R.prj.chain && R.gstep && (R.gstep & 63u) == 0u && R.pending_pat == 0xFFu)
        R.prj.cur_pat = (uint8_t)((R.prj.cur_pat + 1u) % (R.prj.chain + 1u));
    if (R.pending_pat != 0xFFu && (R.gstep & 15u) == 0u) {
        R.prj.cur_pat = R.pending_pat;
        R.pending_pat = 0xFFu;
    }
    for (t = 0; t < SS_TRACKS; t++) {
        int k;
        R.hit_prev[t] = R.hit[t];
        R.hit[t] = 0;
        R.pos[t] = (uint8_t)(R.gstep % (PAT->tr[t].len ? PAT->tr[t].len : 1u));
        for (k = 0; k < 2; k++) {       /* the drift: a slow random walk, pulled back to the middle */
            float d = R.drift[t][k] * 0.985f + brand(&R.rng) * 0.11f;
            R.drift[t][k] = d < -1.0f ? -1.0f : d > 1.0f ? 1.0f : d;
        }
    }
    /* the free tracks first, then those linked to a track already decided; a ring of links never fires */
    for (pass = 0; pass < SS_TRACKS; pass++)
        for (t = 0; t < SS_TRACKS; t++) {
            const ss_track_t *tr = &PAT->tr[t];
            uint32_t len = tr->len ? tr->len : 1u, pos = R.gstep % len, npos = (R.gstep + 1u) % len;
            int src = tr->dep_src && tr->dep_src - 1 != t ? tr->dep_src - 1 : -1, open = 1;
            if (done >> t & 1u)
                continue;
            if (src >= 0) {
                if (tr->dep_mode == 2)
                    open = R.hit_prev[src];
                else if (!(done >> src & 1u))
                    continue;           /* its source is not decided yet: a later pass */
                else
                    open = tr->dep_mode == 1 ? !R.hit[src] : R.hit[src];
            }
            done |= 1u << t;
            /* a step shifted early was scheduled a step ago; the very first step cannot be early, and neither can
             * a linked track's: its source is not known a step ahead */
            if (R.early[t]) {
                R.early[t] = 0;
                R.hit[t] = R.hit_early[t];
            } else if (eff_shift(tr, pos) >= 0 || R.gstep == 0u || src >= 0) {
                R.hit[t] = (uint8_t)(open && schedule(t, pos, 0.0f, dur));
            }
            if (src < 0 && eff_shift(tr, npos) < 0) {
                R.hit_early[t] = (uint8_t)schedule(t, npos, dur, next);
                R.early[t] = 1;
            }
        }
    R.clk += dur;
    R.gstep++;
    R.gstep_ui = R.gstep;
}

static void transport(int start)
{
    if (start) {
        int t;
        for (t = 0; t < SS_TRACKS; t++) {
            R.early[t] = R.hit[t] = R.hit_prev[t] = 0;
            R.hits[t] = 0;
        }
        R.gstep = 0;
        R.clk = 0.0f;
        R.mclk = 0.0f;
        R.playing = 1;
        rt_push(0xFA);
    } else if (R.playing) {
        R.playing = 0;
        rt_push(0xFC);
    }
}

void ss_midi_transport(int start) { transport(start); }

void ss_midi_note(uint32_t channel, uint32_t note, uint32_t velocity)
{
    int t = -1, n = 0;
    if (channel < SS_TRACKS) {
        t = (int)channel;
        n = (int)note - 60;
    } else if (channel == 9u) {
        static const uint8_t MAP[12] = {SS_POTKU, SS_OUTO, SS_OUTO, SS_OUTO, 255, 255,
                                        SS_HATTU, 255, SS_HATTU, 255, SS_HATTU, 255};   /* 36 .. 47 */
        if (note >= 36u && note < 48u && MAP[note - 36u] != 255u)
            t = MAP[note - 36u];
        if (note == 46u)
            velocity = 127;             /* the open hat is the accent */
    }
    if (t >= 0)
        trig(t, n < -48 ? -48 : n > 48 ? 48 : n, (float)velocity * (1.0f / 127.0f), velocity > 100u, 0, 0);
}

static void tq_push(int t, int note, int vel)   /* main loop */
{
    uint8_t w = R.tq_w;
    if ((uint8_t)((w + 1u) & 15u) == R.tq_r)
        return;
    R.tq[w][0] = (uint8_t)t;
    R.tq[w][1] = (uint8_t)(note + 64);
    R.tq[w][2] = (uint8_t)vel;
    R.tq_w = (uint8_t)((w + 1u) & 15u);
}

/* ---------------------------------------------------------------- render --- */
void ss_render(int32_t *lr, uint32_t n, uint32_t gain_q12)
{
    uint32_t i, k;
    float s16 = step_samples(), gain = (float)gain_q12 * (8388607.0f / 4096.0f);
    float duck_amt = (float)R.prj.duck * (0.9f / 127.0f), duck_c = decay_coef(pmap((float)R.prj.sc_rel, 0.03f, 3.7f));
    float sc[SS_TRACKS], mdrive = 0.4f * (1.0f + (float)R.prj.drive * (3.0f / 127.0f));
    float fb = (float)R.prj.dly_fb * (0.85f / 127.0f);
    float rfb = 0.70f + (float)R.prj.rev_size * (0.27f / 127.0f), damp = 0.05f + (float)R.prj.rev_damp * (0.7f / 127.0f);
    static const float DMUL[3] = {1.0f, 2.0f, 3.0f};
    uint32_t dl = (uint32_t)(s16 * DMUL[R.prj.dly_time > 2 ? 2 : R.prj.dly_time]), dr;
    while (dl > DLY_LEN - 8u)           /* a slow tempo: the next shorter note value that fits */
        dl = dl * 2u / 3u;
    if (dl < 16u)
        dl = 16u;
    dr = dl * 2u / 3u;

    if (R.want_play) {
        R.want_play = 0;
        transport(1);
    }
    if (R.want_stop) {
        R.want_stop = 0;
        transport(0);
    }
    while (R.tq_r != R.tq_w) {          /* keys played in the main loop */
        uint8_t r = R.tq_r;
        trig(R.tq[r][0], (int)R.tq[r][1] - 64, (float)R.tq[r][2] * (1.0f / 127.0f), R.tq[r][2] > 100u, 0, 0);
        R.tq_r = (uint8_t)((r + 1u) & 15u);
    }
    for (k = 0; k < SS_TRACKS; k++) {
        bus_update((int)k);
        sc[k] = k == R.prj.sc_src ? 0.0f : (float)PAT->tr[k].sc * (1.0f / 127.0f);
    }

    for (i = 0; i < n; i++) {
        float v[SS_TRACKS], l, r, sd, sr, duck, x, dly_l, dly_r, rv, rl, rr;
        if (R.playing) {
            if (R.clk <= 0.0f)
                seq_step();
            R.clk -= 1.0f;
            if (R.mclk <= 0.0f) {
                rt_push(0xF8);
                R.mclk += s16 * (1.0f / 6.0f);
            }
            R.mclk -= 1.0f;
        }
        for (k = 0; k < 2u * SS_TRACKS; k++) {
            uint32_t t = k >> 1, j = k & 1u;
            if (R.pend[t][j].on && --R.pend[t][j].delay < 0) {
                trig((int)t, R.pend[t][j].note, R.pend[t][j].vel, R.pend[t][j].acc, R.pend[t][j].sl,
                     PAT->tr[t].step[R.pend[t][j].si & 15u].lock);
                if (R.pend[t][j].rep) {
                    R.pend[t][j].rep--;
                    R.pend[t][j].delay = R.pend[t][j].gap;
                    R.pend[t][j].vel *= 0.82f;
                } else {
                    R.pend[t][j].on = 0;
                }
            }
        }

        /* POTKU: a sine falling fast from far above, a click, a soft clipper */
        x = 0.0f;
        if (R.kick.amp > 1e-4f) {
            R.kick.ph += R.kick.f0 * (1.0f + R.kick.pd * R.kick.penv) * INV_SR;
            if (R.kick.ph >= 1.0f)
                R.kick.ph -= 1.0f;
            x = fsin(R.kick.ph);
            if (R.kick.click > 0) {
                x += brand(&R.rng) * R.kick.click_g * (float)R.kick.click * (1.0f / 70.0f);
                R.kick.click--;
            }
            x = clip1(x * R.kick.amp * R.kick.drive) * R.kick.norm * R.kick.vel;
            R.kick.penv *= R.kick.pc;
            if (R.kick.hold > 0)
                R.kick.hold--;
            else
                R.kick.amp *= R.kick.ac;
        }
        v[SS_POTKU] = x;

        v[SS_BASSO] = acid_run(&R.basso);
        v[SS_HAPPO] = acid_run(&R.happo) * 0.8f;

        /* HATTU: two inharmonic FM pairs against noise, or a sound chip's shift register, high-passed */
        x = 0.0f;
        if (R.hat.amp > 1e-4f) {
            float f = R.hat.f * INV_SR, m;
            R.hat.ph += f;
            R.hat.mph += f * 1.4471f;
            R.hat.ph2 += f * 1.6170f;
            R.hat.mph2 += f * 2.3890f;
            if (R.hat.ph >= 1.0f) R.hat.ph -= 1.0f;
            if (R.hat.mph >= 1.0f) R.hat.mph -= 1.0f;
            if (R.hat.ph2 >= 1.0f) R.hat.ph2 -= 1.0f;
            if (R.hat.mph2 >= 1.0f) R.hat.mph2 -= 1.0f;
            m = (fsin(R.hat.ph + fsin(R.hat.mph) * 0.9f) + fsin(R.hat.ph2 + fsin(R.hat.mph2) * 0.7f)) * 0.5f;
            m = m * R.hat.metal + brand(&R.rng) * (1.0f - R.hat.metal);
            if (R.hat.chip > 0.0f) {
                R.hat.cph += f * 3.0f;
                if (R.hat.cph >= 1.0f) {
                    uint32_t b = (R.hat.lfsr ^ R.hat.lfsr >> 1) & 1u;
                    R.hat.cph -= (float)(int)R.hat.cph;
                    R.hat.lfsr = R.hat.lfsr >> 1 | b << 14;
                    R.hat.nz = R.hat.lfsr & 1u ? 0.8f : -0.8f;
                }
                m += (R.hat.nz - m) * R.hat.chip;
            }
            R.hat.lp += (m - R.hat.lp) * 0.45f;
            x = (m - R.hat.lp) * R.hat.amp * R.hat.att * R.hat.vel * 1.6f;
            R.hat.att += (1.0f - R.hat.att) * R.hat.atc;
            R.hat.amp *= R.hat.ac;
        }
        v[SS_HATTU] = x;

        /* PIIP: the waves of a sound chip, naive on purpose: pulses of three widths, a 4-bit triangle, a saw,
         * pitched noise; its chord a note a frame, vibrato, a pitch bend at the start */
        x = 0.0f;
        if (R.piip.amp > 1e-4f) {
            float f, duty, lfo;
            R.piip.f += (R.piip.ft - R.piip.f) * R.piip.gc;
            if (R.piip.arp_n > 1 && ++R.piip.arp_cnt >= R.piip.arp_len) {
                R.piip.arp_cnt = 0;
                R.piip.arp_i = (R.piip.arp_i + 1) % R.piip.arp_n;
            }
            R.piip.vph += R.piip.vinc;
            if (R.piip.vph >= 1.0f)
                R.piip.vph -= 1.0f;
            lfo = fsin(R.piip.vph);
            f = R.piip.f * R.piip.mul[R.piip.arp_i] * (1.0f + R.piip.vdep * lfo) * (1.0f + R.piip.bend * R.piip.benv) * INV_SR;
            R.piip.benv *= R.piip.bc;
            R.piip.ph += f;
            if (R.piip.ph >= 1.0f)
                R.piip.ph -= 1.0f;
            R.piip.ph2 += f * (1.0f + R.piip.det);
            if (R.piip.ph2 >= 1.0f)
                R.piip.ph2 -= 1.0f;
            duty = R.piip.wave == 0 ? 0.125f : R.piip.wave == 1 ? 0.25f : 0.5f;
            duty += R.piip.pwm * lfo * duty;
            switch (R.piip.wave) {
            case 3: {
                float tr = R.piip.ph < 0.5f ? 4.0f * R.piip.ph : 4.0f - 4.0f * R.piip.ph;   /* 0..2 */
                x = (float)(int)(tr * 7.99f) * (1.0f / 7.5f) - 1.0f;
                break;
            }
            case 4:
                x = 2.0f * R.piip.ph - 1.0f;
                if (R.piip.det > 0.0f)
                    x = (x + 2.0f * R.piip.ph2 - 1.0f) * 0.5f;
                x *= 0.8f;
                break;
            case 5:
                R.piip.nph += f * 12.0f;
                if (R.piip.nph >= 1.0f) {
                    uint32_t b = (R.piip.lfsr ^ R.piip.lfsr >> 6) & 1u;   /* the short sequence: it has a pitch */
                    R.piip.nph -= (float)(int)R.piip.nph;
                    R.piip.lfsr = R.piip.lfsr >> 1 | b << 14;
                    R.piip.nz = R.piip.lfsr & 1u ? 0.8f : -0.8f;
                }
                x = R.piip.nz;
                break;
            default:
                x = R.piip.ph < duty ? 0.7f : -0.7f;
                if (R.piip.det > 0.0f)
                    x = (x + (R.piip.ph2 < duty ? 0.7f : -0.7f)) * 0.5f;
                break;
            }
            x *= R.piip.amp * R.piip.att * R.piip.vel * 0.9f;
            R.piip.att += (1.0f - R.piip.att) * 0.05f;
            R.piip.amp *= R.piip.ac;
        }
        v[SS_PIIP] = x;

        /* OUTO: an FM pair with feedback whose pitch a zap envelope throws about: lasers, bubbles, squeaks */
        x = 0.0f;
        if (R.outo.amp > 1e-4f) {
            float f = R.outo.f * (1.0f + R.outo.zap * R.outo.zenv) * INV_SR, m;
            if (R.outo.winc > 0.0f) {
                R.outo.wph += R.outo.winc;
                if (R.outo.wph >= 1.0f)
                    R.outo.wph -= 1.0f;
                f *= 1.0f + R.outo.wdep * fsin(R.outo.wph);
            }
            R.outo.ph += f;
            R.outo.mph += f * R.outo.ratio;
            if (R.outo.ph >= 1.0f) R.outo.ph -= (float)(int)R.outo.ph;
            if (R.outo.mph >= 1.0f) R.outo.mph -= (float)(int)R.outo.mph;
            m = fsin(R.outo.mph + R.outo.fb * R.outo.last);
            R.outo.last = m;
            x = fsin(R.outo.ph + m * R.outo.idx * (0.2f + 0.8f * R.outo.ienv)) * R.outo.amp * R.outo.att * R.outo.vel * 0.8f;
            R.outo.att += (1.0f - R.outo.att) * 0.03f;
            R.outo.zenv *= R.outo.zc;
            R.outo.ienv *= R.outo.ic;
            R.outo.amp *= R.outo.ac;
        }
        v[SS_OUTO] = x;

        /* the 8-bit part, a track at a time: a sample held for RATE samples, rounded to BITS levels */
        for (k = 0; k < SS_TRACKS; k++) {
            if (R.crush[k].n) {
                if (++R.crush[k].cnt >= R.crush[k].n) {
                    R.crush[k].cnt = 0;
                    R.crush[k].held = v[k];
                }
                v[k] = R.crush[k].held;
            }
            if (R.crush[k].q > 0.0f) {
                float y = v[k] * R.crush[k].q;
                v[k] = (float)(int)(y + (y < 0.0f ? -0.5f : 0.5f)) / R.crush[k].q;
            }
        }

        /* the mix: the sidechain's source ducks each track by that track's amount, and the effects */
        x = duck_amt * R.duck_env;
        duck = 1.0f - x;
        R.duck_env *= duck_c;
        l = r = sd = sr = 0.0f;
        for (k = 0; k < SS_TRACKS; k++) {
            float y = v[k] * (1.0f - x * sc[k]);
            if (k == SS_OUTO) {
                l += y * R.bus[k].gl * 2.0f * (1.0f - R.outo.pan);
                r += y * R.bus[k].gr * 2.0f * R.outo.pan;
            } else {
                l += y * R.bus[k].gl;
                r += y * R.bus[k].gr;
            }
            sd += y * R.bus[k].sd;
            sr += y * R.bus[k].sr;
        }

        /* delay: one line, two taps (left the full time, right two thirds of it), filtered feedback */
        dly_l = (float)dly_buf[(R.dly_w - dl) & (DLY_LEN - 1u)] * (1.0f / 16384.0f);
        dly_r = (float)dly_buf[(R.dly_w - dr) & (DLY_LEN - 1u)] * (1.0f / 16384.0f);
        R.dly_lp += (dly_l - R.dly_lp) * 0.35f;
        x = clip1(sd + R.dly_lp * fb) * 16384.0f;
        dly_buf[R.dly_w] = (int16_t)x;
        R.dly_w = (R.dly_w + 1u) & (DLY_LEN - 1u);
        sr += (dly_l + dly_r) * 0.15f;

        /* reverb: four damped combs, then two all-passes a side */
        rv = 0.0f;
        sr = sr * 0.25f + 1e-20f;
        for (k = 0; k < 4; k++) {
            float y = comb_buf[k][R.comb_i[k]];
            R.comb_f[k] = y + (R.comb_f[k] - y) * damp;
            comb_buf[k][R.comb_i[k]] = sr + R.comb_f[k] * rfb;
            if (++R.comb_i[k] >= COMB_LEN[k])
                R.comb_i[k] = 0;
            rv += y;
        }
        rl = rr = rv;
        for (k = 0; k < 4; k++) {
            float *io = k < 2 ? &rl : &rr, b = ap_buf[k][R.ap_i[k]];
            ap_buf[k][R.ap_i[k]] = *io + b * 0.5f;
            *io = b - *io;
            if (++R.ap_i[k] >= AP_LEN[k])
                R.ap_i[k] = 0;
        }
        l += dly_l + rl * duck;
        r += dly_r + rr * duck;

        lr[2u * i] = (int32_t)(clip1(l * mdrive) * gain);
        lr[2u * i + 1u] = (int32_t)(clip1(r * mdrive) * gain);
    }
}

/* ------------------------------------------------------------- generator --- */
/* What makes a pattern suomisaundi here: the kick on every beat, a bass line that bounces between the kicks and
 * jumps the octave, an acid line and a melody on loops of their own lengths that walk through the scale, and
 * zaps that come seldom and never the same. */
static uint32_t urand(uint32_t n) { return xs32(&R.ui_rng) % n; }
static int scale_note(int degree)
{
    uint32_t s = R.prj.scale < SS_NSCALES ? R.prj.scale : 0u;
    int n = SCALE_N[s], oct = 0;
    while (degree < 0) {
        degree += n;
        oct--;
    }
    while (degree >= n) {
        degree -= n;
        oct++;
    }
    return SCALES[s][degree] + 12 * oct;
}
/* a note moved by d degrees of the scale (a note off the scale counts as the degree below it) */
static int scale_shift(int note, int d)
{
    uint32_t s = R.prj.scale < SS_NSCALES ? R.prj.scale : 0u;
    int n = SCALE_N[s], oct = 0, deg = 0, i;
    while (note < 0) {
        note += 12;
        oct--;
    }
    while (note >= 12) {
        note -= 12;
        oct++;
    }
    for (i = 0; i < n; i++)
        if (SCALES[s][i] <= note)
            deg = i;
    return scale_note(deg + d + oct * n);
}
static void step_plain(ss_step_t *st)   /* no shift, no locks */
{
    st->shift = 0;
    st->lock[0] = st->lock[1] = st->lock[2] = st->lock[3] = 0;
}
static void set_step(ss_track_t *tr, uint32_t i, int acc, int prob, int note)
{
    tr->step[i % SS_STEPS].flags = (uint8_t)(SS_ST_ON | (acc ? SS_ST_ACC : 0u) | ((uint32_t)prob << 4));
    tr->step[i % SS_STEPS].note = (int8_t)note;
}
static void euclid(ss_track_t *tr, uint32_t hits, uint32_t rot)
{
    uint32_t i, acc = 0;
    for (i = 0; i < tr->len; i++) {
        acc += hits;
        if (acc >= tr->len) {
            acc -= tr->len;
            set_step(tr, (i + rot) % tr->len, 0, 0, 0);
        }
    }
}

static void gen_track(int t)
{
    ss_track_t *tr = &PAT->tr[t];
    uint32_t i, k;
    for (i = 0; i < SS_STEPS; i++) {
        tr->step[i].flags = 0;
        tr->step[i].note = 0;
        step_plain(&tr->step[i]);
    }
    tr->len = 16;
    switch (t) {
    case SS_POTKU:
        for (i = 0; i < 16; i += 4)
            set_step(tr, i, 1, 0, 0);
        k = urand(5);
        if (k == 0)
            set_step(tr, 14, 0, 2, 0);              /* a ghost before the bar, half the time */
        else if (k == 1)
            tr->step[12].flags |= 0x10u | 1u;       /* a double on the last beat, three times in four */
        break;
    case SS_BASSO: {
        /* 0 rest, 1 the root, 2 its octave, 3 the fifth, 4 the note below the root, 5 the third */
        static const uint8_t LINE[6][16] = {
            {0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 2},   /* the gallop */
            {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 2},   /* rolling */
            {0, 0, 1, 0, 0, 1, 2, 0, 0, 0, 1, 1, 0, 2, 0, 1},   /* funk */
            {0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 2, 1},   /* the off-beat, in octaves */
            {0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 2, 2},   /* over the kick */
            {0, 0, 1, 1, 0, 2, 0, 1, 0, 0, 1, 1, 0, 3, 2, 1},
        };
        const uint8_t *ln = LINE[urand(6)];
        uint32_t odd = urand(3) == 0 ? urand(16) : 99u;   /* one note off the root, in one line of three */
        for (i = 0; i < 16; i++) {
            int kind = ln[i], note;
            if (!kind)
                continue;
            if (i == odd)
                kind = 3 + (int)urand(3);
            note = kind == 1 ? 0 : kind == 2 ? 12 : scale_note(kind == 3 ? 4 : kind == 4 ? -1 : 2);
            set_step(tr, i, kind != 1 || urand(6) == 0, urand(8) == 0 ? 1 : 0, note);
            if (kind != 1 && i && ln[i - 1u] && urand(5) < 2u)
                tr->step[i].flags |= SS_ST_SLIDE;   /* into the octave on a slide */
        }
        break;
    }
    case SS_HATTU:
        for (i = 2; i < 16; i += 4)
            set_step(tr, i, urand(3) != 0, 0, 0);   /* the off-beat, mostly open */
        for (i = 0; i < 16; i++)
            if (!(tr->step[i].flags & SS_ST_ON) && urand(100) < 50u) {
                set_step(tr, i, 0, (int)urand(3), 0);
                if (urand(7) == 0)
                    tr->step[i].flags |= 1u;        /* now and then a double */
            }
        break;
    case SS_HAPPO: {                    /* the acid line: a walk through the scale, accents and slides */
        static const uint8_t LEN[6] = {16, 16, 12, 14, 8, 10};
        int deg = 0, prev = 0, on = 0;
        tr->len = LEN[urand(6)];
        for (i = 0; i < tr->len; i++) {
            if (urand(100) >= 62u) {
                prev = 0;
                continue;
            }
            deg += (int)urand(5) - 2;
            deg = deg < -3 ? -3 : deg > 8 ? 8 : deg;
            set_step(tr, i, urand(4) == 0, urand(6) == 0 ? 1 : 0, scale_note(deg) + (urand(8) == 0 ? 12 : 0));
            if (prev && urand(3) == 0)
                tr->step[i].flags |= SS_ST_SLIDE;
            prev = 1;
            on++;
        }
        if (on < 2) {
            set_step(tr, 2, 1, 0, 0);
            set_step(tr, 3, 0, 0, scale_note(2));
        }
        break;
    }
    case SS_PIIP: {                     /* the melody: a figure of four notes on an even spread, moved up every other time */
        static const uint8_t LEN[5] = {16, 16, 12, 10, 6};
        static const int8_t SHAPE[6][4] = {{0, 2, 4, 7}, {7, 4, 2, 0}, {0, 4, 2, 5}, {0, 2, 4, 2}, {4, 2, 0, -1}, {0, 7, 4, 9}};
        const int8_t *sh = SHAPE[urand(6)];
        uint32_t n = 0, up = 1 + urand(3);
        tr->len = LEN[urand(5)];
        euclid(tr, tr->len / 2u + urand(tr->len / 4u + 1u), urand(tr->len));
        for (i = 0; i < tr->len; i++)
            if (tr->step[i].flags & SS_ST_ON) {
                set_step(tr, i, n % 4u == 0u, urand(5) == 0 ? 2 : 0, scale_note(sh[n % 4u] + (n / 4u % 2u ? (int)up : 0)));
                if (n && urand(6) == 0)
                    tr->step[i].flags |= SS_ST_SLIDE;
                n++;
            }
        break;
    }
    default: {                          /* OUTO: a few zaps on an odd length, seldom, each with its own sound */
        static const uint8_t LEN[5] = {16, 11, 13, 7, 9};
        tr->len = LEN[urand(5)];
        euclid(tr, 1 + urand(3), urand(tr->len));
        for (i = 0; i < tr->len; i++)
            if (tr->step[i].flags & SS_ST_ON) {
                set_step(tr, i, (int)urand(2), 1 + (int)urand(3), (int)urand(25) - 12);
                tr->step[i].lock[1] = (uint8_t)(1u + urand(128));
                tr->step[i].lock[2] = (uint8_t)(1u + urand(128));
                tr->step[i].lock[3] = (uint8_t)(1u + urand(128));
                if (urand(4) == 0)
                    tr->step[i].flags |= (uint8_t)(1u + urand(3));   /* a stutter */
            }
        break;
    }
    }
    tr->eu_rot = 0;
    tr->eu_hits = 0;
    for (i = 0; i < SS_STEPS; i++)
        tr->eu_hits += (tr->step[i].flags & SS_ST_ON) != 0;
}

/* the EUCLID page: hits spread as evenly as the loop allows, rotated; the notes stay */
static void apply_euclid(int t)
{
    ss_track_t *tr = &PAT->tr[t];
    uint32_t i, n = 0, len = tr->len ? tr->len : 1u;
    if (tr->eu_hits > len)
        tr->eu_hits = (uint8_t)len;
    if (tr->eu_rot >= len)
        tr->eu_rot = (uint8_t)(len - 1u);
    for (i = 0; i < SS_STEPS; i++)
        tr->step[i].flags = 0;
    for (i = 0; i < len; i++)
        if (i * tr->eu_hits % len < tr->eu_hits) {
            uint32_t acc = tr->eu_acc && n % tr->eu_acc == 0u ? SS_ST_ACC : 0u;
            tr->step[(i + tr->eu_rot) % len].flags = (uint8_t)(SS_ST_ON | acc | (3u - (tr->eu_prob & 3u)) << 4);
            n++;
        }
}

static void mutate_track(int t)
{
    ss_track_t *tr = &PAT->tr[t];
    uint32_t i = urand(tr->len ? tr->len : 1u), kind = urand(4);
    ss_step_t *st = &tr->step[i];
    if (t == SS_POTKU && (i & 3u) == 0u)
        kind = 1;                       /* the four on the floor stays; its probability does not change either */
    if (kind == 0) {
        if (st->flags & SS_ST_ON)
            st->flags = (uint8_t)(st->flags & ~SS_ST_ON);
        else
            set_step(tr, i, 0, 1 + (int)urand(3), st->note);
    } else if (kind == 1 && t != SS_POTKU) {
        if (st->flags & SS_ST_ON)
            st->flags = (uint8_t)((st->flags & ~0x30u) | (urand(4) << 4));
    } else if (kind == 2) {
        if ((st->flags & SS_ST_ON) && t != SS_POTKU && t != SS_HATTU)
            st->note = (int8_t)scale_note((int)urand(6) - 1);
    } else if (st->flags & SS_ST_ON) {
        st->flags ^= SS_ST_ACC;
    }
}

static void rotate_track(int t, int dir)
{
    ss_track_t *tr = &PAT->tr[t];
    uint32_t i, n = tr->len;
    ss_step_t s;
    if (n < 2u)
        return;
    if (dir > 0) {
        s = tr->step[n - 1u];
        for (i = n - 1u; i > 0; i--)
            tr->step[i] = tr->step[i - 1u];
        tr->step[0] = s;
    } else {
        s = tr->step[0];
        for (i = 0; i + 1u < n; i++)
            tr->step[i] = tr->step[i + 1u];
        tr->step[n - 1u] = s;
    }
}

static void pattern_defaults(ss_pattern_t *p)
{
    /* (tools/manual.py reads this table and SOUND_NAME for the manuals) */
    static const uint8_t DEF[SS_TRACKS][14] = {
        /* p0   p1   p2   p3   p4   p5   p6   p7  level pan  dly  rev  bits rate */
        {  48,  58,  62,  40,  40,  50,  20,   0, 122,  64,   0,   0,   0,   0},   /* POTKU */
        {  52,  40,  60,  34,   0,  40,  50,  40, 108,  64,   0,   0,   0,   0},   /* BASSO */
        {  40,  72,  70,  64,   0,   0,   0,   0,  84,  76,  14,  14,   0,   0},   /* HATTU */
        {  58,  96,  74,  44,   0,  90,  60,  60,  84,  52,  60,  20,   0,   0},   /* HAPPO */
        {  30,  46,  40,  30,  64,  30,  64,   0,  76,  80,  70,  40,   0,   0},   /* PIIP */
        {  60,  80,  48, 100,  50,  40,   0,  50,  80,  64,  60,  50,   0,   0},   /* OUTO */
    };
    int t, i;
    p->xpose = 0;
    p->rsv[0] = p->rsv[1] = p->rsv[2] = 0;
    for (t = 0; t < SS_TRACKS; t++) {
        ss_track_t *tr = &p->tr[t];
        for (i = 0; i < 8; i++)
            tr->p[i] = DEF[t][i];
        tr->level = DEF[t][8];
        tr->pan = DEF[t][9];
        tr->dly = DEF[t][10];
        tr->rev = DEF[t][11];
        tr->bits = DEF[t][12];
        tr->rate = DEF[t][13];
        tr->mute = 0;
        tr->eu_hits = tr->eu_rot = tr->eu_acc = tr->dep_src = tr->dep_mode = 0;
        tr->tshift = 0;
        tr->eu_prob = 3;
        tr->sc = t == SS_POTKU ? 0 : t == SS_BASSO ? 127 : 50;
        tr->len = 16;
        for (i = 0; i < SS_STEPS; i++) {
            tr->step[i].flags = 0;
            tr->step[i].note = 0;
            tr->step[i].rsv = 0;
            step_plain(&tr->step[i]);
        }
    }
}

/* --------------------------------------------------------------- project --- */
static uint32_t prj_sum(const ss_project_t *p)
{
    const uint8_t *b = (const uint8_t *)p;
    uint32_t i, n = (uint32_t)((const uint8_t *)&p->sum - b), a = 1, c = 0;
    for (i = 0; i < n; i++) {           /* Adler-32 */
        a = (a + b[i]) % 65521u;
        c = (c + a) % 65521u;
    }
    return c << 16 | a;
}

static void prj_sanitize(ss_project_t *p)
{
    int a, t, i;
#define LIM(v, lo, hi) do { if ((v) < (lo)) (v) = (lo); if ((v) > (hi)) (v) = (hi); } while (0)
    LIM(p->bpm, 120, 180);
    LIM(p->swing, 50, 75);
    LIM(p->seko, 0, 127);
    LIM(p->drive, 0, 127);
    LIM(p->duck, 0, 127);
    LIM(p->root, 24, 47);
    LIM(p->scale, 0, SS_NSCALES - 1);
    LIM(p->dly_time, 0, 2);
    LIM(p->dly_fb, 0, 127);
    LIM(p->rev_size, 0, 127);
    LIM(p->rev_damp, 0, 127);
    LIM(p->clock_out, 0, 1);
    LIM(p->cur_pat, 0, SS_PATTERNS - 1);
    LIM(p->chain, 0, SS_PATTERNS - 1);
    LIM(p->sc_src, 0, SS_TRACKS - 1);
    LIM(p->sc_rel, 0, 127);
    for (a = 0; a < SS_PATTERNS; a++) {
        LIM(p->pat[a].xpose, -12, 12);
        for (t = 0; t < SS_TRACKS; t++) {
            ss_track_t *tr = &p->pat[a].tr[t];
            LIM(tr->len, 1, SS_STEPS);
            LIM(tr->mute, 0, 1);
            LIM(tr->level, 0, 127);
            LIM(tr->pan, 0, 127);
            LIM(tr->dly, 0, 127);
            LIM(tr->rev, 0, 127);
            LIM(tr->sc, 0, 127);
            LIM(tr->dep_src, 0, SS_TRACKS);
            LIM(tr->dep_mode, 0, 2);
            LIM(tr->tshift, -8, 8);
            LIM(tr->eu_hits, 0, tr->len);
            LIM(tr->eu_rot, 0, tr->len - 1);
            LIM(tr->eu_prob, 0, 3);
            LIM(tr->eu_acc, 0, 4);
            LIM(tr->bits, 0, 127);
            LIM(tr->rate, 0, 127);
            for (i = 0; i < 8; i++)
                LIM(tr->p[i], 0, 127);
            for (i = 0; i < SS_STEPS; i++) {
                int k;
                LIM(tr->step[i].note, -48, 48);
                LIM(tr->step[i].shift, -8, 8);
                for (k = 0; k < 4; k++)
                    LIM(tr->step[i].lock[k], 0, 128);
            }
        }
    }
#undef LIM
}

uint32_t ss_project_get(void *dst, uint32_t max)
{
    uint32_t i;
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)&R.prj;
    if (max < sizeof R.prj)
        return 0;
    R.prj.magic = SS_MAGIC;
    R.prj.version = SS_VERSION;
    R.prj.size = (uint16_t)sizeof R.prj;
    R.prj.sum = prj_sum(&R.prj);
    for (i = 0; i < sizeof R.prj; i++)
        d[i] = s[i];
    return (uint32_t)sizeof R.prj;
}

int ss_project_set(const void *src, uint32_t len)
{
    static ss_project_t in;           /* (static: not on the stack of a small device) */
    uint32_t i;
    uint8_t *d = (uint8_t *)&in;
    const uint8_t *s = (const uint8_t *)src;
    if (len != sizeof in)
        return -1;
    for (i = 0; i < sizeof in; i++)
        d[i] = s[i];
    if (in.magic != SS_MAGIC || in.version != SS_VERSION || in.size != sizeof in || in.sum != prj_sum(&in))
        return -2;
    prj_sanitize(&in);
    R.playing = 0;
    R.pending_pat = 0xFFu;
    R.prj = in;
    R.unsaved = 0;
    R.undo_ok = 0;
    R.dirty = ~0u;
    return 0;
}

ss_project_t *ss_project(void) { return &R.prj; }

int ss_save_wanted(void)
{
    int w = R.save_wanted;
    R.save_wanted = 0;
    if (w)
        R.unsaved = 0;
    return w;
}

void ss_init(uint32_t seed)
{
    uint32_t i;
    uint8_t *z = (uint8_t *)&R;
    int a, t;
    for (i = 0; i < sizeof R; i++)
        z[i] = 0;
    sin_init();
    ss_panic();
    R.rng = seed | 1u;
    R.ui_rng = (seed * 2654435761u) | 1u;
    R.pending_pat = 0xFFu;
    R.solo = -1;
    R.hat.lfsr = R.piip.lfsr = 1u;
    R.prj.bpm = 145;
    R.prj.swing = 50;
    R.prj.seko = 40;
    R.prj.duck = 100;
    R.prj.sc_src = SS_POTKU;
    R.prj.sc_rel = 50;
    R.prj.root = 30;                    /* F#1 */
    R.prj.scale = 2;                    /* phrygian dominant */
    R.prj.dly_time = 2;
    R.prj.dly_fb = 60;
    R.prj.rev_size = 60;
    R.prj.rev_damp = 60;
    R.prj.clock_out = 1;
    R.prj.drive = 30;
    for (a = SS_PATTERNS - 1; a >= 0; a--) {   /* four grooves; A is the one playing */
        static const int8_t KEY[SS_PATTERNS] = {0, 0, 3, -2};   /* with CHAIN on, C and D change the key */
        R.prj.cur_pat = (uint8_t)a;
        pattern_defaults(PAT);
        PAT->xpose = KEY[a];
        for (t = 0; t < SS_TRACKS; t++)
            gen_track(t);
    }
    R.dirty = ~0u;
}

/* ---------------------------------------------------------------- canvas --- */
#define RGB(r, g, b) ((uint16_t)((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)))
#define C_BG 0x0000u
#define C_FG RGB(228, 224, 212)         /* warm white: what is set */
#define C_MID RGB(150, 147, 138)
#define C_DIM RGB(92, 90, 84)
#define C_LINE RGB(44, 43, 40)          /* empty steps, rules */
#define C_ACC RGB(170, 255, 60)         /* acid green: what is selected, what the page says */
/* a colour a track: kick red, bass orange, hats yellow, acid green, the chip blue, the weird one pink */
static const uint16_t TRACK_COL[SS_TRACKS] = {RGB(255, 80, 60), RGB(255, 168, 40), RGB(240, 236, 90), RGB(110, 255, 80),
                                              RGB(70, 200, 255), RGB(255, 90, 220)};
static uint16_t shade(uint16_t c, int q)   /* q of four parts darker */
{
    uint32_t r = (c >> 11) * (uint32_t)(4 - q) / 4u, g = (c >> 5 & 63u) * (uint32_t)(4 - q) / 4u, b = (c & 31u) * (uint32_t)(4 - q) / 4u;
    return (uint16_t)(r << 11 | g << 5 | b);
}

static uint16_t *cv;
static int cv_y0;

static void rect(int x, int y, int w, int h, uint16_t c)
{
    int xx, yy, x1 = x + w, y1 = y + h;
    y -= cv_y0;
    y1 -= cv_y0;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x1 > SS_W) x1 = SS_W;
    if (y1 > SS_STRIP_H) y1 = SS_STRIP_H;
    for (yy = y; yy < y1; yy++)
        for (xx = x; xx < x1; xx++)
            cv[yy * SS_W + xx] = c;
}

/* 5 x 7, columns, bit 0 at the top: 0x20 .. 0x5F, then a l m o r */
static const uint8_t FONT[69][5] = {
    {0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00},{0x00,0x07,0x00,0x07,0x00},{0x14,0x7F,0x14,0x7F,0x14},
    {0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,0x08,0x64,0x62},{0x36,0x49,0x55,0x22,0x50},{0x00,0x05,0x03,0x00,0x00},
    {0x00,0x1C,0x22,0x41,0x00},{0x00,0x41,0x22,0x1C,0x00},{0x14,0x08,0x3E,0x08,0x14},{0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x50,0x30,0x00,0x00},{0x08,0x08,0x08,0x08,0x08},{0x00,0x60,0x60,0x00,0x00},{0x20,0x10,0x08,0x04,0x02},
    {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},{0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E},{0x00,0x36,0x36,0x00,0x00},{0x00,0x56,0x36,0x00,0x00},
    {0x08,0x14,0x22,0x41,0x00},{0x14,0x14,0x14,0x14,0x14},{0x00,0x41,0x22,0x14,0x08},{0x02,0x01,0x51,0x09,0x06},
    {0x32,0x49,0x79,0x41,0x3E},{0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},{0x3E,0x41,0x49,0x49,0x7A},
    {0x7F,0x08,0x08,0x08,0x7F},{0x00,0x41,0x7F,0x41,0x00},{0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},{0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},{0x46,0x49,0x49,0x49,0x31},
    {0x01,0x01,0x7F,0x01,0x01},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},
    {0x63,0x14,0x08,0x14,0x63},{0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},{0x00,0x7F,0x41,0x41,0x00},
    {0x02,0x04,0x08,0x10,0x20},{0x00,0x41,0x41,0x7F,0x00},{0x04,0x02,0x01,0x02,0x04},{0x40,0x40,0x40,0x40,0x40},
    {0x20,0x54,0x54,0x54,0x78},{0x00,0x41,0x7F,0x40,0x00},{0x7C,0x04,0x18,0x04,0x78},{0x38,0x44,0x44,0x44,0x38},
    {0x7C,0x08,0x04,0x04,0x08},
};

static int text(int x, int y, const char *s, uint16_t c, int sc)
{
    for (; *s; s++) {
        int ch = (uint8_t)*s, g, col, row;
        if (ch == 'a') g = 64;
        else if (ch == 'l') g = 65;
        else if (ch == 'm') g = 66;
        else if (ch == 'o') g = 67;
        else if (ch == 'r') g = 68;
        else {
            if (ch >= 'a' && ch <= 'z')
                ch -= 32;
            g = ch >= 0x20 && ch <= 0x5F ? ch - 0x20 : '?' - 0x20;
        }
        for (col = 0; col < 5; col++)
            for (row = 0; row < 7; row++)
                if (FONT[g][col] >> row & 1)
                    rect(x + col * sc, y + row * sc, sc, sc, c);
        x += 6 * sc;
    }
    return x;
}
static int text_w(const char *s, int sc)
{
    int n = 0;
    while (s[n])
        n++;
    return n ? n * 6 * sc - sc : 0;
}

static void cat(char *d, const char *s)
{
    while (*d)
        d++;
    while (*s)
        *d++ = *s++;
    *d = 0;
}
static void cat_int(char *d, int v)
{
    char t[12];
    int n = 0;
    uint32_t u = (uint32_t)(v < 0 ? -v : v);
    while (*d)
        d++;
    do
        t[n++] = (char)('0' + u % 10u);
    while ((u /= 10u) && n < 11);
    if (v < 0)
        *d++ = '-';
    while (n)
        *d++ = t[--n];
    *d = 0;
}

/* ------------------------------------------------------------ parameters --- */
enum { F_NUM, F_BPM, F_PCT, F_PAN, F_ROOT, F_SCALE, F_DTIME, F_ONOFF, F_WAVE, F_ARP, F_ORATIO, F_BIPOLAR, F_OFFNUM, F_CHAIN,
       F_EPROB, F_EACC, F_TRACK, F_DEPSRC, F_DEPMODE, F_SHIFT, F_XPOSE };
typedef struct { const char *name; uint8_t *v; uint8_t lo, hi, fmt, big; } param_t;

static const char *const TRACK_NAME[SS_TRACKS] = {"POTKU", "BASSO", "HATTU", "HAPPO", "PIIP", "OUTO"};
static const char *const PAGE_NAME[SS_NPAGES] = {"GROOVE", "SOUND", "MIX", "FX", "GLOBAL", "EUCLID", "SIDECHAIN", "LINK", "TIMING",
                                                 "SOUND 2", "8-BIT", "GLOBAL 2"};
/* K1 .. K4 on the pages SOUND and SOUND 2 (0: the knob is free) */
static const char *const SOUND_NAME[SS_TRACKS][8] = {
    {"TUNE", "DECAY", "SWEEP", "DRIVE", "SNAP", "CLICK", "HOLD", 0},
    {"CUTOFF", "RESO", "ENVMOD", "DECAY", "WAVE", "SUB", "GLIDE", "DRIVE"},
    {"DECAY", "TONE", "METAL", "OPEN", "CHIP", "SHAKE", 0, 0},
    {"CUTOFF", "RESO", "ENVMOD", "DECAY", "WAVE", "ACCENT", "GLIDE", "DRIVE"},
    {"WAVE", "DECAY", "ARP", "VIBRATO", "SPEED", "PWM", "BEND", "DETUNE"},
    {"DECAY", "FM", "RATIO", "ZAP", "ZTIME", "FEEDBK", "WOBBLE", "RANDOM"},
};
static const char *const NOTE_NAME[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

static int page_params(param_t p[4])
{
    ss_track_t *tr = &PAT->tr[R.track];
    int i;
    for (i = 0; i < 4; i++) {
        p[i].name = 0;
        p[i].v = 0;
        p[i].lo = 0;
        p[i].hi = 127;
        p[i].fmt = F_NUM;
        p[i].big = 1;
    }
#define P(i, n, ptr, l, h, f, b) (p[i].name = (n), p[i].v = (ptr), p[i].lo = (l), p[i].hi = (h), p[i].fmt = (f), p[i].big = (b))
    switch (R.page) {
    case SS_PG_HOME:
        P(0, "TEMPO", &R.prj.bpm, 120, 180, F_BPM, 0);
        P(1, "SWING", &R.prj.swing, 50, 75, F_PCT, 0);
        P(2, "SEKO", &R.prj.seko, 0, 127, F_NUM, 1);
        P(3, "DUCK", &R.prj.duck, 0, 127, F_NUM, 1);
        break;
    case SS_PG_SOUND:
    case SS_PG_SOUND2: {
        int o = R.page == SS_PG_SOUND2 ? 4 : 0;
        for (i = 0; i < 4; i++)
            if (SOUND_NAME[R.track][o + i])
                P(i, SOUND_NAME[R.track][o + i], &tr->p[o + i], 0, 127, F_NUM, 1);
        if (R.track == SS_PIIP && !o) {
            p[0].fmt = F_WAVE;
            p[2].fmt = F_ARP;
        }
        if (R.track == SS_PIIP && o)
            p[2].fmt = F_BIPOLAR;
        if (R.track == SS_OUTO && !o) {
            p[2].fmt = F_ORATIO;
            p[3].fmt = F_BIPOLAR;
        }
        break;
    }
    case SS_PG_BITS:
        P(0, "BITS", &tr->bits, 0, 127, F_OFFNUM, 1);
        P(1, "RATE", &tr->rate, 0, 127, F_OFFNUM, 1);
        break;
    case SS_PG_GLOBAL2:                 /* (a signed value: F_XPOSE is handled apart, -12..12) */
        P(0, "XPOSE", (uint8_t *)&PAT->xpose, 0, 24, F_XPOSE, 0);
        P(1, "DRIVE", &R.prj.drive, 0, 127, F_NUM, 1);
        break;
    case SS_PG_MIX:
        P(0, "LEVEL", &tr->level, 0, 127, F_NUM, 1);
        P(1, "PAN", &tr->pan, 0, 127, F_PAN, 1);
        P(2, "DELAY", &tr->dly, 0, 127, F_NUM, 1);
        P(3, "REVERB", &tr->rev, 0, 127, F_NUM, 1);
        break;
    case SS_PG_FX:
        P(0, "D.TIME", &R.prj.dly_time, 0, 2, F_DTIME, 0);
        P(1, "D.FDBK", &R.prj.dly_fb, 0, 127, F_NUM, 1);
        P(2, "R.SIZE", &R.prj.rev_size, 0, 127, F_NUM, 1);
        P(3, "R.DAMP", &R.prj.rev_damp, 0, 127, F_NUM, 1);
        break;
    case SS_PG_EUCLID:
        P(0, "HITS", &tr->eu_hits, 0, tr->len, F_NUM, 0);
        P(1, "ROTATE", &tr->eu_rot, 0, (uint8_t)(tr->len - 1u), F_NUM, 0);
        P(2, "CHANCE", &tr->eu_prob, 0, 3, F_EPROB, 0);
        P(3, "ACCENT", &tr->eu_acc, 0, 4, F_EACC, 0);
        break;
    case SS_PG_TIMING: {              /* (signed values: F_SHIFT is handled apart, -8..8) */
        static char name[10];
        name[0] = 0;
        cat(name, "STEP ");
        cat_int(name, R.cursor + 1);
        P(0, name, (uint8_t *)&tr->step[R.cursor & 15u].shift, 0, 16, F_SHIFT, 0);
        P(1, "TRACK", (uint8_t *)&tr->tshift, 0, 16, F_SHIFT, 0);
        break;
    }
    case SS_PG_LINK:
        P(0, "SOURCE", &tr->dep_src, 0, SS_TRACKS, F_DEPSRC, 0);
        P(1, "MODE", &tr->dep_mode, 0, 2, F_DEPMODE, 0);
        break;
    case SS_PG_SC:
        P(0, "SOURCE", &R.prj.sc_src, 0, SS_TRACKS - 1, F_TRACK, 0);
        P(1, "DEPTH", &R.prj.duck, 0, 127, F_NUM, 1);
        P(2, "RELEASE", &R.prj.sc_rel, 0, 127, F_NUM, 1);
        P(3, "AMOUNT", &tr->sc, 0, 127, F_NUM, 1);
        break;
    default:
        P(0, "ROOT", &R.prj.root, 24, 47, F_ROOT, 0);
        P(1, "SCALE", &R.prj.scale, 0, SS_NSCALES - 1, F_SCALE, 0);
        P(2, "CLOCK", &R.prj.clock_out, 0, 1, F_ONOFF, 0);
        P(3, "CHAIN", &R.prj.chain, 0, 3, F_CHAIN, 0);
        break;
    }
#undef P
    return 4;
}

static void param_text(const param_t *p, char *t)
{
    static const char *const SCALE[SS_NSCALES] = {"MINOR", "PHRYG", "PHRDOM", "HARMIN", "DORIAN", "MAJOR", "PENTA", "WHOLE"};
    static const char *const WAVE[6] = {"P12", "P25", "P50", "TRI", "SAW", "NOISE"};
    static const char *const ARP[7] = {"OFF", "OCT", "MIN", "MAJ", "SUS4", "5+8", "DIM"};
    static const char *const DTIME[3] = {"1/16", "1/8", "3/16"};
    static const char *const CHAIN[4] = {"OFF", "A-B", "A-C", "A-D"};
    static const char *const EPROB[4] = {"25%", "50%", "75%", "100%"};
    static const char *const EACC[5] = {"OFF", "ALL", "2ND", "3RD", "4TH"};
    static const char *const DEPMODE[3] = {"WITH", "GAPS", "AFTER"};
    int v = *p->v;
    t[0] = 0;
    switch (p->fmt) {
    case F_PCT: cat_int(t, v); cat(t, "%"); break;
    case F_PAN:
        if (v == 64) cat(t, "C");
        else { cat(t, v < 64 ? "L" : "R"); cat_int(t, v < 64 ? 64 - v : v - 64); }
        break;
    case F_ROOT: cat(t, NOTE_NAME[v % 12]); cat_int(t, v / 12 - 1); break;
    case F_SCALE: cat(t, SCALE[v < SS_NSCALES ? v : 0]); break;
    case F_DTIME: cat(t, DTIME[v > 2 ? 2 : v]); break;
    case F_ONOFF: cat(t, v ? "ON" : "OFF"); break;
    case F_WAVE: cat(t, WAVE[v * 6 / 128]); break;
    case F_ARP: cat(t, ARP[v * 7 / 128]); break;
    case F_ORATIO: cat(t, "R"); cat_int(t, 1 + (v >> 4)); break;
    case F_BIPOLAR:                     /* 64 is the middle */
        if (v > 64)
            cat(t, "+");
        cat_int(t, v - 64);
        break;
    case F_OFFNUM:
        if (v)
            cat_int(t, v);
        else
            cat(t, "OFF");
        break;
    case F_CHAIN: cat(t, CHAIN[v & 3]); break;
    case F_EPROB: cat(t, EPROB[v & 3]); break;
    case F_EACC: cat(t, EACC[v > 4 ? 4 : v]); break;
    case F_TRACK: cat(t, TRACK_NAME[v % SS_TRACKS]); break;
    case F_DEPSRC: cat(t, v && v <= SS_TRACKS ? TRACK_NAME[v - 1] : "OFF"); break;
    case F_DEPMODE: cat(t, DEPMODE[v > 2 ? 2 : v]); break;
    case F_SHIFT:                       /* - early, + late */
    case F_XPOSE:
        v = (int8_t)*p->v;
        if (v > 0)
            cat(t, "+");
        cat_int(t, v);
        break;
    default: cat_int(t, v); break;
    }
}

/* ------------------------------------------------------------------ draw --- */
#define GRID_Y 25
#define LANE_H 20
#define CELL_X 46
#define CELL_W 12
#define PARAM_Y 152
#define FOOT_Y 214

static int held_step(void)              /* the lowest held white key in step mode: the step being edited, or -1 */
{
    static const uint8_t WK[16] = {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26};
    int i;
    if (R.keys_mode || R.page == SS_PG_TIMING)
        return -1;
    for (i = 0; i < 16; i++)
        if (R.keys >> WK[i] & 1u)
            return i;
    return -1;
}

void ss_draw(uint16_t *px, int strip)
{
    char t[32];
    int i, lane, hs = held_step();
    param_t pp[4];
    cv = px;
    cv_y0 = strip * SS_STRIP_H;
    rect(0, cv_y0, SS_W, SS_STRIP_H, C_BG);

    /* header: the name, the pattern, the tempo, the transport */
    i = text(3, 5, "SUOM", C_FG, 2);
    i = text(i, 5, "1", C_ACC, 2);
    text(i, 5, "SAUNDI", C_FG, 2);
    t[0] = (char)('A' + R.prj.cur_pat);
    t[1] = 0;
    text(140, 5, t, R.pending_pat != 0xFFu ? C_DIM : C_FG, 2);
    if (R.pending_pat != 0xFFu) {
        t[0] = (char)('A' + R.pending_pat);
        text(153, 5, ">", C_DIM, 1);
        text(160, 5, t, C_ACC, 1);
    }
    t[0] = 0;
    cat_int(t, R.prj.bpm);
    text(176, 5, t, C_FG, 2);
    if (R.playing) {
        for (i = 0; i < 7; i++)         /* a triangle */
            rect(220 + i, 5 + i, 1, 14 - 2 * i, C_ACC);
    } else {
        rect(220, 7, 10, 10, C_DIM);
    }
    rect(0, 23, SS_W, 1, C_LINE);

    /* the grid: a lane a track, each as long as its own loop */
    for (lane = 0; lane < SS_TRACKS; lane++) {
        const ss_track_t *tr = &PAT->tr[lane];
        int y = GRID_Y + lane * LANE_H, cur = lane == R.track;
        int quiet = tr->mute || (R.solo >= 0 && R.solo != lane);
        if (cur)
            rect(0, y, 42, 17, R.solo == lane ? C_FG : TRACK_COL[lane]);
        text(3, y + 5, TRACK_NAME[lane], cur ? C_BG : R.solo == lane ? C_FG : quiet ? C_DIM : shade(TRACK_COL[lane], 1), 1);
        if (tr->mute)
            rect(2, y + 8, text_w(TRACK_NAME[lane], 1) + 2, 1, cur ? C_BG : C_DIM);
        if (tr->dep_src && tr->dep_src - 1 != lane)
            rect(37, y + 7, 3, 3, C_ACC);   /* linked to another track */
        for (i = 0; i < SS_STEPS; i++) {
            int x = CELL_X + i * CELL_W, on, play;
            const ss_step_t *sp = &tr->step[i];
            uint8_t f = sp->flags;
            uint16_t c;
            if (i >= tr->len) {
                rect(x + 4, y + 8, 2, 2, C_LINE);
                continue;
            }
            on = (f & SS_ST_ON) != 0;
            play = R.playing && R.pos[lane] == i;
            c = quiet ? C_DIM : shade(TRACK_COL[lane], (int)SS_ST_PROB(f) * 3 / 4 + (SS_ST_PROB(f) != 0));   /* less likely, darker */
            if (play && on && !quiet)
                c = C_FG;
            if (on)                     /* microtiming: the cell sits a pixel or two off its place */
                x += sp->shift / 3;
            if (!on)
                rect(x, y + 6, 10, 10, (i & 3) == 0 ? RGB(60, 58, 54) : C_LINE);
            else if (f & SS_ST_ACC)
                rect(x, y, 10, 16, c);  /* an accent stands taller */
            else
                rect(x, y + 6, 10, 10, c);
            if (on && (sp->lock[0] | sp->lock[1] | sp->lock[2] | sp->lock[3]))
                rect(x + 3, y + 12, 4, 3, C_BG);    /* its own sound */
            if (on && (f & SS_ST_SLIDE))
                rect(x - 2, y + 10, 2, 2, c);       /* a slide joins it to the step before */
            if (on) {                   /* a ratchet splits its step */
                int n = (int)SS_ST_RATCH(f), j;
                for (j = 1; j < n; j++)
                    rect(x + 10 * j / n, y + 6, 1, 10, C_BG);
            }
            if (play)
                rect(x, y + 17, 10, 2, C_FG);
            if (cur && (hs == i || (R.page == SS_PG_TIMING && R.cursor == i)))
                rect(x, y + 17, 10, 2, C_FG);
        }
    }
    rect(0, 147, SS_W, 1, C_LINE);

    /* the page: what K1 .. K4 turn */
    t[0] = 0;
    cat(t, PAGE_NAME[R.page]);
    if (R.page == SS_PG_SOUND || R.page == SS_PG_MIX || R.page == SS_PG_EUCLID || R.page == SS_PG_SC ||
        R.page == SS_PG_LINK || R.page == SS_PG_TIMING || R.page == SS_PG_SOUND2 || R.page == SS_PG_BITS) {
        cat(t, " ");
        cat(t, TRACK_NAME[R.track]);
    }
    text(4, PARAM_Y, t, C_ACC, 1);
    if (hs >= 0 && R.page == SS_PG_SOUND) {   /* a step held on the sound page: its own values (locks) */
        const ss_step_t *st = &PAT->tr[R.track].step[hs];
        t[0] = 0;
        cat(t, "STEP ");
        cat_int(t, hs + 1);
        cat(t, " LOCKS");
        text(236 - text_w(t, 1), PARAM_Y, t, C_FG, 1);
        for (i = 0; i < 4; i++) {
            int x = 4 + i * 60, v = st->lock[i] ? st->lock[i] - 1 : PAT->tr[R.track].p[i];
            text(x, PARAM_Y + 14, SOUND_NAME[R.track][i], st->lock[i] ? C_ACC : C_MID, 1);
            t[0] = 0;
            cat_int(t, v);
            text(x, PARAM_Y + 26, t, st->lock[i] ? C_FG : C_DIM, 2);
            rect(x, PARAM_Y + 46, 52, 3, C_LINE);
            rect(x, PARAM_Y + 46, v * 52 / 127, 3, st->lock[i] ? C_ACC : C_DIM);
        }
    } else if (hs >= 0) {               /* a step is held: the knobs edit it */
        const ss_step_t *st = &PAT->tr[R.track].step[hs];
        static const char *const PROB[4] = {"100%", "75%", "50%", "25%"};
        t[0] = 0;
        cat(t, "STEP ");
        cat_int(t, hs + 1);
        cat(t, st->flags & SS_ST_SLIDE ? " SLIDE" : "");
        cat(t, st->shift < 0 ? " EARLY " : st->shift > 0 ? " LATE " : "");
        if (st->shift)
            cat_int(t, st->shift < 0 ? -st->shift : st->shift);
        text(236 - text_w(t, 1), PARAM_Y, t, C_FG, 1);
        text(4, PARAM_Y + 14, "NOTE", C_MID, 1);
        t[0] = 0;
        if (st->note > 0)
            cat(t, "+");
        cat_int(t, st->note);
        text(4, PARAM_Y + 26, t, C_FG, 2);
        text(64, PARAM_Y + 14, "CHANCE", C_MID, 1);
        text(64, PARAM_Y + 26, PROB[SS_ST_PROB(st->flags)], C_FG, 2);
        text(124, PARAM_Y + 14, "ACC/SLIDE", C_MID, 1);
        {
            static const char *const AS[4] = {"OFF", "ACC", "SLD", "A+S"};
            text(124, PARAM_Y + 26, AS[((st->flags & SS_ST_ACC) ? 1 : 0) | ((st->flags & SS_ST_SLIDE) ? 2 : 0)], C_FG, 2);
        }
        text(184, PARAM_Y + 14, "RATCHET", C_MID, 1);
        t[0] = 0;
        cat(t, "X");
        cat_int(t, (int)SS_ST_RATCH(st->flags));
        text(184, PARAM_Y + 26, t, C_FG, 2);
    } else {
        page_params(pp);
        for (i = 0; i < 4; i++) {
            int x = 4 + i * 60;
            if (!pp[i].name)
                continue;
            text(x, PARAM_Y + 14, pp[i].name, C_MID, 1);
            param_text(&pp[i], t);
            text(x, PARAM_Y + 26, t, C_FG, text_w(t, 2) > 54 ? 1 : 2);
            rect(x, PARAM_Y + 46, 52, 3, C_LINE);
            if (pp[i].fmt == F_SHIFT || pp[i].fmt == F_XPOSE) {   /* from the middle: left early, right late */
                int v = (int8_t)*pp[i].v, full = pp[i].fmt == F_SHIFT ? 8 : 12;
                rect(x + 25, PARAM_Y + 44, 2, 7, C_MID);
                if (v < 0)
                    rect(x + 26 + v * 26 / full, PARAM_Y + 46, -v * 26 / full, 3, C_ACC);
                else
                    rect(x + 26, PARAM_Y + 46, v * 26 / full, 3, C_ACC);
            } else if (pp[i].hi > pp[i].lo)
                rect(x, PARAM_Y + 46, (*pp[i].v - pp[i].lo) * 52 / (pp[i].hi - pp[i].lo), 3, C_FG);
        }
    }
    rect(0, FOOT_Y - 4, SS_W, 1, C_LINE);

    /* the foot: a message for a moment, else where we are */
    if (R.msg_until) {
        i = text(4, FOOT_Y + 2, R.msg[0], C_ACC, 2);
        text(i + 6, FOOT_Y + 2, R.msg[1], C_FG, 2);
    } else {
        t[0] = 0;
        cat(t, TRACK_NAME[R.track]);
        cat(t, " LEN ");
        cat_int(t, PAT->tr[R.track].len);
        text(4, FOOT_Y, t, C_FG, 1);
        text(236 - text_w(R.keys_mode ? "KEYS" : "STEPS", 1), FOOT_Y, R.keys_mode ? "KEYS" : "STEPS",
             R.keys_mode ? C_ACC : C_MID, 1);
        if (R.unsaved)
            text(150, FOOT_Y, "UNSAVED", C_DIM, 1);
        text(4, FOOT_Y + 13, R.keys_mode ? "KEYS PLAY THE TRACK  REC: STEPS" :
             R.page == SS_PG_TIMING ? "SELECT: NEXT HIT  KEY: PICK STEP" : "SEL NEW  ARP MUTATE  LFO MUTE/SOLO",
             C_DIM, 1);
    }
}

/* ----------------------------------------------------------------- input --- */
#define STRIP_ALL 0x1Fu
#define STRIP_GRID 0x07u                /* rows 0 .. 143: header and grid */
#define STRIP_PAGE 0x18u                /* rows 144 .. 239 */

void ss_message(const char *a, const char *b)
{
    int i;
    for (i = 0; i < 19 && a && a[i]; i++)
        R.msg[0][i] = a[i];
    R.msg[0][i] = 0;
    for (i = 0; i < 19 && b && b[i]; i++)
        R.msg[1][i] = b[i];
    R.msg[1][i] = 0;
    R.msg_until = R.now + 1100u;
    if (!R.msg_until)
        R.msg_until = 1;
    R.dirty |= STRIP_PAGE;
}

static void changed(void)
{
    R.unsaved = 1;
    R.dirty |= STRIP_ALL;
}
static void keep_undo(void)
{
    R.undo = *PAT;
    R.undo_pat = R.prj.cur_pat;
    R.undo_ok = 1;
}
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void ss_input(uint32_t buttons, uint32_t keys, const int32_t enc[SS_NE], uint32_t now_ms)
{
    static const uint8_t WK[16] = {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26};
    static const uint8_t BK[11] = {1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25};
    uint32_t pressed = buttons & ~R.btn, kdown = keys & ~R.keys, kup = R.keys & ~keys;
    ss_track_t *tr;
    int i, hs, e_sel = (int)enc[SS_E_SELECT], e_pre = (int)enc[SS_E_PRESET];
    R.now = now_ms;
    hs = held_step();                   /* from the keys as they were: a step held since before this call */
    R.btn = buttons;
    R.keys = keys;

    if (R.msg_until && (int32_t)(now_ms - R.msg_until) >= 0) {
        R.msg_until = 0;
        R.dirty |= STRIP_PAGE;
    }
    if (R.gstep_ui != R.seen_gstep) {   /* the playhead moved */
        R.seen_gstep = R.gstep_ui;
        R.dirty |= STRIP_GRID;
    }

    /* transport and pages */
    if (pressed & 1u << SS_B_PLAY) {
        if (R.playing)
            R.want_stop = 1;
        else
            R.want_play = 1;
        R.dirty |= STRIP_ALL;
    }
    {
        static const uint8_t PAGE_BTN[SS_NPAGES] = {SS_B_HOME, SS_B_EDIT, SS_B_ENV, SS_B_FX, SS_B_GLO,
                                                  SS_B_SEQ, SS_B_FX, SS_B_SEQ, SS_B_SEQ, SS_B_EDIT, SS_B_ENV, SS_B_GLO};
        uint8_t was = R.page;
        for (i = 0; i < SS_PG_EUCLID; i++)
            if (pressed & 1u << PAGE_BTN[i]) {
                R.page = (uint8_t)i;
                R.dirty |= STRIP_ALL;
            }
        if ((pressed & 1u << SS_B_FX) && was == SS_PG_FX)
            R.page = SS_PG_SC;          /* FX again: the sidechain */
        if ((pressed & 1u << SS_B_EDIT) && was == SS_PG_SOUND)
            R.page = SS_PG_SOUND2;      /* EDIT again: the sound's other four knobs */
        if ((pressed & 1u << SS_B_ENV) && was == SS_PG_MIX)
            R.page = SS_PG_BITS;        /* ENV again: the track's 8-bit crusher */
        if ((pressed & 1u << SS_B_GLO) && was == SS_PG_GLOBAL)
            R.page = SS_PG_GLOBAL2;     /* GLO again: the pattern's key, the drive */
    }
    if (pressed & 1u << SS_B_SEQ) {   /* back to the steps, and the page that spreads them */
        R.keys_mode = 0;
        /* SEQ again: the link to another track; and again: the timing of single hits and of the track */
        R.page = R.page == SS_PG_EUCLID ? SS_PG_LINK : R.page == SS_PG_LINK ? SS_PG_TIMING : SS_PG_EUCLID;
        R.eu_armed = 1;
        R.dirty |= STRIP_ALL;
    }
    if (pressed & 1u << SS_B_REC) {
        R.keys_mode ^= 1u;
        R.dirty |= STRIP_ALL;
        ss_message("KEYS", R.keys_mode ? "PLAY" : "STEPS");
    }
    /* SAVE: a tap saves. Held, PRESETS picks a pattern to copy this whole pattern to, SELECT one to copy this
     * track to; letting go copies */
    if (pressed & 1u << SS_B_SAVE) {
        R.save_held = 1;
        R.copy_kind = 0;
        R.copy_dst = R.prj.cur_pat;
    }
    if (R.save_held) {
        char a[16], b[8];
        int d = e_pre ? e_pre : e_sel;
        if (d) {
            R.copy_kind = e_pre ? 1 : 2;
            R.copy_dst = (uint8_t)clampi((int)R.copy_dst + d, 0, SS_PATTERNS - 1);
        }
        e_pre = e_sel = 0;
        a[0] = 0;
        cat(a, "COPY ");
        cat(a, R.copy_kind == 1 ? "ALL" : TRACK_NAME[R.track]);
        b[0] = (char)('A' + R.prj.cur_pat);
        b[1] = '>';
        b[2] = R.copy_dst == R.prj.cur_pat ? '-' : (char)('A' + R.copy_dst);
        b[3] = 0;
        if (!(buttons & 1u << SS_B_SAVE)) {
            R.save_held = 0;
            if (!R.copy_kind) {
                R.save_wanted = 1;
                ss_message("SAVED", "");
            } else if (R.copy_dst != R.prj.cur_pat) {
                R.undo = R.prj.pat[R.copy_dst];     /* UNDO in the pattern copied to brings it back */
                R.undo_pat = R.copy_dst;
                R.undo_ok = 1;
                if (R.copy_kind == 1)
                    R.prj.pat[R.copy_dst] = *PAT;
                else
                    R.prj.pat[R.copy_dst].tr[R.track] = PAT->tr[R.track];
                a[2] = 'P'; a[3] = 'I'; a[4] = 'E'; a[5] = 'D'; a[6] = 0;   /* "COPIED" */
                ss_message(a, b);
                changed();
            } else {
                ss_message("COPY", "OFF");
            }
            R.dirty |= STRIP_ALL;
        } else if (R.copy_kind && (d || !R.msg_until)) {
            ss_message(a, b);
        } else if (R.copy_kind) {
            R.msg_until = now_ms + 1100u ? now_ms + 1100u : 1u;   /* stays up while SAVE is held */
        }
    }

    if (hs >= 0 && e_sel) {             /* a step held: SELECT nudges it early or late */
        ss_step_t *st = &PAT->tr[R.track].step[hs];
        st->shift = (int8_t)clampi(st->shift + e_sel, -8, 8);
        st->flags |= SS_ST_ON;
        R.step_edited = 1;
        e_sel = 0;
        changed();
    }

    if (R.page == SS_PG_TIMING && !R.keys_mode && e_sel) {   /* SELECT walks the cursor from hit to hit */
        const ss_track_t *ct = &PAT->tr[R.track];
        int n = e_sel < 0 ? -e_sel : e_sel, dir = e_sel < 0 ? -1 : 1, len = ct->len ? ct->len : 1, k;
        while (n--)
            for (k = 1; k <= len; k++) {
                int c = ((int)R.cursor + dir * k % len + len) % len;
                if (ct->step[c].flags & SS_ST_ON) {
                    R.cursor = (uint8_t)c;
                    break;
                }
            }
        e_sel = 0;
        R.dirty |= STRIP_ALL;
    }

    /* the track */
    i = e_sel;
    if (R.keys_mode) {
        int o = R.octave + !!(pressed & 1u << SS_B_OCTUP) - !!(pressed & 1u << SS_B_OCTDN);
        R.octave = (int8_t)clampi(o, -2, 2);
    } else {
        i += !!(pressed & 1u << SS_B_OCTUP) - !!(pressed & 1u << SS_B_OCTDN);
    }
    if (i) {
        R.track = (uint8_t)clampi((int)R.track + i, 0, SS_TRACKS - 1);
        R.eu_armed = 1;
        R.dirty |= STRIP_ALL;
    }
    tr = &PAT->tr[R.track];
    if (R.cursor >= tr->len)
        R.cursor = (uint8_t)(tr->len ? tr->len - 1u : 0u);

    if (hs >= 0 && enc[SS_E_ALGO]) {    /* a step held: ALGORITHM slides into it, or not */
        ss_step_t *st = &tr->step[hs];
        st->flags = (uint8_t)(enc[SS_E_ALGO] > 0 ? st->flags | SS_ST_SLIDE : st->flags & ~SS_ST_SLIDE);
        st->flags |= SS_ST_ON;
        R.step_edited = 1;
        changed();
    } else if (enc[SS_E_ALGO]) {        /* the loop's length */
        tr->len = (uint8_t)clampi((int)tr->len + (int)enc[SS_E_ALGO], 1, SS_STEPS);
        if (tr->eu_hits > tr->len)
            tr->eu_hits = tr->len;
        if (tr->eu_rot >= tr->len)
            tr->eu_rot = (uint8_t)(tr->len - 1u);
        if (R.page == SS_PG_EUCLID)   /* on its page the spread follows the length */
            apply_euclid(R.track);
        changed();
    }
    if (e_pre) {                        /* the pattern: at the next bar while playing */
        int cur = R.pending_pat != 0xFFu ? R.pending_pat : R.prj.cur_pat;
        int p = clampi(cur + e_pre, 0, SS_PATTERNS - 1);
        R.eu_armed = 1;
        if (R.playing)
            R.pending_pat = (uint8_t)(p == R.prj.cur_pat ? 0xFF : p);
        else
            R.prj.cur_pat = (uint8_t)p;
        R.dirty |= STRIP_ALL;
        tr = &PAT->tr[R.track];
    }

    /* the pattern's helpers */
    if (hs >= 0 && (pressed & 1u << SS_B_SEL)) {   /* a step held: SEL takes its shift and locks away */
        step_plain(&tr->step[hs]);
        R.step_edited = 1;
        ss_message("STEP", "PLAIN");
        changed();
    } else if (pressed & 1u << SS_B_SEL) {
        keep_undo();
        gen_track(R.track);
        ss_message("NEW", TRACK_NAME[R.track]);
        changed();
    }
    if (pressed & 1u << SS_B_ARP) {
        keep_undo();
        mutate_track(R.track);
        ss_message("MUTATE", TRACK_NAME[R.track]);
        changed();
    }
    if (pressed & 1u << SS_B_LFO)   /* a tap mutes the track, a hold solos it */
        R.lfo_t = now_ms ? now_ms : 1u;
    if (R.lfo_t && (buttons & 1u << SS_B_LFO) && now_ms - R.lfo_t >= 450u) {
        R.lfo_t = 0;
        R.solo = (int8_t)(R.solo == (int)R.track ? -1 : (int)R.track);
        ss_message("SOLO", R.solo < 0 ? "OFF" : TRACK_NAME[R.track]);
        R.dirty |= STRIP_ALL;
    }
    if (R.lfo_t && !(buttons & 1u << SS_B_LFO)) {
        R.lfo_t = 0;
        tr->mute ^= 1u;
        changed();
    }

    /* K1 .. K4: the held step, or the page */
    if (hs >= 0 && R.page == SS_PG_SOUND) {   /* the step's own sound: a lock a knob */
        ss_step_t *st = &tr->step[hs];
        for (i = 0; i < 4; i++) {
            int d = (int)enc[SS_E_K1 + i], v = st->lock[i] ? st->lock[i] - 1 : tr->p[i];
            if (!d)
                continue;
            st->lock[i] = (uint8_t)(clampi(v + d * (d > 1 || d < -1 ? 4 : 2), 0, 127) + 1);
            st->flags |= SS_ST_ON;
            R.step_edited = 1;
            changed();
        }
    } else if (hs >= 0) {
        ss_step_t *st = &tr->step[hs];
        int any = 0;
        if (enc[SS_E_K1]) {
            st->note = (int8_t)clampi(st->note + (int)enc[SS_E_K1], -24, 36);
            any = 1;
        }
        if (enc[SS_E_K2]) {           /* clockwise: more likely */
            int p = clampi((int)SS_ST_PROB(st->flags) - (int)enc[SS_E_K2], 0, 3);
            st->flags = (uint8_t)((st->flags & ~0x30u) | (uint32_t)p << 4);
            any = 1;
        }
        if (enc[SS_E_K3]) {
            /* four states in turn: plain, accent, slide, accent and slide */
            int a = ((st->flags & SS_ST_ACC) ? 1 : 0) | ((st->flags & SS_ST_SLIDE) ? 2 : 0);
            a = clampi(a + (int)enc[SS_E_K3], 0, 3);
            st->flags = (uint8_t)((st->flags & ~(SS_ST_ACC | SS_ST_SLIDE)) | (a & 1 ? SS_ST_ACC : 0u) |
                                  (a & 2 ? SS_ST_SLIDE : 0u));
            any = 1;
        }
        if (enc[SS_E_K4]) {
            int n = clampi((int)(st->flags & 3u) + (int)enc[SS_E_K4], 0, 3);
            st->flags = (uint8_t)((st->flags & ~3u) | (uint32_t)n);
            any = 1;
        }
        if (any) {
            st->flags |= SS_ST_ON;    /* editing a step sets it */
            R.step_edited = 1;
            changed();
        }
    } else {
        param_t pp[4];
        page_params(pp);
        for (i = 0; i < 4; i++) {
            int d = (int)enc[SS_E_K1 + i];
            if (!d || !pp[i].v)
                continue;
            if (pp[i].big)              /* 0..127: two a detent, more when turned fast */
                d *= d > 1 || d < -1 ? 4 : 2;
            if (R.page == SS_PG_EUCLID && R.eu_armed) {
                keep_undo();            /* what the steps were before the page spread them */
                R.eu_armed = 0;
            }
            if (pp[i].fmt == F_SHIFT || pp[i].fmt == F_XPOSE)
                *pp[i].v = (uint8_t)(int8_t)clampi((int)(int8_t)*pp[i].v + d, pp[i].fmt == F_SHIFT ? -8 : -12,
                                                   pp[i].fmt == F_SHIFT ? 8 : 12);
            else
                *pp[i].v = (uint8_t)clampi((int)*pp[i].v + d, pp[i].lo, pp[i].hi);
            if (R.page == SS_PG_EUCLID)
                apply_euclid(R.track);
            changed();
        }
    }

    /* the keys */
    if (R.keys_mode) {
        for (i = 0; i < SS_NKEYS; i++)
            if (kdown >> i & 1u) {
                int note = i - 7 + 12 * R.octave;   /* key 7 is C4: the base note */
                tq_push(R.track, note, 110);
                if (R.playing) {        /* written into the step it fell on */
                    uint32_t len = tr->len ? tr->len : 1u;
                    set_step(tr, R.pos[R.track] % len, 0, 0, note);
                    changed();
                }
            }
    } else {
        for (i = 0; i < 16; i++)        /* a white key is a step: a tap toggles it */
            if (kup >> WK[i] & 1u) {
                if (R.page == SS_PG_TIMING) {   /* on the timing page a key picks the step, and leaves it as it is */
                    if (i < tr->len)
                        R.cursor = (uint8_t)i;
                    R.dirty |= STRIP_ALL;
                } else if (R.step_edited) {
                    if (!(keys & 0x7FFFFFFu))
                        R.step_edited = 0;
                } else if (i < tr->len) {
                    tr->step[i].flags ^= SS_ST_ON;
                    changed();
                }
            }
        if (kdown || kup)
            R.dirty |= STRIP_GRID | STRIP_PAGE;
        for (i = 0; i < 11; i++) {
            if (!(kdown >> BK[i] & 1u))
                continue;
            if (i < SS_TRACKS) {      /* the first six black keys: the tracks, heard as chosen */
                R.track = (uint8_t)i;
                R.eu_armed = 1;
                tr = &PAT->tr[R.track];
                if (!R.playing)
                    tq_push(i, 0, 110);
                R.dirty |= STRIP_ALL;
            } else if (i == 6) {
                int k;
                keep_undo();
                for (k = 0; k < SS_STEPS; k++) {
                    tr->step[k].flags = 0;
                    tr->step[k].note = 0;
                    step_plain(&tr->step[k]);
                }
                ss_message("CLEAR", TRACK_NAME[R.track]);
                changed();
            } else if (i == 7 || i == 8) {
                keep_undo();
                rotate_track(R.track, i == 8 ? 1 : -1);
                changed();
            } else if (i == 9) {
                int k;
                keep_undo();
                for (k = 0; k < SS_TRACKS; k++)
                    gen_track(k);
                ss_message("NEW", "GROOVE");
                changed();
            } else if (R.undo_ok && R.undo_pat == R.prj.cur_pat) {
                static ss_pattern_t now;      /* (static: not on a small stack) */
                now = *PAT;
                *PAT = R.undo;
                R.undo = now;           /* undo again: redo */
                ss_message("UNDO", "");
                changed();
            }
        }
    }
}

uint32_t ss_dirty(void)
{
    uint32_t d = R.dirty & STRIP_ALL;
    R.dirty = 0;
    return d;
}

void ss_lights(uint32_t *buttons, uint32_t *keys, uint32_t *dim_buttons, uint32_t *dim_keys)
{
    static const uint8_t WK[16] = {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26};
    static const uint8_t BK[11] = {1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25};
    static const uint8_t PAGE_BTN[SS_NPAGES] = {SS_B_HOME, SS_B_EDIT, SS_B_ENV, SS_B_FX, SS_B_GLO,
                                                  SS_B_SEQ, SS_B_FX, SS_B_SEQ, SS_B_SEQ, SS_B_EDIT, SS_B_ENV, SS_B_GLO};
    const ss_track_t *tr = &PAT->tr[R.track];
    uint32_t b = 1u << PAGE_BTN[R.page], k = 0, dk = 0;
    int i;
    if (R.playing)
        b |= 1u << SS_B_PLAY;
    if (R.keys_mode)
        b |= 1u << SS_B_REC;
    if (tr->mute || R.solo >= 0)
        b |= 1u << SS_B_LFO;
    if (R.unsaved)
        b |= 1u << SS_B_SAVE;
    if (R.keys_mode) {
        k = R.keys;
    } else {
        for (i = 0; i < 16; i++) {
            if (i >= tr->len)
                continue;
            if (tr->step[i].flags & SS_ST_ON)
                k |= 1u << WK[i];
            else
                dk |= 1u << WK[i];      /* the loop's length shows as a glow */
            if (R.playing && R.pos[R.track] == i)
                k ^= 1u << WK[i];       /* the playhead inverts its step */
        }
        for (i = 0; i < SS_TRACKS; i++) {
            if (i == R.track || R.flash[i])
                k |= 1u << BK[i];
            else if (!PAT->tr[i].mute && R.solo < 0)
                dk |= 1u << BK[i];
            R.flash[i] = 0;
        }
    }
    *buttons = b;
    *keys = k;
    *dim_buttons = ~b & ((1u << SS_NB) - 1u);
    *dim_keys = dk & ~k;
}

int ss_state(int what)
{
    const ss_track_t *tr = &PAT->tr[R.track];
    int i, n = 0;
    switch (what) {
    case SS_Q_PLAYING: return R.playing;
    case SS_Q_TRACK: return R.track;
    case SS_Q_PAGE: return R.page;
    case SS_Q_BPM: return R.prj.bpm;
    case SS_Q_GSTEP: return (int)R.gstep;
    case SS_Q_PATTERN: return R.prj.cur_pat;
    case SS_Q_KEYS_MODE: return R.keys_mode;
    case SS_Q_LEN: return tr->len;
    case SS_Q_STEPS_ON:
        for (i = 0; i < SS_STEPS; i++)
            n += (tr->step[i].flags & SS_ST_ON) != 0;
        return n;
    case SS_Q_MUTE: return tr->mute;
    case SS_Q_UNSAVED: return R.unsaved;
    case SS_Q_POS: return R.pos[R.track];
    case SS_Q_SOLO: return R.solo;
    case SS_Q_CURSOR: return R.cursor;
    case SS_Q_HITS + 0: case SS_Q_HITS + 1: case SS_Q_HITS + 2: case SS_Q_HITS + 3: case SS_Q_HITS + 4:
    case SS_Q_HITS + 5: return (int)R.hits[what - SS_Q_HITS];
    default: return -1;
    }
}
