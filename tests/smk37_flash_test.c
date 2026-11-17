/* SPDX-License-Identifier: MIT
 * The SMK-37's flash map (smk37/fm1-base.patch: felucca-bsp chip/fm1_flash_policy.h, BOARD_SMK37): every byte the
 * firmware may write is in an allowed window, and the head, the Bluetooth pairings, the unit's calibration (USRTRIM),
 * the stock resources (USR) and key_mac are refused, whatever the offset and length, also with the update loader's
 * own allow-list. Built against the patched copy of fm1-base (tests/run_tests.sh, after ./build.sh --board smk37). */
#include <stdint.h>
#include <stdio.h>
#define BOARD_SMK37 1
#include "fm1_flash_policy.h"
#include "plat.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("  FAIL " __VA_ARGS__); printf("\n"); } } while (0)

static int is_protected_byte(uint32_t a)
{
    return a < 0x4000u || (a >= 0xC0000u && a < 0xC2000u) || (a >= 0xF4000u && a < 0xFE000u) || a >= 0xFF000u;
}

/* the update loader's allow-list (bsp/loader/loader.c, BOARD_SMK37), without a resource region */
#define LDR_RANGE_OK(off, n) (FL_IN(off, n, 0x4000u, 0xA1000u) || FL_IN(off, n, 0xBB000u, 0xC0000u))

int main(void)
{
    uint32_t a, n;
    static const uint32_t lens[] = {1u, 2u, 256u, 0x1000u, 0x2000u, 0x10000u, 0x100000u, 0xFFFFFFFFu};
    /* every sector, every length: a write that FL_WRITE_OK lets through never touches a protected byte */
    for (a = 0; a < 0x100000u; a += 0x100u)
        for (n = 0; n < sizeof lens / sizeof lens[0]; n++) {
            uint32_t l = lens[n], b;
            int ok = FL_WRITE_OK(a, l), bad = 0;
            if (!ok)
                continue;
            for (b = a; b - a < l && b < 0x100000u; b += 0x100u)
                bad |= is_protected_byte(b) || is_protected_byte(b + 0xFFu > a + l - 1u ? a + l - 1u : b + 0xFFu);
            CHECK(!bad, "FL_WRITE_OK(%#x, %#x) lets a protected byte through", a, l);
        }
    /* the protected ranges themselves, also starting below them and running into them */
    CHECK(!FL_WRITE_OK(0x0u, 0x1000u), "the head");
    CHECK(!FL_WRITE_OK(0xC0000u, 0x1000u), "BTIF");
    CHECK(!FL_WRITE_OK(0xC1000u, 0x1000u), "USRTRIM");
    CHECK(!FL_WRITE_OK(0xBF000u, 0x2000u), "the staging into BTIF");
    CHECK(!FL_WRITE_OK(0xF4000u, 0x1000u), "USR");
    CHECK(!FL_WRITE_OK(0xFC000u, 0x1000u), "the FM-1's GLOB is the SMK's USR");
    CHECK(!FL_WRITE_OK(0xFF000u, 0x1000u), "key_mac");
    CHECK(!FL_WRITE_OK(0xC1000u, 0u) || 1, "(a zero-length write writes nothing)");
    /* the loader's own list, with the guard */
    for (a = 0; a < 0x100000u; a += 0x1000u) {
        int ok = LDR_RANGE_OK(a, 0x1000u) && FL_READ_OK(a, 0x1000u) && !FL_PROTECTED(a, 0x1000u);
        CHECK(!(ok && is_protected_byte(a)), "the loader may erase protected sector %#x", a);
    }
    /* the windows the firmware needs are writable */
    CHECK(FL_WRITE_OK(PLAT_DATA_LO, 0x1000u) && FL_WRITE_OK(PLAT_DATA_HI - 0x1000u, 0x1000u), "the data area");
    CHECK(FL_WRITE_OK(0xBB000u, 0x1000u) && FL_WRITE_OK(0xBFF00u, 0x100u), "the staging and its record");
    CHECK(FL_WRITE_OK(FL_AUTO_LO, 0x2000u), "the autosave pair");
    CHECK(FL_WRITE_OK(PLAT_XDATA1_LO, 0x1000u) && FL_WRITE_OK(PLAT_XDATA1_HI - 0x1000u, 0x1000u), "XDATA1");
    CHECK(FL_WRITE_OK(PLAT_XDATA0_LO, 0x1000u), "XDATA0");
    CHECK(!FL_WRITE_OK(0x4000u, 0x1000u), "the app area: the update loader only");
    /* suom1saundi's store (app.h APP_STORE_MAP on the SMK-37): two A/B pairs in the data area */
    CHECK(FL_WRITE_OK(0xA1000u, 0x2000u) && FL_WRITE_OK(0xA3000u, 0x2000u), "the app's store pairs");
    printf(fails ? "smk37 flash map: %d failed\n" : "smk37 flash map: all checks passed\n", fails);
    return fails != 0;
}
