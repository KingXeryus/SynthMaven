/* SPDX-License-Identifier: GPL-3.0-only */
/* The factory bank: layering starts off, Layer 1 leads, each support layer is audible, nothing clips, and every program has its chorus and reverb. */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
#include <assert.h>
#include <math.h>
static void measure(uint32_t mask, double *peak, double *rms)
{
    static const uint8_t chord[4] = {48, 60, 64, 67};
    int32_t out[CTL * 2]; double sum = 0, top = 0; uint64_t n = 0;
    panic_req |= 15u; for (uint32_t b = 0; b < 8000u / CTL + 1u; b++) { memset(out, 0, sizeof out); mix_block(out, CTL); }
    for (uint32_t k = 0; k < 4; k++) if (mask & (1u << k))
        for (uint32_t i = 0; i < 4; i++) trk_note_on(&trk[k], chord[i] + (uint32_t)(int32_t)trk[k].p[P_TRANS], 100);
    for (uint32_t b = 0; b < 88200u / CTL; b++) {
        memset(out, 0, sizeof out); mix_block(out, CTL);
        for (uint32_t i = 0; i < CTL * 2u; i++) { double v = fabs((double)out[i] / 32768.0); if (v > top) top = v; sum += v * v; n++; }
    }
    for (uint32_t k = 0; k < 4; k++) for (uint32_t i = 0; i < 4; i++) trk_note_off(&trk[k], chord[i] + (uint32_t)(int32_t)trk[k].p[P_TRANS]);
    *peak = top; *rms = sqrt(sum / (double)n);
}
int main(void)
{
    reset(); settings_budget = 64;
    for (uint32_t s = 0; s < 8; s++) {
        double lead_peak, lead_rms, all_peak, all_rms; char name[13]; assert(!sm_program_load(s)); assert(!sm_layers.enabled && song.sel == 0);
        memcpy(name, sm_program_name, 13);
        measure(1u, &lead_peak, &lead_rms); assert(!sm_program_load(s)); measure(15u, &all_peak, &all_rms);
        for (uint32_t k = 1; k < 4; k++) { double a, b; assert(!sm_program_load(s)); sm_layers.enabled = 1; measure(1u << k, &a, &b); printf("   L%u eng %u rms %.3f peak %.3f\n", k + 1, trk[k].eng_req, b, a); assert(b > lead_rms * 0.08); }
        printf("P%u %-12s lead peak %.3f rms %.3f | layered peak %.3f rms %.3f (%+.1f dB)\n", s + 1, name, lead_peak, lead_rms, all_peak, all_rms, 20 * log10(all_rms / lead_rms));
        assert(lead_rms > 0.05 && lead_peak < 0.98 && all_peak < 1.0 && all_rms > lead_rms * 0.8);
        assert(!sm_program_load(s) && song.g[G_MFXTYPE] == FX_CHORUS && song.g[G_MFX2TYPE] == FX_REVERB);
    }
    puts("Factory bank: layering off, audible leads, layered stacks measured"); return 0;
}
