/* SPDX-License-Identifier: MIT
 * suom1saundi on the M-VAVE FM-1: the app's half of the contract with fm1-base (base/device/unity.c on the FM-1,
 * base/host/plat_host.c in the host simulator). The instrument itself is core/suomi.c. */
#pragma once
#include <stdint.h>
#include "plat.h"

#define APP_NAME "suom1saundi"
#define APP_TAGLINE "suomisaundi groovebox"
#define APP_SPLASH_RGB RGB(255, 40, 200)
#define APP_BLOCK 256u                   /* frames per audio block: 5.8 ms */

/* the store: the project (settings and four patterns, about 3.7 KiB), and the platform's panel calibration */
enum { OBJ_PROJECT, OBJ_PANEL, OBJ_NOBJ };
#define APP_STORE_MAP {0xFC000u, 0xDC000u}

/* the contract: the platform calls these */
void app_init(void);
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12);
void app_panic(void);
void app_ui_init(void);
void app_frame(void);
void app_input_only(void);
void app_say(const char *a, const char *b);

/* the host simulator's checks (app/tests/basic.txt) */
#define APP_HOST_EXPECT
int app_expect(const char *what, int *got);
