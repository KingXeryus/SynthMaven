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
    check(HY_NFACTORY == 36 && NELEM(HY_PRESETS) == 41,
          "HYBRID: nine XD defaults append without shifting native factory/preset IDs");
    for (i = 0; i < 9; i++) {
        uint32_t slot = 27u + i, k, active = 0;
        set_engine_of(TSEL, ENGI_HYBRID); apply_preset_to(TSEL, 32u+i);
        hy_slot_seen[0] = -1; hy_gen_seen[0] = hy_gen[0]; hy_block(TSEL);
        check(TSEL->p[P_E3] == (int16_t)slot && !memcmp(&hy_cur[song.sel], &HY_FACTORY[slot], HY_PATCH_SIZE),
              "XD hardware preset resolves to its exact translated factory patch");
        check(!strncmp(HY_PRESETS[32u+i].name, "XD ", 3), "XD default has a visible compatibility name");
        for(k=0;k<HY_NOSC;k++)active += hy_cur[song.sel].osc[k].type && hy_cur[song.sel].osc[k].level;
        check(active > 0, "XD default has at least one enabled audible oscillator");
    }
    return fails ? 1 : 0;
}
