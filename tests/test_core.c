/* SPDX-License-Identifier: MIT
 * Host tests for the suom1saundi core: tests/run_tests.sh builds and runs this. It also writes a WAV of the factory
 * groove and the screen as PPM images into the directory given as argv[1]. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "suomi.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static int32_t blk[2 * 128];
static uint32_t now_ms;
static uint32_t held_b, held_k;

static void frame(const int32_t *enc)       /* one UI frame, 16 ms of audio */
{
    static const int32_t none[SS_NE];
    int i;
    ss_input(held_b, held_k, enc ? enc : none, now_ms);
    for (i = 0; i < 6; i++)
        ss_render(blk, 128, 4096);
    now_ms += 17;
}
static void tap_button(int b) { held_b |= 1u << b; frame(0); held_b &= ~(1u << b); frame(0); }
static void tap_key(int k) { held_k |= 1u << k; frame(0); held_k &= ~(1u << k); frame(0); }
static void turn(int e, int d) { int32_t enc[SS_NE] = {0}; enc[e] = d; frame(enc); }

static void write_wav(const char *path, const int16_t *pcm, uint32_t frames)
{
    FILE *f = fopen(path, "wb");
    uint32_t n = frames * 4, v;
    uint16_t w;
    if (!f) return;
    fwrite("RIFF", 1, 4, f); v = 36 + n; fwrite(&v, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f);
    v = 16; fwrite(&v, 4, 1, f); w = 1; fwrite(&w, 2, 1, f); w = 2; fwrite(&w, 2, 1, f);
    v = SS_SR; fwrite(&v, 4, 1, f); v = SS_SR * 4; fwrite(&v, 4, 1, f); w = 4; fwrite(&w, 2, 1, f);
    w = 16; fwrite(&w, 2, 1, f); fwrite("data", 1, 4, f); fwrite(&n, 4, 1, f); fwrite(pcm, 4, frames, f);
    fclose(f);
}

static void write_screen(const char *path)
{
    static uint16_t px[SS_W * SS_STRIP_H];
    FILE *f = fopen(path, "wb");
    int s, i;
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", SS_W, SS_H);
    for (s = 0; s < SS_STRIPS; s++) {
        ss_draw(px, s);
        for (i = 0; i < SS_W * SS_STRIP_H; i++) {
            uint8_t rgb[3] = {(uint8_t)((px[i] >> 11) << 3), (uint8_t)((px[i] >> 5 & 63) << 2), (uint8_t)((px[i] & 31) << 3)};
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
}

/* render seconds of the playing groove: peak, RMS, a hash, NaN-free */
static uint32_t run(double seconds, int16_t *pcm, double *peak, double *rms)
{
    uint32_t frames = (uint32_t)(seconds * SS_SR) / 128 * 128, i, k, h = 2166136261u;
    double sum = 0, pk = 0;
    for (i = 0; i < frames; i += 128) {
        ss_render(blk, 128, 4096);
        for (k = 0; k < 256; k++) {
            double v = blk[k] / 8388608.0;
            if (fabs(v) > pk) pk = fabs(v);
            sum += v * v;
            h = (h ^ (uint32_t)blk[k]) * 16777619u;
            if (pcm) pcm[2 * i + k] = (int16_t)(blk[k] >> 8);
        }
    }
    if (peak) *peak = pk;
    if (rms) *rms = sqrt(sum / (frames * 2.0));
    return h;
}

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : ".";
    char path[512];
    double peak, rms;
    uint32_t h1, h2, n;
    static int16_t pcm[2 * SS_SR * 20];
    static uint8_t saved[4096], saved2[4096];
    int i, v;

    /* silence until PLAY; then sound, bounded, deterministic for a seed */
    ss_init(1234);
    run(0.5, 0, &peak, &rms);
    CHECK(peak == 0.0, "sound before PLAY: peak %f", peak);
    tap_button(SS_B_PLAY);
    CHECK(ss_state(SS_Q_PLAYING) == 1, "PLAY did not start");
    h1 = run(15.2, pcm, &peak, &rms);
    printf("groove: peak %.3f rms %.3f hash %08x\n", peak, rms, h1);
    CHECK(peak > 0.3 && peak <= 1.0, "peak %f", peak);
    CHECK(rms > 0.03 && rms < 0.5, "rms %f", rms);
    snprintf(path, sizeof path, "%s/groove.wav", out);
    write_wav(path, pcm, (uint32_t)(15.2 * SS_SR) / 128 * 128);
    snprintf(path, sizeof path, "%s/screen-groove.ppm", out);
    write_screen(path);

    ss_init(1234);
    now_ms = 0;
    tap_button(SS_B_PLAY);
    h2 = run(15.2, 0, 0, 0);
    CHECK(h1 == h2, "same seed, different audio: %08x %08x", h1, h2);
    ss_init(99);
    tap_button(SS_B_PLAY);
    CHECK(run(15.2, 0, 0, 0) != h1, "another seed, the same audio");

    /* the clock: 145 BPM is 9.67 sixteenths a second; MIDI clock 6 a sixteenth */
    ss_init(5);
    tap_button(SS_B_PLAY);
    while (ss_midi_rt_pop() >= 0) {}
    v = ss_state(SS_Q_GSTEP);
    run(10.0, 0, 0, 0);
    v = ss_state(SS_Q_GSTEP) - v;
    CHECK(v >= 96 && v <= 98, "steps in 10 s at 145 BPM: %d", v);
    for (n = 0; ss_midi_rt_pop() >= 0;) n++;
    CHECK(n > 0, "no MIDI clock");
    tap_button(SS_B_PLAY);
    CHECK(ss_state(SS_Q_PLAYING) == 0, "PLAY did not stop");
    run(6.0, 0, 0, 0);
    run(0.5, 0, &peak, 0);
    CHECK(peak < 0.01, "tails after stop: %f", peak);

    /* steps: a white key toggles; hold and turn edits without toggling; ALGO sets the length (polymetre) */
    ss_init(7);
    tap_key(1 + 2 * 3 > 0 ? 8 : 8);                 /* black key 4 (key 8): the HAPPO track */
    CHECK(ss_state(SS_Q_TRACK) == SS_HAPPO, "track %d", ss_state(SS_Q_TRACK));
    tap_key(15);                                    /* black key 6: clear */
    CHECK(ss_state(SS_Q_STEPS_ON) == 0, "clear left %d steps", ss_state(SS_Q_STEPS_ON));
    turn(SS_E_ALGO, 16);
    turn(SS_E_ALGO, -9);
    CHECK(ss_state(SS_Q_LEN) == 7, "len %d", ss_state(SS_Q_LEN));
    tap_key(0);
    tap_key(4);
    CHECK(ss_state(SS_Q_STEPS_ON) == 2, "steps on %d", ss_state(SS_Q_STEPS_ON));
    held_k = 1u << 4; frame(0);                     /* hold step 3, turn K1: the note; no toggle on release */
    turn(SS_E_K1, 5);
    held_k = 0; frame(0); frame(0);
    CHECK(ss_state(SS_Q_STEPS_ON) == 2, "an edited step toggled: %d", ss_state(SS_Q_STEPS_ON));
    CHECK(ss_project()->pat[0].tr[SS_HAPPO].step[2].note == 5, "note %d", ss_project()->pat[0].tr[SS_HAPPO].step[2].note);
    tap_key(26);                                    /* step 16 is beyond the loop: nothing */
    CHECK(ss_state(SS_Q_STEPS_ON) == 2, "a step beyond the length was set");
    tap_button(SS_B_PLAY);
    for (i = 0; i < 400; i++) {                     /* the playhead never leaves the loop */
        run(0.01, 0, 0, 0);
        CHECK(ss_state(SS_Q_POS) < 7, "pos %d in a loop of 7", ss_state(SS_Q_POS));
    }
    tap_button(SS_B_PLAY);
    tap_key(25);                                    /* black key 10: undo (the last kept: before the clear) */

    /* pages, tempo, mute, generate */
    tap_button(SS_B_HOME);
    turn(SS_E_K1, -6);
    CHECK(ss_state(SS_Q_BPM) == 139, "bpm %d", ss_state(SS_Q_BPM));
    tap_button(SS_B_FX);
    CHECK(ss_state(SS_Q_PAGE) == SS_PG_FX, "page %d", ss_state(SS_Q_PAGE));
    tap_button(SS_B_LFO);
    CHECK(ss_state(SS_Q_MUTE) == 1, "mute");
    tap_button(SS_B_LFO);
    tap_button(SS_B_SEL);
    CHECK(ss_state(SS_Q_STEPS_ON) > 0, "generate made nothing");
    snprintf(path, sizeof path, "%s/screen-fx.ppm", out);
    write_screen(path);
    held_k = 1u << 0; frame(0);
    snprintf(path, sizeof path, "%s/screen-step.ppm", out);
    write_screen(path);
    held_k = 0; frame(0);
    tap_key(0);                                     /* (put step 1 back as it was) */

    /* the project: saved, damaged copies refused, loaded back the same */
    CHECK(ss_state(SS_Q_UNSAVED) == 1, "not marked unsaved");
    tap_button(SS_B_SAVE);
    CHECK(ss_save_wanted() == 1 && ss_save_wanted() == 0, "save request");
    n = ss_project_get(saved, sizeof saved);
    CHECK(n == sizeof(ss_project_t) && n <= 3840, "project size %u", n);
    printf("project: %u bytes\n", n);
    ss_init(31337);
    saved[40] ^= 1;
    CHECK(ss_project_set(saved, n) != 0, "a damaged project loaded");
    saved[40] ^= 1;
    CHECK(ss_project_set(saved, n - 1) != 0, "a short project loaded");
    CHECK(ss_project_set(saved, n) == 0, "the project did not load");
    CHECK(ss_state(SS_Q_BPM) == 139, "bpm after load %d", ss_state(SS_Q_BPM));
    CHECK(ss_project_get(saved2, sizeof saved2) == n && !memcmp(saved, saved2, n), "load and save differ");

    /* every generated groove of many seeds plays without clipping hard or going silent */
    for (i = 0; i < 40; i++) {
        ss_init(1000u + (uint32_t)i * 7919u);
        tap_button(SS_B_PLAY);
        run(4.0, 0, &peak, &rms);
        CHECK(peak > 0.2 && peak <= 1.0 && rms > 0.02, "seed %d: peak %f rms %f", i, peak, rms);
    }

    /* K4 on a held step: the ratchet; LFO held: solo; CHAIN walks the patterns every four bars */
    ss_init(11);
    held_k = 1u << 0; frame(0);
    turn(SS_E_K4, 2);
    held_k = 0; frame(0); frame(0);
    CHECK(SS_ST_RATCH(ss_project()->pat[0].tr[0].step[0].flags) == 3, "ratchet %u",
          SS_ST_RATCH(ss_project()->pat[0].tr[0].step[0].flags));
    held_b = 1u << SS_B_LFO;
    for (i = 0; i < 40; i++) frame(0);
    held_b = 0; frame(0);
    CHECK(ss_state(SS_Q_SOLO) == 0 && ss_state(SS_Q_MUTE) == 0, "solo %d mute %d", ss_state(SS_Q_SOLO), ss_state(SS_Q_MUTE));
    tap_button(SS_B_PLAY);
    run(4.0, 0, &peak, &rms);
    CHECK(peak > 0.2, "the solo kick is silent: %f", peak);
    held_b = 1u << SS_B_LFO;
    for (i = 0; i < 40; i++) frame(0);
    held_b = 0; frame(0);
    CHECK(ss_state(SS_Q_SOLO) == -1, "solo off: %d", ss_state(SS_Q_SOLO));
    tap_button(SS_B_PLAY);
    ss_project()->chain = 2;
    tap_button(SS_B_PLAY);
    run(64 * 15.0 / 145 + 0.3, 0, 0, 0);
    CHECK(ss_state(SS_Q_PATTERN) == 1, "chain after 4 bars: pattern %d", ss_state(SS_Q_PATTERN));
    run(2 * 64 * 15.0 / 145, 0, 0, 0);
    CHECK(ss_state(SS_Q_PATTERN) == 0, "chain after 12 bars: pattern %d", ss_state(SS_Q_PATTERN));

    /* SEQ: the EUCLID page spreads hits over the loop; UNDO brings the steps back */
    ss_init(21);
    tap_key(8);                                     /* HAPPO */
    v = ss_state(SS_Q_STEPS_ON);
    tap_button(SS_B_SEQ);
    CHECK(ss_state(SS_Q_PAGE) == SS_PG_EUCLID, "SEQ: page %d", ss_state(SS_Q_PAGE));
    turn(SS_E_ALGO, 16);
    turn(SS_E_K1, -16);
    CHECK(ss_state(SS_Q_STEPS_ON) == 0, "0 hits: %d steps", ss_state(SS_Q_STEPS_ON));
    turn(SS_E_K1, 5);
    CHECK(ss_state(SS_Q_STEPS_ON) == 5, "5 hits: %d steps", ss_state(SS_Q_STEPS_ON));
    {
        const ss_track_t *tr = &ss_project()->pat[0].tr[SS_HAPPO];
        int gaps = 0, last = -1, lo = 99, hi = 0;
        for (i = 0; i < 32; i++)
            if (tr->step[i % 16].flags & SS_ST_ON) {
                if (last >= 0) { int g = i - last; if (g < lo) lo = g; if (g > hi) hi = g; gaps++; }
                last = i;
            }
        CHECK(gaps && hi - lo <= 1 && (tr->step[0].flags & SS_ST_ON), "5 in 16 uneven: gaps %d..%d", lo, hi);
        turn(SS_E_K2, 2);
        CHECK(!(tr->step[0].flags & SS_ST_ON) && (tr->step[2].flags & SS_ST_ON), "rotate by 2");
        turn(SS_E_K3, -2);
        turn(SS_E_K4, 2);
        CHECK(SS_ST_PROB(tr->step[2].flags) == 2 && (tr->step[2].flags & SS_ST_ACC), "chance and accent: %02x", tr->step[2].flags);
    }
    tap_key(25);                                    /* UNDO */
    CHECK(ss_state(SS_Q_STEPS_ON) == v, "undo after euclid: %d, was %d", ss_state(SS_Q_STEPS_ON), v);

    /* SAVE held: PRESETS copies the pattern, SELECT the track; a tap still saves */
    ss_init(22);
    ss_save_wanted();
    held_b = 1u << SS_B_SAVE; frame(0);
    turn(SS_E_PRESET, 2);
    held_b = 0; frame(0);
    CHECK(!memcmp(&ss_project()->pat[0], &ss_project()->pat[2], sizeof(ss_pattern_t)), "pattern A not copied to C");
    CHECK(ss_state(SS_Q_PATTERN) == 0 && ss_save_wanted() == 0, "a copy changed the pattern or saved");
    CHECK(memcmp(&ss_project()->pat[0].tr[1], &ss_project()->pat[3].tr[1], sizeof(ss_track_t)), "D is A already");
    tap_key(3);                                     /* BASSO */
    held_b = 1u << SS_B_SAVE; frame(0);
    turn(SS_E_SELECT, 3);
    held_b = 0; frame(0);
    CHECK(ss_state(SS_Q_TRACK) == SS_BASSO, "SELECT under SAVE moved the track");
    CHECK(!memcmp(&ss_project()->pat[0].tr[1], &ss_project()->pat[3].tr[1], sizeof(ss_track_t)), "track not copied to D");
    CHECK(memcmp(&ss_project()->pat[0].tr[2], &ss_project()->pat[3].tr[2], sizeof(ss_track_t)), "more than the track was copied");
    tap_button(SS_B_SAVE);
    CHECK(ss_save_wanted() == 1, "a tap on SAVE did not save");

    /* the sidechain: the muted kick still ducks the rest; a track with AMOUNT 0 is left alone */
    {
        double loud, ducked;
        for (v = 0; v < 2; v++) {
            ss_init(23);
            ss_project()->pat[0].tr[SS_POTKU].mute = 1;
            ss_project()->seko = 0;
            ss_project()->duck = v ? 127 : 0;
            tap_button(SS_B_PLAY);
            run(8.0, 0, 0, v ? &ducked : &loud);
        }
        CHECK(ducked < loud * 0.96, "ghost sidechain: rms %f against %f", ducked, loud);
        tap_button(SS_B_FX);
        tap_button(SS_B_FX);
        CHECK(ss_state(SS_Q_PAGE) == SS_PG_SC, "FX twice: page %d", ss_state(SS_Q_PAGE));
    }

    /* microtiming: a step held, SELECT nudges it; it sounds that much early or late */
    for (v = -8; v <= 8; v += 8) {
        static const int32_t none[SS_NE];
        int t, first = -1;
        ss_init(24);
        for (t = 0; t < SS_TRACKS; t++)
            for (i = 0; i < SS_STEPS; i++)
                ss_project()->pat[0].tr[t].step[i].flags = 0;
        tap_key(7);                                 /* step 5 of the kick, set */
        if (v) {                                    /* (held without a turn, the release would toggle it) */
            held_k = 1u << 7; frame(0);
            turn(SS_E_SELECT, v);
            held_k = 0; frame(0); frame(0);
        }
        CHECK(ss_project()->pat[0].tr[0].step[4].shift == v && ss_state(SS_Q_STEPS_ON) == 1 &&
              ss_state(SS_Q_TRACK) == 0, "shift %d: %d", v, ss_project()->pat[0].tr[0].step[4].shift);
        run(1.0, 0, 0, 0);
        ss_input(1u << SS_B_PLAY, 0, none, now_ms);
        run(1.0, pcm, 0, 0);
        ss_input(0, 0, none, now_ms);
        for (i = 0; i < SS_SR - 128 && first < 0; i++)
            if (abs(pcm[2 * i]) > 300) first = i;
        /* step 5 starts 4 sixteenths in: 18248 samples at 145 BPM; a full shift is half a sixteenth, 2281 */
        CHECK(abs(first - (18248 + v * 2281 / 8)) < 200, "shift %d: the kick at sample %d", v, first);
    }

    /* locks: a step held on the sound page gets its own value; the track's stays; SEL makes the step plain */
    ss_init(25);
    tap_button(SS_B_EDIT);
    v = ss_project()->pat[0].tr[0].p[0];
    held_k = 1u << 0; frame(0);
    turn(SS_E_K1, 3);
    CHECK(ss_project()->pat[0].tr[0].step[0].lock[0] == v + 12 + 1 && ss_project()->pat[0].tr[0].p[0] == v,
          "lock %d, track %d (was %d)", ss_project()->pat[0].tr[0].step[0].lock[0], ss_project()->pat[0].tr[0].p[0], v);
    snprintf(path, sizeof path, "%s/screen-lock.ppm", out);
    write_screen(path);
    tap_button(SS_B_SEL);
    CHECK(ss_project()->pat[0].tr[0].step[0].lock[0] == 0, "SEL on a held step left its lock");
    held_k = 0; frame(0); frame(0);
    CHECK(ss_project()->pat[0].tr[0].step[0].flags & SS_ST_ON, "the step went off");
    {   /* a locked step sounds different from the same step without */
        uint32_t a, b;
        ss_init(26); ss_project()->seko = 0; tap_button(SS_B_PLAY); a = run(2.0, 0, 0, 0);
        ss_init(26); ss_project()->seko = 0; ss_project()->pat[0].tr[0].step[0].lock[1] = 128;
        tap_button(SS_B_PLAY); b = run(2.0, 0, 0, 0);
        CHECK(a != b, "a lock changed nothing");
    }

    /* LINK: a track's steps gated by another track's hits: with them, in their gaps, a step after them */
    for (v = 0; v < 3; v++) {
        int kick, hat, steps;
        ss_init(27);
        ss_project()->seko = 0;
        for (i = 0; i < SS_STEPS; i++) {
            ss_project()->pat[0].tr[SS_POTKU].step[i].flags = (uint8_t)(i % 4 == 0 ? 0x80 : 0);
            ss_project()->pat[0].tr[SS_HATTU].step[i].flags = 0x80;
        }
        ss_project()->pat[0].tr[SS_POTKU].len = ss_project()->pat[0].tr[SS_HATTU].len = 16;
        ss_project()->pat[0].tr[SS_POTKU].mute = (uint8_t)(v == 1);   /* a muted source gates all the same */
        tap_key(5);                                 /* HATTU */
        tap_button(SS_B_SEQ);
        tap_button(SS_B_SEQ);
        CHECK(ss_state(SS_Q_PAGE) == SS_PG_LINK, "SEQ twice: page %d", ss_state(SS_Q_PAGE));
        turn(SS_E_K1, 1);
        if (v) turn(SS_E_K2, v);
        CHECK(ss_project()->pat[0].tr[SS_HATTU].dep_src == 1 && ss_project()->pat[0].tr[SS_HATTU].dep_mode == v,
              "link %d mode %d", ss_project()->pat[0].tr[SS_HATTU].dep_src, ss_project()->pat[0].tr[SS_HATTU].dep_mode);
        tap_button(SS_B_PLAY);
        run(64 * 15.0 / 145 + 0.05, 0, 0, 0);       /* four bars and a little: step 65 has begun */
        steps = ss_state(SS_Q_GSTEP);
        kick = ss_state(SS_Q_HITS + SS_POTKU);
        hat = ss_state(SS_Q_HITS + SS_HATTU);
        if (v == 0) CHECK(kick == 17 && hat == kick, "WITH: kick %d hat %d of %d steps", kick, hat, steps);
        if (v == 1) CHECK(kick == 0 && hat == steps - 17, "GAPS: kick %d hat %d of %d steps", kick, hat, steps);
        if (v == 2) CHECK(kick == 17 && hat == 16, "AFTER: kick %d hat %d of %d steps", kick, hat, steps);
    }

    /* the TIMING page: SELECT walks the hits, a key picks a step without toggling it; the first knob shifts that
     * step, the second the whole track; the track's choice and its steps stay */
    ss_init(28);
    {
        ss_track_t *tr = &ss_project()->pat[0].tr[SS_HAPPO];
        int on;
        for (i = 0; i < SS_STEPS; i++)
            tr->step[i].flags = (uint8_t)(i == 2 || i == 5 || i == 11 ? 0x80 : 0);
        tr->len = 16;
        tap_key(8);                                 /* HAPPO */
        tap_button(SS_B_SEQ); tap_button(SS_B_SEQ); tap_button(SS_B_SEQ);
        CHECK(ss_state(SS_Q_PAGE) == SS_PG_TIMING, "SEQ three times: page %d", ss_state(SS_Q_PAGE));
        turn(SS_E_SELECT, 1);
        CHECK(ss_state(SS_Q_CURSOR) == 2 && ss_state(SS_Q_TRACK) == SS_HAPPO, "cursor %d track %d",
              ss_state(SS_Q_CURSOR), ss_state(SS_Q_TRACK));
        turn(SS_E_K1, -3);
        turn(SS_E_SELECT, 2);
        CHECK(ss_state(SS_Q_CURSOR) == 11, "cursor %d", ss_state(SS_Q_CURSOR));
        turn(SS_E_K1, 5);
        turn(SS_E_SELECT, 1);                     /* round the end, to the first hit */
        CHECK(ss_state(SS_Q_CURSOR) == 2, "cursor wrapped to %d", ss_state(SS_Q_CURSOR));
        turn(SS_E_SELECT, -1);
        CHECK(ss_state(SS_Q_CURSOR) == 11, "cursor back to %d", ss_state(SS_Q_CURSOR));
        on = ss_state(SS_Q_STEPS_ON);
        tap_key(9);                                 /* the white key of step 6 */
        CHECK(ss_state(SS_Q_CURSOR) == 5 && ss_state(SS_Q_STEPS_ON) == on, "a key on the timing page: cursor %d, %d steps",
              ss_state(SS_Q_CURSOR), ss_state(SS_Q_STEPS_ON));
        turn(SS_E_K1, 20);
        turn(SS_E_K2, -2);
        CHECK(tr->step[2].shift == -3 && tr->step[11].shift == 5 && tr->step[5].shift == 8 && tr->tshift == -2,
              "shifts %d %d %d track %d", tr->step[2].shift, tr->step[11].shift, tr->step[5].shift, tr->tshift);
        snprintf(path, sizeof path, "%s/screen-timing.ppm", out);
        write_screen(path);
    }
    for (v = -8; v <= 8; v += 16) {                 /* the whole track early or late: heard */
        static const int32_t none[SS_NE];
        int t, first = -1;
        ss_init(29);
        for (t = 0; t < SS_TRACKS; t++)
            for (i = 0; i < SS_STEPS; i++)
                ss_project()->pat[0].tr[t].step[i].flags = (uint8_t)(t == 0 && i == 4 ? 0x80 : 0);
        ss_project()->pat[0].tr[0].tshift = (int8_t)v;
        ss_input(1u << SS_B_PLAY, 0, none, now_ms);
        run(1.0, pcm, 0, 0);
        ss_input(0, 0, none, now_ms);
        for (i = 0; i < SS_SR - 128 && first < 0; i++)
            if (abs(pcm[2 * i]) > 300) first = i;
        CHECK(abs(first - (18248 + v * 2281 / 8)) < 200, "track shift %d: the kick at sample %d", v, first);
    }

    /* each voice alone sounds, stays finite and within range */
    for (v = 0; v < SS_TRACKS; v++) {
        int t;
        ss_init(40);
        ss_project()->seko = 0;
        for (t = 0; t < SS_TRACKS; t++) {
            ss_project()->pat[0].tr[t].mute = (uint8_t)(t != v);
            for (i = 0; i < SS_STEPS; i += 2)       /* (whatever the generator left: every other step, sure) */
                ss_project()->pat[0].tr[t].step[i].flags = 0x80;
        }
        tap_button(SS_B_PLAY);
        run(4.0, 0, &peak, &rms);
        printf("voice %d alone: peak %.3f rms %.3f\n", v, peak, rms);
        CHECK(peak > 0.05 && peak <= 1.0 && rms > 0.008 && rms == rms, "voice %d alone: peak %f rms %f", v, peak, rms);
    }
    {   /* every sound parameter at both ends: still finite, never beyond full scale */
        int t, k, e;
        for (e = 0; e < 2; e++) {
            ss_init(41);
            for (t = 0; t < SS_TRACKS; t++) {
                for (k = 0; k < 8; k++)
                    ss_project()->pat[0].tr[t].p[k] = (uint8_t)(e ? 127 : 0);
                ss_project()->pat[0].tr[t].bits = ss_project()->pat[0].tr[t].rate = (uint8_t)(e ? 127 : 0);
            }
            ss_project()->seko = 127;
            ss_project()->drive = (uint8_t)(e ? 127 : 0);
            tap_button(SS_B_PLAY);
            run(6.0, 0, &peak, &rms);
            CHECK(peak <= 1.0 && rms == rms, "parameters at %d: peak %f rms %f", e ? 127 : 0, peak, rms);
        }
    }

    /* the pages behind EDIT, ENV and GLO */
    ss_init(42);
    tap_button(SS_B_EDIT);
    tap_button(SS_B_EDIT);
    CHECK(ss_state(SS_Q_PAGE) == SS_PG_SOUND2, "EDIT twice: page %d", ss_state(SS_Q_PAGE));
    v = ss_project()->pat[0].tr[0].p[4];
    turn(SS_E_K1, 1);
    CHECK(ss_project()->pat[0].tr[0].p[4] == v + 2, "SOUND 2, K1: %d (was %d)", ss_project()->pat[0].tr[0].p[4], v);
    snprintf(path, sizeof path, "%s/screen-sound2.ppm", out);
    write_screen(path);
    tap_button(SS_B_ENV);
    tap_button(SS_B_ENV);
    CHECK(ss_state(SS_Q_PAGE) == SS_PG_CHAN, "ENV twice: page %d", ss_state(SS_Q_PAGE));
    tap_button(SS_B_GLO);
    tap_button(SS_B_GLO);
    CHECK(ss_state(SS_Q_PAGE) == SS_PG_GLOBAL2, "GLO twice: page %d", ss_state(SS_Q_PAGE));
    turn(SS_E_K1, -20);
    CHECK(ss_project()->pat[0].xpose == -12, "xpose %d", ss_project()->pat[0].xpose);
    turn(SS_E_K1, 15);
    CHECK(ss_project()->pat[0].xpose == 3, "xpose %d", ss_project()->pat[0].xpose);
    {   /* the key, the crusher and a slide are heard; a held step and ALGORITHM set the slide, not the length */
        uint32_t a, b, c, d;
        ss_init(43); ss_project()->seko = 0; tap_button(SS_B_PLAY); a = run(2.0, 0, 0, 0);
        ss_init(43); ss_project()->seko = 0; ss_project()->pat[0].xpose = 5; tap_button(SS_B_PLAY); b = run(2.0, 0, 0, 0);
        ss_init(43); ss_project()->seko = 0; ss_project()->pat[0].tr[SS_BASSO].bits = 100; tap_button(SS_B_PLAY); c = run(2.0, 0, 0, 0);
        ss_init(43); ss_project()->seko = 0;
        for (i = 0; i < SS_STEPS; i++)
            ss_project()->pat[0].tr[SS_BASSO].step[i].flags ^= SS_ST_SLIDE;
        tap_button(SS_B_PLAY); d = run(2.0, 0, 0, 0);
        CHECK(a != b && a != c && a != d, "xpose, bits or slide changed nothing: %08x %08x %08x %08x", a, b, c, d);
        ss_init(44);
        v = ss_state(SS_Q_LEN);
        held_k = 1u << 2; frame(0);
        turn(SS_E_ALGO, 1);
        held_k = 0; frame(0); frame(0);
        CHECK((ss_project()->pat[0].tr[0].step[1].flags & (SS_ST_SLIDE | SS_ST_ON)) == (SS_ST_SLIDE | SS_ST_ON) && ss_state(SS_Q_LEN) == v,
              "slide on a held step: flags %02x len %d", ss_project()->pat[0].tr[0].step[1].flags, ss_state(SS_Q_LEN));
    }
    {   /* SEKO: off, the melody's steps play as written; on, some of them play as other notes of the scale */
        uint32_t same[2] = {0, 0};
        int t;
        for (v = 0; v < 2; v++) {
            uint32_t a = 0, b = 0;
            for (t = 0; t < 2; t++) {               /* the same seed twice: once as written, once with SEKO as asked */
                ss_init(45);
                ss_project()->seko = (uint8_t)(t && v ? 127 : 0);
                tap_button(SS_B_PLAY);
                if (t) b = run(8.0, 0, 0, 0); else a = run(8.0, 0, 0, 0);
            }
            same[v] = a == b;
        }
        CHECK(same[0] && !same[1], "SEKO: off the same %u, on the same %u", same[0], same[1]);
    }

    /* MIDI: channel 1 (the SMK-37's keys) plays the chosen track, the kick */
    ss_init(3);
    ss_midi_note(0, 60, 127);
    run(0.2, 0, &peak, 0);
    CHECK(peak > 0.2, "MIDI note: peak %f", peak);

    /* the generator: every note of the melodic tracks in the scale (phrygian dominant from the factory); the
     * patterns carry a progression; the lines are more than one note, and differ from seed to seed */
    {
        static const int PHRDOM[12] = {1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0};
        int seed, a, t, bad = 0, chords = 0, flat = 0;
        uint32_t lines = 0, last = 0;
        for (seed = 0; seed < 30; seed++) {
            ss_init(500u + (uint32_t)seed * 31u);
            for (a = 0; a < SS_PATTERNS; a++) {
                chords |= 1 << ss_project()->pat[a].chord;
                for (t = SS_BASSO; t <= SS_OUTO; t++) {
                    const ss_track_t *tr = &ss_project()->pat[a].tr[t];
                    int distinct = 0, seen[128] = {0};
                    uint32_t h = 2166136261u;
                    if (t == SS_HATTU)
                        continue;
                    for (i = 0; i < SS_STEPS; i++)
                        if (tr->step[i].flags & SS_ST_ON) {
                            int nn = tr->step[i].note;
                            bad += !PHRDOM[((nn % 12) + 12) % 12];
                            if (!seen[nn + 64]++) distinct++;
                            h = (h ^ (uint32_t)(nn + 64)) * 16777619u;
                        }
                    if (t != SS_BASSO && t != SS_OUTO && distinct < 2)
                        flat++;
                    if (t == SS_HAPPO && a == 0) {
                        lines += h != last;
                        last = h;
                    }
                }
            }
        }
        CHECK(bad == 0, "generator: %d notes off the scale", bad);
        CHECK(flat < 6, "generator: %d acid or melody lines of a single note", flat);
        CHECK(lines >= 25, "generator: only %u different acid lines in 30 seeds", lines);
        CHECK(chords != 1, "generator: the patterns have no progression (chords %x)", chords);
    }

    /* the mixer: ENV three times MASTER; WIDTH 0 is mono; TONE darkens; the meters move */
    ss_init(40);
    tap_button(SS_B_ENV); tap_button(SS_B_ENV); tap_button(SS_B_ENV);
    CHECK(ss_state(SS_Q_PAGE) == SS_PG_MASTER, "ENV x3: page %d", ss_state(SS_Q_PAGE));
    tap_button(SS_B_ENV);
    CHECK(ss_state(SS_Q_PAGE) == SS_PG_MIX, "ENV x4: page %d", ss_state(SS_Q_PAGE));
    ss_project()->width = 0;
    tap_button(SS_B_PLAY);
    {
        int32_t b2[256];
        int diff = 0, j;
        for (j = 0; j < 200; j++) {
            ss_render(b2, 128, 4096);
            for (i = 0; i < 128; i++) diff |= b2[2 * i] != b2[2 * i + 1];
        }
        CHECK(!diff, "WIDTH 0 is not mono");
    }
    snprintf(path, sizeof path, "%s/screen-mixer.ppm", out);
    frame(0);
    write_screen(path);
    {
        double bright = 0, dark = 0;
        int v2, t;
        for (v2 = 0; v2 < 2; v2++) {
            int32_t b3[256];
            double sum = 0, prev = 0;
            int j;
            ss_init(41);
            ss_project()->seko = 0;
            for (t = 0; t < SS_TRACKS; t++) ss_project()->pat[0].tr[t].tone = v2 ? 10 : 64;
            tap_button(SS_B_PLAY);
            for (j = 0; j < 1000; j++) {      /* the energy of the difference: high frequencies */
                ss_render(b3, 128, 4096);
                for (i = 0; i < 128; i++) { double x = b3[2 * i] / 8388608.0; sum += (x - prev) * (x - prev); prev = x; }
            }
            if (v2) dark = sum; else bright = sum;
        }
        CHECK(dark < bright * 0.5, "TONE LP: %f against %f", dark, bright);
    }

    /* MOD: an LFO on BASSO's cutoff changes the sound */
    {
        uint32_t a1, a2;
        ss_init(42); ss_project()->seko = 0; tap_button(SS_B_PLAY); a1 = run(3.0, 0, 0, 0);
        ss_init(42); ss_project()->seko = 0;
        ss_project()->pat[0].tr[SS_BASSO].lfo_depth = 127;
        ss_project()->pat[0].tr[SS_BASSO].lfo_rate = 3;
        tap_button(SS_B_PLAY); a2 = run(3.0, 0, 0, 0);
        CHECK(a1 != a2, "the LFO changed nothing");
        tap_button(SS_B_EDIT); tap_button(SS_B_EDIT); tap_button(SS_B_EDIT);
        CHECK(ss_state(SS_Q_PAGE) == SS_PG_MOD, "EDIT x3: page %d", ss_state(SS_Q_PAGE));
    }

    /* MAXI with the factory map: K1 on BASSO, F2 its level, pad 10 mutes it, the keys sound, the mod wheel SEKO */
    ss_init(43);
    CHECK(ss_state(SS_Q_MAXI) == 0, "MAXI before any SMK-37 message");
    tap_key(3);                                     /* BASSO */
    ss_midi_cc(0, 20, 5); frame(0);
    CHECK(ss_project()->pat[0].tr[SS_BASSO].p[0] == 5 && ss_state(SS_Q_MAXI) == 1, "K1: cutoff %d",
          ss_project()->pat[0].tr[SS_BASSO].p[0]);
    ss_midi_cc(0, 27, 127); frame(0);
    CHECK(ss_project()->pat[0].tr[SS_BASSO].p[7] == 127, "K8: drive %d", ss_project()->pat[0].tr[SS_BASSO].p[7]);
    ss_midi_cc(0, 42, 33); frame(0);
    CHECK(ss_project()->pat[0].tr[SS_BASSO].level == 33, "F2: level %d", ss_project()->pat[0].tr[SS_BASSO].level);
    ss_midi_note(9, 36 + 9, 100); frame(0);
    CHECK(ss_project()->pat[0].tr[SS_BASSO].mute == 1, "pad 10 did not mute BASSO");
    ss_midi_note(9, 36 + 4, 100); frame(0);
    CHECK(ss_state(SS_Q_TRACK) == SS_PIIP, "pad 5: track %d", ss_state(SS_Q_TRACK));
    ss_midi_cc(0, 1, 99); frame(0);
    CHECK(ss_project()->seko == 99, "mod wheel: SEKO %d", ss_project()->seko);
    /* PAD BANK: pad 17 + 2 toggles PIIP's step 3; ▶ (CC 103) the next pattern, ◀ (CC 102) back */
    {
        int was = ss_project()->pat[0].tr[SS_PIIP].step[2].flags & SS_ST_ON;
        ss_midi_note(9, 52 + 2, 90); frame(0);
        CHECK((ss_project()->pat[0].tr[SS_PIIP].step[2].flags & SS_ST_ON) != was, "PAD BANK: step 3 not toggled");
        ss_midi_cc(0, 103, 127); ss_midi_cc(0, 103, 0); frame(0);
        CHECK(ss_project()->cur_pat == 1, "right: pattern %d", ss_project()->cur_pat);
        ss_midi_cc(0, 102, 127); frame(0);
        CHECK(ss_project()->cur_pat == 0, "left: pattern %d", ss_project()->cur_pat);
    }
    tap_button(SS_B_EDIT);
    snprintf(path, sizeof path, "%s/screen-maxi.ppm", out);
    write_screen(path);
    ss_midi_note(0, 72, 120);
    run(0.3, 0, &peak, 0);
    CHECK(peak > 0.05, "a key on the SMK-37 played nothing: %f", peak);

    /* learning: GLO x3, SEL; a key, a pad, a knob that sends positions, one that sends steps; SAVE ends */
    ss_init(44);
    tap_button(SS_B_GLO); tap_button(SS_B_GLO); tap_button(SS_B_GLO);
    CHECK(ss_state(SS_Q_PAGE) == SS_PG_SMK, "GLO x3: page %d", ss_state(SS_Q_PAGE));
    tap_button(SS_B_SEL);
    CHECK(ss_state(SS_Q_LEARN) == 1, "SEL did not start learning");
    snprintf(path, sizeof path, "%s/screen-learn.ppm", out);
    write_screen(path);
    ss_midi_note(2, 48, 90); frame(0);
    ss_midi_note(3, 60, 90); frame(0);
    ss_midi_note(3, 60, 90); frame(0);             /* pad 1 again: not pad 17 */
    ss_midi_note(3, 76, 90); frame(0);
    ss_midi_cc(4, 70, 10); ss_midi_cc(4, 70, 11); ss_midi_cc(4, 70, 12); frame(0);
    ss_midi_cc(4, 71, 1); ss_midi_cc(4, 71, 1); ss_midi_cc(4, 71, 1); frame(0);
    tap_button(SS_B_SAVE);
    {
        const ss_smk_t *m = &ss_project()->smk;
        CHECK(ss_state(SS_Q_LEARN) == 0 && m->key_ch == 2 && m->key_base == 48 && m->pad_ch == 3 && m->pad_note == 60 &&
              m->pad2_note == 76 && m->left_cc == 255 &&
              m->cc_ch == 4 && m->knob_cc[0] == 70 && m->knob_cc[1] == 71 && m->knob_cc[2] == 255 &&
              m->knob_rel == 2 && m->rel_kind == 0 && m->fader_cc[0] == 255,
              "learned: keys %d/%d pads %d/%d cc ch %d knobs %d %d %d rel %x/%d", m->key_ch, m->key_base, m->pad_ch,
              m->pad_note, m->cc_ch, m->knob_cc[0], m->knob_cc[1], m->knob_cc[2], m->knob_rel, m->rel_kind);
        v = ss_project()->pat[0].tr[0].p[1];
        ss_midi_cc(4, 71, 127); frame(0);
        CHECK(ss_project()->pat[0].tr[0].p[1] == v - 2, "a knob that steps: %d, was %d", ss_project()->pat[0].tr[0].p[1], v);
    }

    printf(fails ? "%d FAILED\n" : "all tests passed\n", fails);
    return fails != 0;
}
