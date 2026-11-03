/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Kerem Kilic (Ellic Studio) */
/* Layered, OCT- / OCT+ on Stage move the selected layer's own octave (its TRN): every layer plays the same key at its
 * own octave, the split stays on the key played, and OCT- with OCT+ puts the layer back to 0. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
static uint32_t sounding_note(const track_t *t)
{ for (uint32_t i = 0; i < NVOICE; i++) if (t->v[i].active && t->v[i].gate) return t->v[i].note; return 0; }
static void both_oct(void)
{
    uint32_t m = (1u << panel.btn[B_OCTDN]) | (1u << panel.btn[B_OCTUP]);
    fm1_ms += 320u; fm1_in.buttons |= m; host_pressed |= m; frame(); fm1_in.buttons &= ~m; frame();
}
int main(void)
{
    static int32_t out[4 * CTL];
    ui_power_on(); go_home(); song.ntrk = 4;
    for (uint32_t p = 0; p < 4; p++) { trk[p].p[P_TRANS] = 0; trk[p].p[P_VOICE] = V_POLY; trk[p].p[P_MUTE] = 0; }
    sm_layers = (sm_layers_t){.enabled = 1, .low = {0, 0, 0, 0}, .high = {127, 127, 127, 127}};
    track_select(0); frame(); song.octave = 0;
    press(B_OCTDN); press(B_OCTDN);
    assert(trk[0].p[P_TRANS] == -24 && song.octave == 0 && oct_leds() == 1u);   /* (OCT- lit: this layer is down) */
    press(B_OCTDN);                                     /* (TRN's range: no further) */
    assert(trk[0].p[P_TRANS] == -24);
    track_select(2); frame(); press(B_OCTUP);
    assert(trk[2].p[P_TRANS] == 12 && trk[0].p[P_TRANS] == -24 && !trk[1].p[P_TRANS]);
    fm1_in.notes = 1u << 7;                             /* the C4 key */
    for (uint32_t b = 0; b < 4; b++) mix_block(out, CTL);
    assert(sounding_note(&trk[0]) == 36 && sounding_note(&trk[1]) == 60 && sounding_note(&trk[2]) == 72);   /* (layer 4: the drum kit, its own key map) */
    fm1_in.notes = 0; for (uint32_t b = 0; b < 4; b++) mix_block(out, CTL);
    sm_layers.low[0] = 60; sm_layers.high[1] = 59;      /* a split at C4: layer 1 above, layer 2 below */
    track_select(0); frame();
    fm1_in.notes = 1u << 7;
    for (uint32_t b = 0; b < 4; b++) mix_block(out, CTL);
    assert(sounding_note(&trk[0]) == 36 && !sounding_note(&trk[1]));   /* the split follows the key, not -2 octaves */
    fm1_in.notes = 0; for (uint32_t b = 0; b < 4; b++) mix_block(out, CTL);
    trk[0].p[P_TRANS] = -22; both_oct();
    assert(trk[0].p[P_TRANS] == 0);                     /* back to 0 */
    sm_layers.enabled = 0; song.octave = 0; press(B_OCTUP);
    assert(song.octave == 1 && trk[0].p[P_TRANS] == 0);     /* not layered: the shared octave, as before */
    puts("Layer octaves: OCT-/OCT+ per layer, each layer at its own octave, split on the key, both together reset");
    return 0;
}
