/* SPDX-License-Identifier: MIT
 * suom1saundi on the M-VAVE FM-1: the glue between fm1-base's platform and the core (core/suomi.c, built as a
 * separate -O2 unit: fm1.toml o2). Part of fm1-base's unity build, after the platform: lcd.c and gfx.c are in scope.
 *
 * NOT YET RUN ON AN FM-1. See README.md, "Stand".
 */
#include "core/suomi.h"

/* the core names the panel's controls in plat.h's order */
_Static_assert((int)SS_B_FX == B_FX && (int)SS_B_SEL == B_SEL && (int)SS_B_ENV == B_ENV && (int)SS_B_LFO == B_LFO &&
               (int)SS_B_EDIT == B_EDIT && (int)SS_B_GLO == B_GLO && (int)SS_B_HOME == B_HOME && (int)SS_B_SAVE == B_SAVE &&
               (int)SS_B_ARP == B_ARP && (int)SS_B_SEQ == B_SEQ && (int)SS_B_PLAY == B_PLAY && (int)SS_B_REC == B_REC &&
               (int)SS_B_OCTDN == B_OCTDN && (int)SS_B_OCTUP == B_OCTUP && (int)SS_NB == NB, "buttons");
_Static_assert((int)SS_E_SELECT == EN_SELECT && (int)SS_E_ALGO == EN_ALGO && (int)SS_E_PRESET == EN_PRESET &&
               (int)SS_E_K1 == EN_K1 && (int)SS_E_K4 == EN_K4 && (int)SS_NE == NE, "encoders");
_Static_assert(SS_NKEYS == NKEYS, "keys");
_Static_assert(sizeof(ss_project_t) <= PLAT_STORE_MAX, "the project fits one store object");
_Static_assert(SS_W * SS_STRIP_H <= CV_MAX, "a strip fits the canvas");

static uint8_t prj_buf[sizeof(ss_project_t)];

/* ------------------------------------------------------------------ audio --- */
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    uint32_t pkt;
    int port, b;
    while (plat_midi_in(&pkt, &port)) {
        uint32_t st = (pkt >> 8) & 0xFFu, d1 = (pkt >> 16) & 0x7Fu, d2 = (pkt >> 24) & 0x7Fu;
        if ((st & 0xF0u) == 0x90u && d2)
            ss_midi_note(st & 15u, d1, d2);
        else if (st == 0xFAu)
            ss_midi_transport(1);
        else if (st == 0xFCu)
            ss_midi_transport(0);
    }
    ss_render(lr, n, master_q12);
    while ((b = ss_midi_rt_pop()) >= 0)            /* MIDI clock, start and stop: single-byte packets (CIN 0xF) */
        plat_midi_send((uint32_t)b << 8 | 0x0Fu, TO_USB);
}

void app_panic(void) { ss_panic(); }

/* ------------------------------------------------------------------- boot --- */
void app_init(void)
{
    int n;
    ss_init(plat_ticks24() ^ 0x524F4D49u);
    n = plat_store_load(OBJ_PROJECT, prj_buf, sizeof prj_buf);
    if (n > 0)
        (void)ss_project_set(prj_buf, (uint32_t)n);   /* refused (another version, damaged): the fresh groove stays */
}

/* --------------------------------------------------------------------- UI --- */
static void input(void)
{
    int32_t enc[SS_NE];
    uint32_t i, b, k, db, dk;
    for (i = 0; i < SS_NE; i++)
        enc[i] = plat_enc((int)i);
    ss_input(plat_buttons(), plat_keys(), enc, plat_ms());
    ss_lights(&b, &k, &db, &dk);
    plat_lights(b, k, db, dk, 1);
}

void app_input_only(void) { input(); }

void app_say(const char *a, const char *b) { ss_message(a, b); }

void app_ui_init(void)
{
    lcd_fill(0, 0, 240, 240, C_BLACK);
}

void app_frame(void)
{
    uint32_t dirty, s;
    input();
    if (ss_save_wanted()) {                        /* (a flash write holds the sound for a moment) */
        uint32_t n = ss_project_get(prj_buf, sizeof prj_buf);
        if (!n || plat_store_save(OBJ_PROJECT, prj_buf, n) != 0)
            ss_message("SAVE", "FAILED");
    }
    dirty = ss_dirty();
    for (s = 0; s < SS_STRIPS; s++)
        if (dirty >> s & 1u) {
            cv_begin(SS_W, SS_STRIP_H, C_BLACK);
            uint32_t i;
            ss_draw(cv_px, (int)s);
            for (i = 0; i < SS_W * SS_STRIP_H; i++)   /* the canvas holds pixels in the panel's byte order */
                cv_px[i] = swap16(cv_px[i]);
            cv_blit(0, s * SS_STRIP_H);
        }
}

#ifdef PLAT_HOST
int app_expect(const char *what, int *got)
{
    static const struct { const char *name; int q; } Q[] = {
        {"playing", SS_Q_PLAYING}, {"track", SS_Q_TRACK}, {"page", SS_Q_PAGE}, {"bpm", SS_Q_BPM},
        {"pattern", SS_Q_PATTERN}, {"keys_mode", SS_Q_KEYS_MODE}, {"len", SS_Q_LEN},
        {"steps", SS_Q_STEPS_ON}, {"mute", SS_Q_MUTE}, {"unsaved", SS_Q_UNSAVED},
    };
    uint32_t i;
    for (i = 0; i < sizeof Q / sizeof Q[0]; i++)
        if (!strcmp(what, Q[i].name)) {
            *got = ss_state(Q[i].q);
            return 1;
        }
    return 0;
}
#endif
