/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void) {
    uint32_t i;
    uint8_t pk[128], v[FP_SIZE + 1u];
    panel = PANEL_DEFAULT; layers_init(); host_tracks_init(); bank_resolve();
    check(FM6_NFACTORY == 41 && ENG_FM6.npresets == 41, "FM6: eight existing plus 33 imported stock voices");
    for (i = 8; i < FM6_NFACTORY; ++i) {
        set_engine_of(TSEL, ENGI_FM6); apply_preset_to(TSEL, i); fm6_track_loaded(TSEL);
        fm6_slot_get(35u + i - 8u, pk);
        fm6_unpack(FM6_FACTORY[i], v);
        check(!memcmp(pk, FM6_FACTORY[i], 128) && !memcmp(fm6_patch[0], v, FP_SIZE),
              "stock preset loads its exact imported DX7 voice");
    }
    check(!strcmp(FM6_PRESETS[8].name, "BRASS   1") && !strcmp(FM6_PRESETS[40].name, "ARABESC"),
          "stock names: ROM1A first, ARABESC last");
    fm6_slot_get(8, pk);
    check(!memcmp(pk, FM6_INIT, 128), "legacy B1 stays PTCH 8 (empty bank still init)");
    fm6_slot_get(34, pk);
    check(!memcmp(pk, FM6_INIT, 128), "legacy B27 stays PTCH 34");
    return fails ? 1 : 0;
}
