/* SPDX-License-Identifier: MIT
 * The browser test page's side of suom1saundi: the same core as the firmware (app/core/suomi.c), compiled to
 * WebAssembly without a libc (sim/build.sh), with a framebuffer and an audio block the page reads. */
#include "../app/core/suomi.h"

#define EXPORT(n) __attribute__((export_name(#n))) n

void *memcpy(void *d, const void *s, unsigned long n)
{
    unsigned char *a = d;
    const unsigned char *b = s;
    while (n--)
        *a++ = *b++;
    return d;
}
void *memset(void *d, int c, unsigned long n)
{
    unsigned char *a = d;
    while (n--)
        *a++ = (unsigned char)c;
    return d;
}

static uint16_t fb[SS_W * SS_H];
static int32_t pcm[2 * 128];
static float out[2 * 128];
static uint32_t lights[4];
static uint8_t prj[4096];

void EXPORT(sim_init)(uint32_t seed) { ss_init(seed); }
uint16_t *EXPORT(sim_fb)(void) { return fb; }
float *EXPORT(sim_out)(void) { return out; }
uint32_t *EXPORT(sim_lights)(void) { return lights; }
uint8_t *EXPORT(sim_prj)(void) { return prj; }

void EXPORT(sim_audio)(uint32_t gain_q12)   /* 128 stereo frames into sim_out, -1..1 */
{
    int i;
    ss_render(pcm, 128, gain_q12);
    for (i = 0; i < 256; i++)
        out[i] = (float)pcm[i] * (1.0f / 8388608.0f);
}

/* one UI frame: the panel in, the strips that changed drawn; returns the strips drawn */
uint32_t EXPORT(sim_frame)(uint32_t buttons, uint32_t keys, int32_t e0, int32_t e1, int32_t e2, int32_t e3,
                           int32_t e4, int32_t e5, int32_t e6, uint32_t now_ms)
{
    int32_t enc[SS_NE];
    uint32_t dirty, s;
    enc[0] = e0; enc[1] = e1; enc[2] = e2; enc[3] = e3; enc[4] = e4; enc[5] = e5; enc[6] = e6;
    ss_input(buttons, keys, enc, now_ms);
    ss_lights(&lights[0], &lights[1], &lights[2], &lights[3]);
    dirty = ss_dirty();
    for (s = 0; s < SS_STRIPS; s++)
        if (dirty >> s & 1u)
            ss_draw(fb + s * SS_W * SS_STRIP_H, (int)s);
    return dirty;
}

uint32_t EXPORT(sim_save)(void) { return ss_save_wanted() ? ss_project_get(prj, sizeof prj) : 0; }
int EXPORT(sim_load)(uint32_t len) { return ss_project_set(prj, len); }
int EXPORT(sim_state)(int what) { return ss_state(what); }
/* MIDI from the page (the virtual SMK-37, or a real one through Web MIDI): one message of up to three bytes */
void EXPORT(sim_midi)(uint32_t st, uint32_t d1, uint32_t d2)
{
    if ((st & 0xF0u) == 0x90u && d2)
        ss_midi_note(st & 15u, d1, d2);
    else if ((st & 0xF0u) == 0xB0u)
        ss_midi_cc(st & 15u, d1, d2);
    else if ((st & 0xF0u) == 0xE0u)
        ss_midi_bend(st & 15u, (int)(d2 << 7 | d1) - 8192);
    else if (st == 0xFAu)
        ss_midi_transport(1);
    else if (st == 0xFCu)
        ss_midi_transport(0);
}

/* the SMK-37 as itself (docs/emulator-smk37.html): its own controls straight into the core (ss_smk_*) */
void EXPORT(sim_smk_native)(int on) { ss_smk_native(on); }
void EXPORT(sim_smk_key)(int key, int vel) { ss_smk_key(key, vel); }
void EXPORT(sim_smk_pad)(int pad, int vel) { ss_smk_pad(pad, vel); }
void EXPORT(sim_smk_knob)(int knob, int d) { ss_smk_knob(knob, d); }
void EXPORT(sim_smk_fader)(int f, int v) { ss_smk_fader(f, v); }
void EXPORT(sim_smk_bend)(int v) { ss_smk_bend(v); }
void EXPORT(sim_smk_mod)(int v) { ss_smk_mod(v); }
void EXPORT(sim_smk_button)(int b) { ss_smk_button(b); }
int EXPORT(sim_smk_knob_bank)(void) { return ss_smk_knob_bank(); }
uint32_t EXPORT(sim_smk_pad_rgb)(int pad) { return ss_smk_pad_rgb(pad); }
uint32_t EXPORT(sim_smk_leds)(void) { return ss_smk_leds(); }
