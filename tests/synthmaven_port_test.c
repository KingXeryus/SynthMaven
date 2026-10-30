/* SPDX-License-Identifier: GPL-3.0-only */
/* Integration checks of the appended banks and allocator-backed HYBRID voices. */
#define main hostsim_main
#include "hostsim.c"
#undef main

static int render_preset(unsigned preset)
{
    host_tracks_init();
    host_preset(&trk[0], ENGI_HYBRID, preset);
    hy_block(&trk[0]);
    if (!eng_state_prepare(&trk[0])) return 1;
    trk_note_on(&trk[0], 60, 100);
    int32_t out[2 * CTL];
    uint64_t energy = 0;
    for (unsigned block = 0; block < FS / CTL; ++block) {
        mix_block(out, CTL);
        for (unsigned i = 0; i < 2 * CTL; ++i) {
            if (out[i] > 32767 || out[i] < -32768) return 2;
            energy += out[i] < 0 ? -(int64_t)out[i] : out[i];
        }
    }
    trk_note_off(&trk[0], 60);
    return energy > FS ? 0 : 3;
}
int main(void)
{
    int bad = 0, status;
    uint8_t pk[128];
    fm6_factory(24, pk);
    bad += memcmp(pk, FM6_INIT, sizeof pk) != 0;
    for (unsigned i = 0; i < FM6_NSTOCK; ++i) {
        fm6_factory(25 + i, pk);
        bad += memcmp(pk, FM6_STOCK[i], sizeof pk) != 0;
    }
    for (unsigned i = 0; i < FM6_NFACTORY; ++i) {
        fm6_factory(i, pk);
        bad += memcmp(pk, FM6_FACTORY[i], sizeof pk) != 0;
    }
    printf("DX7: existing packed bank and INIT indices preserved; %u stock voices appended\n", FM6_NSTOCK);
    for (unsigned i = 0; i < ENG_HYBRID.npresets; ++i) {
        pid_t pid = fork();
        if (!pid) _exit(render_preset(i));
        if (pid < 0 || waitpid(pid, &status, 0) != pid || !WIFEXITED(status) || WEXITSTATUS(status)) {
            fprintf(stderr, "HYBRID preset %u failed (status %d)\n", i, status); ++bad;
        }
    }
    host_tracks_init();
    for (unsigned p = 0; p < NPART; ++p) {
        host_preset(&trk[p], ENGI_HYBRID, 41 + p);
        hy_block(&trk[p]);
        bad += !eng_state_prepare(&trk[p]);
        for (unsigned q = 0; q < p; ++q) bad += hy_part_of(&trk[p]) == hy_part_of(&trk[q]);
        trk_note_on(&trk[p], 48 + p * 7, 100);
    }
    int32_t out[2 * CTL];
    for (unsigned i = 0; i < FS / CTL; ++i) mix_block(out, CTL);
    for (unsigned p = 0; p < NPART; ++p) {
        bad += memcmp(&hy_cur[p], &HY_FACTORY[HY_PRESETS[41 + p].e[3]], sizeof(hy_patch_t)) != 0;
    }
    printf("HYBRID: %u factory presets, four independent part allocations; %d failures\n", ENG_HYBRID.npresets, bad);
    return bad ? 1 : 0;
}
