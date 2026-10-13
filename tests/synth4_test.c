/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper 0.4 (CLIPPER_SYNTH 1): the modulation matrix on every engine, LFO 2, ENV 2, MPE, and where they are kept.
 *   MATRIX    LFO 2 > CUT sweeps the brightness; ENV 2 > PITCH starts high and settles; KEY > AMP: low notes louder
 *             than high ones; RAND > PITCH: each note its own pitch; VEL > an EDIT value (ANALOG CUT): a hard note
 *             brighter, the stored value untouched; MW > PAN: to the right; MW > LFO 2 RATE: faster; LFO 2 KEY:
 *             restarted by each note; an AMT of 0: bit for bit as no slot; every engine with all six slots in use:
 *             bounded, finite
 *   MPE       a member channel's bend moves its own note only (48 semitones), the master channel's every note; its
 *             pressure (AT > CUT) and its CC 74 (SLIDE, to the cutoff) its own note; MPE OFF: as before (omni)
 *   PROJECT   LFO 2, ENV 2 and the matrix of tracks 1 and 2 kept in track 3's free lock slots (the size unchanged, SLOOP
 *             sees tracks 3 and 4 as they are), the EDIT values and locks where they were; 0.4's first build's projects
 *             and Clipper 0.3's read; a SLOOP 2.6 project (FUN6) read
 *   PRESETS   one bank for SLOOP and Clipper: 192-byte records, LFO 2 / ENV 2 / matrix packed in the spare values; a
 *             SLOOP 2.6 record read; Clipper 0.4's first bank (232-byte records) converted */
#define main hostsim_main
#include <stddef.h>
#include "hostsim.c"
#undef main
#define PROJ_HOST 1
static const uint8_t TRK_DEF[NPART][2] = {{ENGI_HYBRID, 0}, {1, 0}, {4, 5}};   /* (ui.c, Clipper) */
static uint32_t trk_def_engine(uint32_t i) { return i < NPART ? TRK_DEF[i][0] : 0u; }
#include "../firmware/src/project.c"
#define UP_HOST 1
#include "../firmware/src/upreset.c"

static int fails;
static void ok(int c, const char *what)
{
    printf("synth4: %-92s %s\n", what, c ? "ok" : "FAIL");
    fails += !c;
}

/* ---- rendering */
static double cl[4096 * 8], cr[4096 * 8];
static uint32_t ncap;
static void run(uint32_t nb)                         /* blocks of the mix, kept from 0 */
{
    static int32_t o[CTL * 2];
    uint32_t b, i;
    ncap = 0;
    for (b = 0; b < nb; b++) {
        mix_block(o, CTL);
        for (i = 0; i < CTL; i++)
            if (ncap < 4096u * 8u) {
                cl[ncap] = o[2 * i];
                cr[ncap++] = o[2 * i + 1];
            }
    }
}
static double rms(uint32_t a, uint32_t n)
{
    double e = 0;
    uint32_t i;
    for (i = a; i < a + n && i < ncap; i++)
        e += cl[i] * cl[i];
    return sqrt(e / n);
}
static double bright(uint32_t a, uint32_t n)
{
    double e = 0, d = 0;
    uint32_t i;
    for (i = a + 1; i < a + n && i < ncap; i++)
        e += cl[i] * cl[i], d += (cl[i] - cl[i - 1]) * (cl[i] - cl[i - 1]);
    return sqrt(d / (e + 1e-9));
}
static double freq(uint32_t a, uint32_t n)           /* rising zero crossings (low-passed) a second */
{
    double lp = 0, prev = 0, first = -1, last = -1, k = 0;
    uint32_t i;
    for (i = a; i < a + n && i < ncap; i++) {
        lp += (cl[i] - lp) * 0.2;
        if (prev < 0 && lp >= 0) {
            if (first < 0)
                first = i;
            else
                k++;
            last = i;
        }
        prev = lp;
    }
    return k > 0 ? k * FS / (last - first) : 0;
}
static void quiet(void)
{
    uint32_t i;
    for (i = 0; i < NPART; i++)
        trk_all_off(&trk[i]);
    run(400);
    for (i = 0; i < NPART; i++) {                    /* (long releases: gone, as engine_block ends them before a
                                                      * switch; host_preset switches at once) */
        uint32_t k;
        for (k = 0; k < NVOICE; k++)
            trk[i].v[k].active = trk[i].v[k].gate = 0, trk[i].v[k].stage = 0, trk[i].v[k].env = trk[i].v[k].env_out = 0;
    }
}
static void slots_off(track_t *t)
{
    uint32_t k;
    for (k = 0; k < MX_SLOTS; k++)
        t->p[P_M1SRC + 3u * k] = MS_OFF, t->p[P_M1DST + 3u * k] = MT_OFF, t->p[P_M1AMT + 3u * k] = 0;
}
static void slot(track_t *t, uint32_t k, uint32_t src, uint32_t dst, int32_t amt)
{
    t->p[P_M1SRC + 3u * k] = (int16_t)src, t->p[P_M1DST + 3u * k] = (int16_t)dst, t->p[P_M1AMT + 3u * k] = (int16_t)amt;
}
/* ANALOG, plain: a saw (E0 0) or a sine (3) through the filter at cut, no effects, no modulation, full sustain */
static void analog(uint32_t wave, int32_t cut)
{
    track_t *t = &trk[0];
    quiet();
    host_preset(t, 0u, 0);
    t->p[P_DIST] = t->p[P_CHOR] = t->p[P_DLY] = t->p[P_REV] = 0;
    t->p[P_E0] = (int16_t)wave, t->p[P_E2] = 0, t->p[P_E3] = 0, t->p[P_E4] = (int16_t)cut;
    t->p[P_ED_PIT] = t->p[P_LD_PIT] = t->p[P_LD_FLT] = t->p[P_LD_SHP] = t->p[P_LD_AMP] = t->p[P_ED_FLT] = 0;
    t->p[P_ATK] = 0, t->p[P_SUS] = 127, t->p[P_REL] = 10, t->p[P_VELF] = 0, t->p[P_VOICE] = V_POLY;
    t->p[P_MWDST] = t->p[P_ATDST] = MD_OFF, t->p[P_PAN] = 0, t->p[P_GLIDE] = 0;
    slots_off(t);
    part_ctl[0].mw = part_ctl[0].at = 0, part_ctl[0].bend = 0;
    run(20);
}

static void t_matrix(void)
{
    track_t *t = &trk[0];
    char w[200];
    double a, b, lo, hi;
    uint32_t i;
    /* LFO 2 > CUT */
    analog(0, 40);
    t->p[P_L2RATE] = 90, t->p[P_L2WAVE] = 0, t->p[P_L2MODE] = L2_FREE;
    trk_note_on(t, 48, 100), run(1000);
    lo = 1e9, hi = 0;
    for (i = 0; i < 16u; i++) {
        double x = bright(4096 + i * 1024, 1024);
        lo = x < lo ? x : lo, hi = x > hi ? x : hi;
    }
    a = hi / lo;
    slot(t, 0, MS_LFO2, MT_CUT, 63), run(1000);
    lo = 1e9, hi = 0;
    for (i = 0; i < 16u; i++) {
        double x = bright(4096 + i * 1024, 1024);
        lo = x < lo ? x : lo, hi = x > hi ? x : hi;
    }
    trk_note_off(t, 48);
    snprintf(w, sizeof w, "LFO 2 > CUT: the brightness sweeps (max / min %.2f; no slot %.2f)", hi / lo, a);
    ok(hi / lo > 1.8 && a < 1.15, w);
    /* ENV 2 > PITCH */
    analog(3, 127);
    t->p[P_E2ATK] = 0, t->p[P_E2DEC] = 60, t->p[P_E2SUS] = 0;
    slot(t, 0, MS_ENV2, MT_PITCH, 40);
    trk_note_on(t, 57, 100), run(800);
    a = freq(0, 600), b = freq(20000, 4000);
    trk_note_off(t, 57);
    snprintf(w, sizeof w, "ENV 2 > PITCH: starts high, settles on the note (%.0f Hz, then %.0f Hz; A3 220)", a, b);
    ok(a > b * 1.25 && fabs(b - 220) < 4, w);
    /* KEY > AMP */
    analog(3, 127);
    trk_note_on(t, 36, 100), run(300), a = rms(4096, 4096), trk_note_off(t, 36), quiet();
    trk_note_on(t, 84, 100), run(300), b = rms(4096, 4096), trk_note_off(t, 84), quiet();
    lo = a / b;
    slot(t, 0, MS_KEY, MT_AMP, -63);
    trk_note_on(t, 36, 100), run(300), a = rms(4096, 4096), trk_note_off(t, 36), quiet();
    trk_note_on(t, 84, 100), run(300), b = rms(4096, 4096), trk_note_off(t, 84), quiet();
    snprintf(w, sizeof w, "KEY > AMP -63: C2 against C6 x%.2f louder than without (x%.2f)", (a / b) / lo, lo);
    ok((a / b) / lo > 1.8, w);
    /* RAND > PITCH */
    slots_off(t);
    slot(t, 0, MS_RAND, MT_PITCH, 24);
    trk_note_on(t, 69, 100), run(300), a = freq(2048, 8192), trk_note_off(t, 69), quiet();
    trk_note_on(t, 69, 100), run(300), b = freq(2048, 8192), trk_note_off(t, 69), quiet();
    snprintf(w, sizeof w, "RAND > PITCH: the same key twice, two pitches (%.1f Hz, %.1f Hz)", a, b);
    ok(fabs(a - b) > 1.0 && a > 350 && a < 540 && b > 350 && b < 540, w);   /* (AMT 24: up to +-3.4 semitones) */
    /* VEL > an EDIT value: ANALOG CUT (E4) */
    analog(0, 30);
    slot(t, 0, MS_VEL, MT_E1 + 4u, 63);
    trk_note_on(t, 48, 20), run(200), a = bright(2048, 2048), trk_note_off(t, 48), quiet();
    trk_note_on(t, 48, 127), run(200), b = bright(2048, 2048);
    snprintf(w, sizeof w, "VEL > CUT (ANALOG's EDIT): a hard note brighter (%.3f vs %.3f), CUT stored still 30", b, a);
    ok(b > a * 1.5 && t->p[P_E4] == 30, w);
    trk_note_off(t, 48);
    /* MW > PAN */
    analog(3, 127);
    slot(t, 0, MS_MW, MT_PAN, 63);
    part_ctl[0].mw = 127;
    trk_note_on(t, 60, 100), run(400);
    {
        double el = 0, er = 0;
        for (i = 6000; i < 12000u; i++)
            el += cl[i] * cl[i], er += cr[i] * cr[i];
        snprintf(w, sizeof w, "MW > PAN: wheel up, to the right (L/R %.3f, panmod %d)", sqrt(el / (er + 1)), t->panmod);
        ok(sqrt(el / (er + 1)) < 0.1 && t->panmod > 56, w);
    }
    trk_note_off(t, 60);
    part_ctl[0].mw = 0;
    /* MW > LFO 2 RATE */
    analog(3, 127);
    slot(t, 0, MS_MW, MT_L2RATE, 63);
    t->p[P_L2RATE] = 40;
    {
        uint32_t p0, d0, d1;
        run(1), p0 = t->l2ph, run(1), d0 = t->l2ph - p0;
        part_ctl[0].mw = 127;
        run(1), p0 = t->l2ph, run(1), d1 = t->l2ph - p0;
        snprintf(w, sizeof w, "MW > LFO 2 RATE: the wheel up, LFO 2 faster (x%.1f)", (double)d1 / (d0 + 1));
        ok(d1 > d0 * 4u, w);
        part_ctl[0].mw = 0;
    }
    /* LFO 2 KEY */
    slots_off(t);
    t->p[P_L2MODE] = L2_KEY, t->p[P_L2RATE] = 60;
    slot(t, 0, MS_LFO2, MT_CUT, 10);
    trk_note_on(t, 60, 100), run(50);
    {
        uint32_t ph = 0;
        for (i = 0; i < NVOICE; i++)
            if (t->v[i].active && t->v[i].gate)
                ph = t->v[i].l2ph;
        trk_note_on(t, 64, 100), run(1);
        for (i = 0; i < NVOICE; i++)
            if (t->v[i].active && t->v[i].gate && t->v[i].note == 64)
                b = t->v[i].l2ph;
        snprintf(w, sizeof w, "LFO 2 KEY: each note its own, from its start (held %u, new %u)", ph, (uint32_t)b);
        ok(ph > (uint32_t)b * 20u && (uint32_t)b == LFO_INC[60], w);
    }
    trk_note_off(t, 60), trk_note_off(t, 64);
    t->p[P_L2MODE] = L2_FREE;
    /* an AMT of 0: bit for bit as no slot */
    {   /* the part rendered twice from the same state */
        static track_t save;
        static int32_t ref[CTL * 128], out[CTL * 128];
        uint32_t same = 1, rs;
        analog(0, 60);
        t->p[P_LD_FLT] = 20;
        trk_note_on(t, 50, 100), run(2);
        save = *t, rs = rng_state;
        for (i = 0; i < 128u; i++)
            track_render(t, ref + i * CTL, CTL);
        *t = save, rng_state = rs;
        slot(t, 0, MS_LFO1, MT_CUT, 0), slot(t, 1, MS_ENV2, MT_E1 + 4u, 0), slot(t, 2, MS_LFO2, MT_PITCH, 0);
        for (i = 0; i < 128u; i++)
            track_render(t, out + i * CTL, CTL);
        for (i = 0; i < CTL * 128u; i++)
            same &= ref[i] == out[i];
        ok(same, "slots with AMT 0: the sound bit for bit as with none");
        trk_note_off(t, 50);
    }
    /* every engine, all six slots in use (every destination), chords: bounded, finite */
    {
        uint32_t e, k, bad = 0;
        double pk = 0;
        for (e = 0; e < NENGINES; e++) {
            const engine_t *en = ENGINES[e];
            uint32_t pi;
            if (!en->npresets)
                continue;
            for (pi = 0; pi < en->npresets; pi += en->npresets / 3u + 1u) {
                quiet();
                host_preset(t, e, pi);
                slot(t, 0, MS_LFO2, MT_PITCH, 30);
                slot(t, 1, MS_ENV2, MT_CUT, 63);
                slot(t, 2, MS_RAND, MT_SHAPE, -64);
                slot(t, 3, MS_KEY, MT_E1 + (pi % 8u), 63);
                slot(t, 4, MS_VEL, MT_E1 + ((pi + 3u) % 8u), -64);
                slot(t, 5, MS_LFO1, MT_AMP, 63);
                t->p[P_L2RATE] = 100, t->p[P_L2WAVE] = 4;
                for (k = 0; k < 4u; k++)
                    trk_note_on(t, 36u + k * 9u, 60u + k * 20u);
                run(300);
                for (k = 0; k < ncap; k++) {
                    double x = fabs(cl[k]) > fabs(cr[k]) ? fabs(cl[k]) : fabs(cr[k]);
                    if (x != x)
                        bad++;
                    pk = x > pk ? x : pk;
                }
                for (k = 0; k < 8u; k++)
                    if (t->p[P_E0 + k] < en->edit[k].min || t->p[P_E0 + k] > en->edit[k].max)
                        bad++;
            }
        }
        quiet();
        slots_off(t);
        snprintf(w, sizeof w, "every engine, six slots on every destination, chords: bounded (peak %.0f), EDIT values in range",
                 pk);
        ok(!bad && pk < 32768, w);
    }
}

static void midi(uint32_t st, uint32_t d1, uint32_t d2)   /* a USB-MIDI packet, as usb.c queues it */
{
    midi_in_q[mi_w % MQ] = (st >> 4) | st << 8 | d1 << 16 | d2 << 24;
    mi_w++;
}
static void bend(uint32_t ch, int32_t v14) { midi(0xE0u | ch, (uint32_t)v14 & 127u, (uint32_t)v14 >> 7); }

static void t_mpe(void)
{
    track_t *t = &trk[0];
    char w[200];
    double a, b, c;
    uint32_t i, ch1 = 0, ch2 = 0;
    analog(3, 127);
    mpe_mode = 1;                                      /* 48 semitones */
    mpe_reset();
    midi(0x91, 57, 100), midi(0x92, 64, 100), run(4);
    for (i = 0; i < NVOICE; i++)
        if (t->v[i].active && t->v[i].gate)
            ch1 |= t->v[i].note == 57 && t->v[i].ch == 1, ch2 |= t->v[i].note == 64 && t->v[i].ch == 2;
    ok(ch1 && ch2, "MPE: each note keeps its channel (2 and 3)");
    midi(0x81, 57, 0), midi(0x82, 64, 0), quiet();
    midi(0x91, 57, 100), run(200), a = freq(2048, 4096);
    bend(1, 8192 + 2048);                              /* + 12 of 48 semitones */
    run(200), b = freq(2048, 4096);
    midi(0x81, 57, 0), quiet();
    midi(0x92, 57, 100), run(200), c = freq(2048, 4096);   /* channel 3: its own bend, none */
    midi(0x82, 57, 0), quiet();
    snprintf(w, sizeof w, "MPE: channel 2's bend +12 st moves its note (%.0f -> %.0f Hz), channel 3's note not (%.0f)", a, b, c);
    ok(fabs(b - 2 * a) < 6 && fabs(c - a) < 3 && fabs(a - 220) < 3, w);
    bend(1, 8192);
    bend(0, 8192 + 4096);                              /* the master channel: every note, +-2 semitones */
    midi(0x92, 57, 100), run(200), b = freq(2048, 4096), midi(0x82, 57, 0), quiet();
    bend(0, 8192);
    snprintf(w, sizeof w, "MPE: the master channel's bend moves every note (+1 st: %.1f Hz)", b);
    ok(fabs(b - 220 * pow(2, 1 / 12.0)) < 3, w);
    /* pressure: AT > CUT */
    analog(0, 30);
    t->p[P_ATDST] = MD_CUT;
    midi(0xD1, 127, 0), midi(0xD2, 0, 0);
    midi(0x92, 48, 100), run(200), a = bright(2048, 2048), midi(0x82, 48, 0), quiet();
    midi(0x91, 48, 100), run(200), b = bright(2048, 2048), midi(0x81, 48, 0), quiet();
    snprintf(w, sizeof w, "MPE: channel 2's pressure opens its note's filter, channel 3's not (%.3f vs %.3f)", b, a);
    ok(b > a * 1.5, w);
    midi(0xD1, 0, 0);
    t->p[P_ATDST] = MD_OFF;
    /* SLIDE (CC 74): to the cutoff when no slot takes it; as a source */
    midi(0xB1, 74, 127);
    midi(0x92, 48, 100), run(200), a = bright(2048, 2048), midi(0x82, 48, 0), quiet();
    midi(0x91, 48, 100), run(200), b = bright(2048, 2048), midi(0x81, 48, 0), quiet();
    snprintf(w, sizeof w, "MPE: CC 74 on channel 2 (SLIDE) opens its note's filter (%.3f vs %.3f); TFLT untouched", b, a);
    ok(b > a * 1.5 && t->p[P_TFLT] == 0, w);
    slot(t, 0, MS_SLIDE, MT_PITCH, 30);
    analog(3, 127);
    slot(t, 0, MS_SLIDE, MT_PITCH, 30);
    midi(0x91, 57, 100), run(200), b = freq(2048, 4096), midi(0x81, 57, 0), quiet();
    snprintf(w, sizeof w, "MPE: SLIDE as a matrix source (> PITCH: %.0f Hz, above 220)", b);
    ok(b > 240, w);
    midi(0xB1, 74, 64);
    /* MPE OFF: the channels are one (omni), the bend the part's */
    mpe_mode = 0;
    slots_off(t);
    bend(1, 8192 + 4096);
    run(1);
    ok(part_ctl[0].bend == 4096 * 4 && mpe_bend[1] == 0, "MPE OFF: a bend on channel 2 is the part's (as ever)");
    midi(0x92, 57, 100), run(4);
    for (i = 0, ch1 = 0; i < NVOICE; i++)
        ch1 |= t->v[i].active && t->v[i].gate && t->v[i].ch != 0;
    ok(!ch1, "MPE OFF: no note keeps a channel");
    midi(0x82, 57, 0), bend(1, 8192), quiet();
}

static project_t pj;
/* a value for track j's parameter k of LFO 2 / ENV 2 / the matrix, in its range */
static int16_t xval(uint32_t j, uint32_t k)
{
    const param_desc_t *d = &TP[PROJ_E0 + k];
    return (int16_t)(d->min + (int32_t)(k * (j ? 5u : 7u) + (j ? 1u : 3u)) % (d->max - d->min + 1));
}
static void t_project(void)
{
    track_t *a = &trk[0], *b = &trk[1];
    uint32_t k, same = 1, e_ok = 1, free3 = 1;
    int16_t ea[8], t3[PROJ_E0];
    quiet();
    host_preset(a, 0u, 2);
    host_preset(b, ENGI_WAVE, 1);
    for (k = 0; k < PROJ_NX; k++)                      /* every LFO 2, ENV 2 and matrix value, in its range */
        a->p[PROJ_E0 + k] = xval(0, k), b->p[PROJ_E0 + k] = xval(1, k);
    for (k = 0; k < 8u; k++)
        ea[k] = a->p[P_E0 + k];
    for (k = 0; k < PROJ_E0; k++)
        t3[k] = trk[2].p[k];
    for (k = 0; k < NLOCK; k++)
        a->lock[k].step = LOCK_FREE;
    a->lock[0].step = 3, a->lock[0].param = P_E0 + 4u, a->lock[0].val = 77;
    a->lock[1].step = 5, a->lock[1].param = P_LD_FLT, a->lock[1].val = -20;
    proj_capture(&pj);
    for (k = 0; k < NLOCK; k++)
        free3 &= pj.t[2].lock[k].step == LOCK_FREE;
    for (k = 0; k < PROJ_E0; k++)
        same &= pj.t[2].p[k] == t3[k];
    ok(sizeof pj == 3840u && pj.rsv[0] == CLIPPER_PROJ_MARK && free3 && same && pj.t[0].p[PROJ_E0 + 4u] == a->p[P_E4] &&
       pj.t[0].lock[0].param == PROJ_E0 + 4u,
       "project: LFO 2 / ENV 2 / matrix in track 3's free lock slots (SLOOP passes them over), its sound as it is");
    for (k = 0; k < PROJ_NX; k++)
        a->p[PROJ_E0 + k] = TP[PROJ_E0 + k].def, b->p[PROJ_E0 + k] = TP[PROJ_E0 + k].def;
    for (k = 0; k < 8u; k++)
        a->p[P_E0 + k] = 0;
    proj_apply(&pj, 1);
    for (k = 0, same = 1; k < PROJ_NX; k++)
        same &= a->p[PROJ_E0 + k] == xval(0, k) && b->p[PROJ_E0 + k] == xval(1, k);
    for (k = 0; k < 8u; k++)
        e_ok &= a->p[P_E0 + k] == ea[k];
    ok(same && e_ok && a->lock[0].param == P_E0 + 4u && a->lock[0].val == 77 && a->lock[1].param == P_LD_FLT,
       "project: back as they were (LFO 2, ENV 2, matrix, EDIT values, an EDIT lock, a common lock)");
    for (k = 0, free3 = 1; k < NLOCK; k++)
        free3 &= trk[2].lock[k].step == LOCK_FREE;
    ok(free3 && trk[2].p[P_LEVEL] == t3[P_LEVEL] && trk[3].p[0] == TP[0].def, "project: tracks 3 and 4 as they were, no lock");
    {   /* as SLOOP reads it (format 5, every lock checked as proj_apply does): track 3 has no lock, its sound its own */
        uint32_t used = 0;
        for (k = 0; k < NLOCK; k++)
            used += pj.t[2].lock[k].step < NSTEP;
        ok(!used && pj.t[2].p[P_LEVEL] == t3[P_LEVEL], "project in SLOOP: track 3 with no lock, its own LEVEL");
    }
    {   /* the first 0.4 build's projects (0xC4: in tracks 3 and 4's p[]) */
        proj_capture(&pj);
        pj.rsv[0] = CLIPPER_PROJ_MARK_04;
        for (k = 0; k < PROJ_NX; k++)
            pj.t[2].p[k] = xval(1, k), pj.t[3].p[k] = xval(0, k);   /* (swapped: tell them apart) */
        proj_apply(&pj, 1);
        for (k = 0, same = 1; k < PROJ_NX; k++)       /* (0.6's INSERT: not in them, at its defaults) */
            same &= k < P_ITYPE - PROJ_E0 ? a->p[PROJ_E0 + k] == xval(1, k) && b->p[PROJ_E0 + k] == xval(0, k)
                                          : a->p[PROJ_E0 + k] == TP[PROJ_E0 + k].def && b->p[PROJ_E0 + k] == TP[PROJ_E0 + k].def;
        ok(same && trk[2].p[0] == TP[0].def, "a project of 0.4's first build (tracks 3 and 4's p[]): read as it was");
    }
    proj_capture(&pj);
    pj.rsv[0] = 0xC3;                                  /* as Clipper 0.3 wrote it: tracks 3 and 4 its own values */
    for (k = 0; k < PROJ_NX; k++)
        pj.t[2].p[k] = TP[k].max, pj.t[3].p[k] = TP[k].min;
    proj_apply(&pj, 1);
    for (k = 0, same = 1; k < PROJ_NX; k++)
        same &= a->p[PROJ_E0 + k] == TP[PROJ_E0 + k].def && b->p[PROJ_E0 + k] == TP[PROJ_E0 + k].def;
    for (k = 0, e_ok = 1; k < 8u; k++)
        e_ok &= a->p[P_E0 + k] == ea[k];
    ok(same && e_ok && trk[2].p[0] == TP[0].max,
       "a Clipper 0.3 project: no LFO 2 / ENV 2 / matrix (defaults), its EDIT values and tracks 3, 4 as they were");
}

/* a SLOOP 2.6 project (format 6, 3816 bytes) as SLOOP 2.6 writes it */
static project_v6_t p6;
static void t_fun6(void)
{
    project_t *q = &proj_slot[0];
    uint32_t i, k, good = 1;
    memset(&p6, 0, sizeof p6);
    p6.magic = PROJ_MAGIC_V6, p6.size = sizeof p6;
    for (i = 0; i < PROJ_NG; i++)
        p6.g[i] = GP[i].def;
    p6.g[G_BPM] = 133, p6.sel = 2, p6.drdly = 9;
    for (i = 0; i < NTRK; i++) {
        proj_trk_v6_t *t = &p6.t[i];
        for (k = 0; k < PROJ_E0; k++)
            t->p[k] = TP[k].def;
        t->p[P_ATK] = (int16_t)(10 + i), t->p[P_SLEN] = 16;
        for (k = 0; k < 8u; k++)
            t->e[k] = (int16_t)(i * 10u + k);
        for (k = 0; k < PROJ_NX_V6; k++)
            t->x[k] = 99;                                /* (2.6's INSERT and the like: not Clipper's) */
        t->engine = (uint8_t)i, t->preset = 1;
        t->step[3].time = ST_NOTE, t->step[3].n = 1, t->step[3].note[0] = (uint8_t)(60 + i);
        t->mf[3] = (uint8_t)((uint32_t)(-5 & 63) | 2u << 6);   /* a nudge of -5, the condition 2 */
        for (k = 0; k < NLOCK; k++)
            t->lock[k].step = LOCK_FREE;
        t->lock[0].step = 3, t->lock[0].param = 63u + 2u, t->lock[0].val = 40;   /* on E2 (2.6's P_E0 is 63) */
        t->lock[1].step = 4, t->lock[1].param = 54u, t->lock[1].val = 50;        /* on 2.6's INSERT A */
        t->lock[2].step = 5, t->lock[2].param = P_DIST, t->lock[2].val = 70;
    }
    p6.sum = proj_hash(&p6, sizeof p6 - 4u);
    good = proj_import(q, &p6, (int)sizeof p6) && proj_ok(q) && q->g[G_BPM] == 133 && q->sel == 2 && q->drdly == 9;
    for (i = 0; i < NTRK && good; i++) {
        const proj_trk_t *t = &q->t[i];
        good &= t->p[P_ATK] == (int16_t)(10 + i) && t->p[PROJ_E0 + 7u] == (int16_t)(i * 10u + 7u) && t->engine == i &&
                t->step[3].note[0] == 60u + i && t->micro[3] == -5 && ((t->fill[0] >> 6) & 3u) == 2u &&
                t->lock[0].param == PROJ_E0 + 2u && t->lock[0].val == 40 && t->lock[1].step == LOCK_FREE &&
                t->lock[2].param == P_DIST;
    }
    ok(good, "a SLOOP 2.6 project (FUN6): read (every value at its place, nudges, conditions, locks; 2.6's own left out)");
    p6.sum ^= 1u;
    ok(!proj_import(q, &p6, (int)sizeof p6), "a damaged FUN6: refused");
}

/* user presets: one bank SLOOP and Clipper both read */
static void t_userbank(void)
{
    static uint8_t raw[8u + UP_PER_BANK * UP_RSIZE_04];
    up_bank_t *bk = &up_bank[0];
    uint32_t k, i, good = 1;
    int16_t def[P_COUNT], out[P_COUNT], v[P_COUNT];
    for (i = 0; i < P_COUNT; i++)
        def[i] = i >= P_E0 ? 0 : TP[i].def;
    {   /* a record round trip: every LFO 2 / ENV 2 / matrix value at its extremes and between */
        up_rec_t r;
        uint32_t pass, same = 1;
        for (pass = 0; pass < 3u; pass++) {
            memset(&r, 0, sizeof r);
            for (i = 0; i < P_COUNT; i++)
                v[i] = (int16_t)(i < P_E0 ? (pass == 0 ? TP[i].min : pass == 1 ? TP[i].max : xval(0, i - PROJ_E0)) : 40 + i);
            for (i = 0; i < PROJ_E0; i++)
                v[i] = (int16_t)(i * 3);
            r.used = UP_USED, r.ver = UP_VER, r.name[0] = 'X';
            up_rec_set(&r, v);
            up_params(&r, out, def);
            for (i = 0; i < P_COUNT; i++)
                same &= out[i] == v[i];
        }
        ok(same && sizeof(up_rec_t) == 192u && r.np == 61u && up_valid(&r),
           "user presets: a record SLOOP reads (192 bytes, np 61), LFO 2 / ENV 2 / matrix packed in it, back exactly");
        /* as SLOOP 2.5 / 2.6 maps it: the first np - 8 by place, the last 8 its engine's */
        ok(r.p[0] == 0 && r.p[52] == 156 && r.p[53] == 40 + P_E0 && r.p[60] == 47 + P_E0,
           "user presets: SLOOP sees its sound as it was (common values, the engine's 8)");
    }
    {   /* a SLOOP 2.6 record (np 71: its ten new values before P_E0 at 63) */
        up_rec_t r;
        memset(&r, 0, sizeof r);
        r.used = UP_USED, r.ver = UP_VER, r.np = 71, r.name[0] = 'S';
        for (i = 0; i < 71u; i++)
            r.p[i] = (int16_t)(i < 53u ? i : i < 63u ? 99 : 200 + i);
        up_params(&r, out, def);
        for (i = 0; i < 53u; i++)
            good &= out[i] == (int16_t)i;
        for (i = 53u; i < P_E0; i++)                 /* (0.6: its INSERT, the same values: 99) */
            good &= out[i] == (i >= P_ITYPE ? 99 : def[i]);
        for (i = 0; i < 8u; i++)
            good &= out[P_E0 + i] == (int16_t)(263 + i);
        ok(good, "user presets: a SLOOP 2.6 record: its sound and its INSERT, 2.6's others left out, the matrix at defaults");
    }
    /* Clipper 0.4's first bank (232-byte records, np 86) */
    memset(raw, 0, sizeof raw);
    {
        uint32_t m = UP_BANK_MAGIC;
        uint16_t rs = UP_RSIZE_04, ns = UP_PER_BANK;
        memcpy(raw, &m, 4), memcpy(raw + 4, &rs, 2), memcpy(raw + 6, &ns, 2);
    }
    for (k = 0; k < UP_PER_BANK; k++) {
        uint8_t *o = raw + 8u + k * UP_RSIZE_04;
        int16_t p[92];
        memset(p, 0, sizeof p);
        o[0] = k % 3u ? UP_USED : 0u, o[1] = UP_VER, o[2] = (uint8_t)(k % NENGINES), o[3] = 86u;   /* (0.4: P_COUNT 86, */
        snprintf((char *)o + 4, 12, "OLD %u", k);                                                /* P_E0 78) */
        for (i = 0; i < 86u; i++)
            p[i] = (int16_t)(i < PROJ_E0 ? k * 100u + i : i < 78u ? xval(k & 1u, i - PROJ_E0) : k * 100u + i - 78u + P_E0);
        memcpy(o + 16, p, sizeof p);
        for (i = 0; i < 16u; i++)
            o[16 + 184 + i] = (uint8_t)(40u + k + i), o[32 + 184 + i] = (uint8_t)(i & 7u);
    }
    good = up_bank_import(0, raw, (int)sizeof raw) && bk->rsize == 192u;
    for (k = 0; k < UP_PER_BANK; k++) {
        const up_rec_t *r = &bk->r[k];
        good &= (r->used == UP_USED) == (k % 3u != 0u);
        if (!(k % 3u))
            continue;
        good &= up_valid(r) && r->np == 61 && r->engine == k % NENGINES && r->note[5] == 45u + k && r->flags[7] == 7u;
        up_params(r, out, def);
        for (i = 0; i < P_COUNT; i++)
            good &= out[i] == (int16_t)(i < PROJ_E0 ? k * 100u + i : i < P_ITYPE ? xval(k & 1u, i - PROJ_E0) :
                                        i < P_E0 ? TP[i].def : k * 100u + i);
        {
            char nm[13];
            snprintf(nm, sizeof nm, "OLD %u", k);
            good &= !strncmp(r->name, nm, 12);
        }
    }
    ok(good, "user presets: Clipper 0.4's first bank (232-byte records) made the shared one, every record as it was");
    memset(raw, 0, sizeof raw);
    memcpy(raw, bk, sizeof *bk);
    ok(!up_bank_import(1, raw, (int)sizeof *bk) && !memcmp(&up_bank[1], bk, sizeof *bk), "user presets: a shared bank read as it is");
}

int main(void)
{
    uint32_t i;
    host_tracks_init();
    for (i = 0; i < NPART; i++)
        host_preset(&trk[i], TRK_DEF[i][0], TRK_DEF[i][1]);
    clipper_proj_neutral();
    trk[1].p[P_LYMODE] = LY_OFF;
    t_matrix();
    t_mpe();
    t_project();
    t_fun6();
    t_userbank();
    printf(fails ? "synth4: %d FAILED\n" : "synth4: all passed\n", fails);
    return fails;
}
