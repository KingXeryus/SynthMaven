/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper 0.4.1 (CLIPPER_SYNTH 1): the fixes of SLOOP 2.6 that apply to Clipper.
 *   LIMITER   it looks one block ahead: a loud attack from silence is already turned down when it comes (no sample
 *             above the threshold through the knee); a held loud bass keeps one gain (no ripple at its pitch); quiet
 *             signals pass bit for bit, one block late
 *   FILTER    the track FILTER / master FILT starting to close under a sound: its low-pass starts where the signal is
 *             (no step to 0)
 *   FM6       a sound changed under held FM6 notes: the old notes fade with their own patch, the new one after
 *   ARP       a latched arp follows TRN, ROOT, SCALE and QNT changed after its keys (#191)
 *   WHEELS    a project load or NEW: the pitch wheel to the middle, no pressure, the pedal up (its notes let go) */
#define main hostsim_main
#include <stddef.h>
#include "hostsim.c"
#undef main

static int fails;
static void ok(int c, const char *what)
{
    printf("sloop26: %-92s %s\n", what, c ? "ok" : "FAIL");
    fails += !c;
}

static void t_limiter(void)
{
    static lim_t s;
    static int32_t l[CTL], r[CTL];
    char w[160];
    uint32_t b, i;
    int32_t pk = 0, g0 = 0, gmin = 1 << 30, gmax = 0, same = 1, late = 1;
    memset(&s, 0, sizeof s);
    s.env = LIM_T, s.g = 32768;
    for (b = 0; b < 40u; b++) {                        /* silence, then a square at 3x the threshold */
        for (i = 0; i < CTL; i++)
            l[i] = r[i] = b < 20u ? 0 : ((i / 8u) & 1u ? -54000 : 54000);
        lim_block(&s, l, r, CTL);
        for (i = 0; i < CTL; i++) {
            int32_t a = l[i] < 0 ? -l[i] : l[i];
            pk = a > pk ? a : pk;
        }
    }
    snprintf(w, sizeof w, "limiter: a loud attack from silence: no sample above the threshold (peak %d, threshold %d)",
             (int)pk, LIM_T);
    ok(pk <= LIM_T + LIM_T / 100, w);
    for (b = 0; b < 600u; b++) {                       /* a held 40 Hz sine at 2x: one gain once settled */
        for (i = 0; i < CTL; i++) {
            double ph = 2 * M_PI * 40 * (double)(b * CTL + i) / FS;
            l[i] = r[i] = (int32_t)(36000 * sin(ph));
        }
        lim_block(&s, l, r, CTL);
        if (b > 300u) {
            gmin = s.g < gmin ? s.g : gmin;
            gmax = s.g > gmax ? s.g : gmax;
        }
    }
    snprintf(w, sizeof w, "limiter: a held loud 40 Hz bass keeps one gain (%d .. %d)", (int)gmin, (int)gmax);
    ok(gmax - gmin <= 1, w);
    memset(&s, 0, sizeof s);
    s.env = LIM_T, s.g = 32768;
    for (b = 0; b < 20u; b++) {                        /* quiet: bit for bit, CTL samples late */
        static int32_t prev[CTL];
        for (i = 0; i < CTL; i++)
            l[i] = r[i] = (int32_t)((b * 977u + i * 131u) % 20000u) - 10000;
        {
            int32_t in[CTL];
            memcpy(in, l, sizeof in);
            lim_block(&s, l, r, CTL);
            for (i = 0; i < CTL; i++)
                same &= b == 0 ? l[i] == 0 : l[i] == prev[i];
            memcpy(prev, in, sizeof prev);
        }
    }
    (void)g0, (void)late;
    ok(same, "limiter: below the threshold the mix bit for bit, one block (0.73 ms) late");
}

static void t_filter(void)
{
    static djf_t f;
    static int32_t b[CTL];
    tsvf_t c;
    uint32_t i;
    int32_t worst = 0;
    char w[160];
    memset(&f, 0, sizeof f);
    for (i = 0; i < CTL; i++)
        b[i] = 12000;                                   /* a sound at rest (DC: the worst case) */
    djf_block(&f, -8, &c);                              /* the FILTER starts to close */
    djf_run(&f, &c, b, CTL, 0);
    for (i = 0; i < CTL; i++) {
        int32_t d = b[i] - 12000;
        d = d < 0 ? -d : d;
        worst = d > worst ? d : worst;
    }
    snprintf(w, sizeof w, "FILTER closing under a sound: the low-pass starts where the signal is (off by %d of 12000)", (int)worst);
    ok(worst < 120, w);
}

static void t_fm6(void)
{
    track_t *t = &trk[0];
    static int32_t o[CTL * 2];
    uint32_t i, old, waited = 1;
    host_preset(t, ENGI_FM6, 0);
    for (i = 0; i < 40u; i++)
        mix_block(o, CTL);
    fm6_poll();
    old = fm6_slot[0];
    trk_note_on(t, 60, 100);
    for (i = 0; i < 10u; i++)
        mix_block(o, CTL);
    sound_fade_begin(t);                               /* a new sound (its PTCH another patch) under the held note */
    t->p[P_E7] = (int16_t)(old ? old - 1u : old + 1u);
    fm6_track_loaded(t);
    ok(fm6_slot[0] == old && ((fm6_pend & 1u) != 0), "FM6: a sound changed under a held note: its patch waits");
    for (i = 0; i < XF_BLOCKS + 2u; i++) {
        mix_block(o, CTL);
        if (t->xf_on && t->xf)
            fm6_poll(), waited &= fm6_slot[0] == old;  /* (the main loop polls meanwhile: still the old one) */
    }
    fm6_poll();
    mix_block(o, CTL);
    ok(waited && fm6_slot[0] == (uint32_t)t->p[P_E7] && !(fm6_pend & 1u) && !t->xf_on,
       "FM6: the old note fades with its own patch, then the new patch (and the new notes)");
    trk_all_off(t);
}

/* (0.7: the arp's next note through arp_list, SLOOP 2.7's: the held notes as they play now, the first of them) */
static uint32_t arp_first(track_t *t)
{
    uint32_t l[65], c;
    return arp_list(t, l, &c) ? l[0] : KB_SILENT;
}

static void t_arp(void)
{
    track_t *t = &trk[0];
    uint32_t k = 12, n, x;                              /* a white key */
    host_preset(t, 0u, 0);
    t->p[P_QUANT] = 2, t->p[P_SCALE] = 0, t->p[P_ROOT] = 0, t->p[P_TRANS] = 0, t->p[P_AMODE] = 1, t->p[P_AHOLD] = 1;
    t->p[P_AOCT] = 1, t->p[P_AORDER] = 0;
    t->nheld = 0, t->arp_phys = 0;
    n = kb_pitch(t, k, 0);
    kb_root[k] = (uint8_t)n, kb_oct[k] = 0, in_key = (uint8_t)(k + 1u);
    arp_add(t, n);
    in_key = 0;
    arp_remove(t, n);                                   /* (let go: latched) */
    t->p[P_TRANS] = 3;
    x = arp_first(t);
    ok(x == n + 3u, "ARP (#191): a latched note follows TRN changed after it (+3)");
    t->p[P_TRANS] = 0, t->p[P_ROOT] = 2;
    x = arp_first(t);
    ok(x == kb_pitch(t, k, 0) && x != n, "ARP (#191): and ROOT (the key walks the new scale)");
    t->p[P_AMODE] = 0, t->p[P_AHOLD] = 0, t->p[P_ROOT] = 0, t->p[P_QUANT] = 0;
    t->nheld = 0;
}

static void t_wheels(void)
{
    track_t *t = &trk[0];
    static int32_t o[CTL * 2];
    uint32_t i, held = 0;
    host_preset(t, 0u, 0);
    t->p[P_VOICE] = V_POLY;
    part_ctl[0].bend = 9000, part_ctl[0].at = 90, part_ctl[0].mw = 70;
    part_ctl_cc(t, 64, 127);                            /* the pedal down */
    trk_note_on(t, 64, 100);
    trk_note_off(t, 64);                                /* held by the pedal */
    mix_block(o, CTL);
    part_ctl_reset();
    mix_block(o, CTL);
    for (i = 0; i < NVOICE; i++)
        held += t->v[i].active && t->v[i].gate;
    ok(part_ctl[0].bend == 0 && part_ctl[0].at == 0 && part_ctl[0].mw == 70 && !part_ctl[0].sus && !held,
       "a project load / NEW: the pitch wheel to the middle, no pressure, the pedal up (its note lets go), MW kept");
}

int main(void)
{
    uint32_t i;
    host_tracks_init();
    for (i = 0; i < NPART; i++)
        host_preset(&trk[i], i ? 1u : 0u, 0);
    clipper_proj_neutral();
    trk[1].p[P_LYMODE] = LY_OFF;
    t_limiter();
    t_filter();
    t_fm6();
    t_arp();
    t_wheels();
    printf(fails ? "sloop26: %d FAILED\n" : "sloop26: all passed\n", fails);
    return fails;
}
