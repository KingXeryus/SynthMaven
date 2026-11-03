/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Kerem Kilic (Ellic Studio) */
/* A held chord on layer 1 does not duck as layers 2..4 join it: at full volume the master limiter stays near idle
 * (the layered trim, synthmaven_layers.h). */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
#include <assert.h>
#include <math.h>
int main(void)
{
    static const uint8_t chord[4] = {48, 60, 64, 67};
    static const uint8_t PROGS[] = {1, 2, 7};             /* P2, P3, P8 (P7, the hottest, still pumps ~4 dB) */
    int32_t out[CTL * 2];
    reset(); settings_budget = 64;
    for (uint32_t pi = 0; pi < sizeof PROGS; pi++) {
        uint32_t s = PROGS[pi];
        assert(!sm_program_load(s) && sm_layers.enabled);
        panic_req |= 15u; for (uint32_t b = 0; b < 400u; b++) mix_block(out, CTL);
        for (uint32_t k = 0; k < 4; k++) {
            for (uint32_t i = 0; i < 4; i++) trk_note_on(&trk[k], chord[i] + (uint32_t)(int32_t)trk[k].p[P_TRANS], 100);
            double s2 = 0, worst = 0;
            for (uint32_t b = 0; b < FS / CTL; b++) {
                mix_block(out, CTL);
                for (uint32_t i = 0; i < 2u * CTL; i++) s2 += (double)out[i] * out[i];
                double g = lim_env > LIM_T ? 20 * log10((double)LIM_T / lim_env) : 0;
                if (g < worst) worst = g;
            }
            double rms = sqrt(s2 / (FS / CTL * 2.0 * CTL));
            printf("P%u +L%u: rms %.0f, limiter %.1f dB\n", s + 1, k + 1, rms, worst);
            assert(worst > -1.5);                         /* (before the trim: -4 .. -11 dB) */
        }
        for (uint32_t k = 0; k < 4; k++) for (uint32_t i = 0; i < 4; i++) trk_note_off(&trk[k], chord[i] + (uint32_t)(int32_t)trk[k].p[P_TRANS]);
    }
    puts("Layered chords: no limiter ducking as layers join");
    return 0;
}
