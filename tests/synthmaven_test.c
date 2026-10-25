/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static void midi_in(uint32_t st,uint32_t a,uint32_t b) {
 midi_in_q[mi_w++ % MQ] = (st>>4) | st<<8 | a<<16 | b<<24;
}
static uint32_t voices_part(uint32_t p) {
 uint32_t i,n=0; for(i=0;i<NVOICE;i++) n += trk[p].v[i].active && trk[p].v[i].gate; return n;
}
int main(void) {
    uint32_t total, before;
    outdir = "build/host"; bank_resolve(); panel = PANEL_DEFAULT; layers_init(); host_tracks_init();
    midi_in_ch = 0; midi_pc_off = 0; go_home(); frames(2);
    check(sm_set(0,1,0) == 0 && sm_set(1,1,0) == 4 && sm_patch_port == 0,
          "unavailable Bluetooth rejected atomically");
    check(sm_set(3,1,0) == 1 && sm_key_port == 1, "invalid routing rejected atomically");
    before = base_pos(&total);
    midi_in(0x90,60,100); frames(3);
    check(!voices_part(0), "TRS-only keyboard ignores USB note-on");
    midi_in_q[mi_w++ % MQ] = 0x19u | 0x90u << 8 | 60u << 16 | 100u << 24; frames(3);
    check(voices_part(0) > 0, "TRS-only keyboard accepts TRS note-on");
    sm_set(0,0,0);
    midi_in_q[mi_w++ % MQ] = 0x18u | 0x80u << 8 | 60u << 16; frames(30);
    check(!voices_part(0), "note-off from previous keyboard port is processed after source change");
    test_up_loaded = UP_SLOTS;
    midi_in_q[mi_w++ % MQ] = 0x1bu | 0xb0u << 8 | 127u << 24;
    midi_in_q[mi_w++ % MQ] = 0x1cu | 0xc0u << 8; frames(2);
    check(test_up_loaded == UP_SLOTS, "USB-only patch switching ignores TRS program changes");
    midi_in(0xb0,0,127); midi_in(0xc0,0,0); frames(2);
    check(test_up_loaded == 0, "USB patch switching loads direct user slot");
    midi_pc_off = 1; test_up_loaded = UP_SLOTS;
    midi_in(0xc0,1,0); frames(2);
    check(test_up_loaded == UP_SLOTS, "patch switching OFF ignores program changes");
    { int16_t *v; const param_desc_t *d = syn_desc(X_SMPORT,&v);
      check(d->min == d->max, "pedal source cascades from patch enable switch"); }
    return fails != 0;
}
