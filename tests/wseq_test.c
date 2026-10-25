/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* SynthMaven 0.6 WSEQ (SYNTHMAVEN_SYNTH 1): wave sequencing.
 *   STEPS   at the song's tempo (1/16 at 120 BPM: 5512 samples), each at its own pitch (MINOR: 0 3 7 10 12 10 7 3)
 *   LOOP    FWD, BACK, PING (back and forth, the ends once), ONCE (stays on the last step)
 *   PITCH   0: every step at the note; 127: twice the intervals
 *   START   the first step
 *   XFADE   127: the next step fades in over the whole step (no step in the level); a cut between two levels (GATE):
 *           glided, no click
 *   bounded, finite, every preset; a voice ends after its release
 *   wseq_test [DEMODIR] */
#define main hostsim_main
#include "hostsim.c"
#undef main

static int fails;
static void ok(int c, const char *what)
{
    printf("wseq: %-96s %s\n", what, c ? "ok" : "FAIL");
    fails += !c;
}
#define NCAP (4096u * 40u)
static double cap[NCAP];
static uint32_t ncap;
static void run(uint32_t nb)
{
    static int32_t o[CTL * 2];
    uint32_t b, i;
    ncap = 0;
    for (b = 0; b < nb; b++) {
        mix_block(o, CTL);
        for (i = 0; i < CTL; i++)
            if (ncap < NCAP)
                cap[ncap++] = o[2 * i];
    }
}
static double pitch(uint32_t a, uint32_t n)
{
    static double r[FS / 40 + 2];
    uint32_t lag, i, lo = FS / 2500, hi = FS / 40, best = 0;
    double rmax = 0;
    for (lag = lo - 1; lag <= hi + 1; lag++) {
        double s = 0, e1 = 0, e2 = 0;
        for (i = a; i < a + n; i++)
            s += cap[i] * cap[i + lag], e1 += cap[i] * cap[i], e2 += cap[i + lag] * cap[i + lag];
        r[lag] = s / sqrt(e1 * e2 + 1e-9);
        if (lag >= lo && lag <= hi && r[lag] > rmax)
            rmax = r[lag];
    }
    for (lag = lo; lag <= hi; lag++)
        if (r[lag] >= 0.9 * rmax && r[lag] >= r[lag - 1] && r[lag] >= r[lag + 1]) {
            best = lag;
            break;
        }
    if (!best || rmax < 0.3)
        return 0;
    {
        double y0 = r[best - 1], y1 = r[best], y2 = r[best + 1], d = (y0 - y2) / (2 * (y0 - 2 * y1 + y2) - 1e-12);
        return FS / (best + (fabs(d) < 1 ? d : 0));
    }
}
static double rms(uint32_t a, uint32_t n)
{
    double e = 0;
    uint32_t i;
    for (i = a; i < a + n && i < ncap; i++)
        e += cap[i] * cap[i];
    return sqrt(e / n);
}
static void quiet(void)
{
    uint32_t i, k;
    for (i = 0; i < NPART; i++)
        trk_all_off(&trk[i]);
    run(600);
    for (i = 0; i < NPART; i++)
        for (k = 0; k < NVOICE; k++)
            trk[i].v[k].active = trk[i].v[k].gate = 0, trk[i].v[k].stage = 0, trk[i].v[k].env = trk[i].v[k].env_out = 0;
    run(100);
}
/* WSEQ, sequence seq, 1/16 at 120 BPM, no fade, wide open, sustained, dry */
static void setup(uint32_t seq, uint32_t loop, int32_t pscale, int32_t start)
{
    track_t *t = &trk[0];
    quiet();
    host_preset(t, ENGI_WSEQ, 0);
    t->p[P_DIST] = t->p[P_CHOR] = t->p[P_DLY] = t->p[P_REV] = 0;
    t->p[P_LD_PIT] = t->p[P_LD_FLT] = t->p[P_LD_SHP] = t->p[P_LD_AMP] = t->p[P_ED_FLT] = 0;
    t->p[P_ATK] = 0, t->p[P_SUS] = 127, t->p[P_REL] = 10;
    t->p[P_E0] = (int16_t)seq, t->p[P_E1] = 40, t->p[P_E2] = 0, t->p[P_E3] = (int16_t)loop, t->p[P_E4] = (int16_t)start;
    t->p[P_E5] = (int16_t)pscale, t->p[P_E6] = 127, t->p[P_E7] = 0;
    song.g[G_BPM] = 120;
}
#define STEP 5512u                                       /* 1/16 at 120 BPM */
/* the semitones of each of the first n steps, from the note's pitch */
static void steps(uint32_t n, double *semi)
{
    uint32_t k;
    double f0 = 220.0;
    run((n * STEP) / CTL + 4u);
    for (k = 0; k < n; k++) {
        double f = pitch(k * STEP + 1200, 2600);
        semi[k] = f > 0 ? 12 * log2(f / f0) : -99;
    }
}
static int same_steps(const double *got, const int *want, uint32_t n, double tol)
{
    uint32_t k;
    for (k = 0; k < n; k++)
        if (fabs(got[k] - want[k]) > tol)
            return 0;
    return 1;
}

int main(int argc, char **argv)
{
    uint32_t i;
    double s[16];
    char w[300];
    host_tracks_init();
    for (i = 0; i < NPART; i++)
        host_preset(&trk[i], i ? 1u : ENGI_WSEQ, 0);
    synthmaven_proj_neutral();
    trk[1].p[P_LYMODE] = LY_OFF;
    {   /* MINOR (4): 0 3 7 10 12 10 7 3, FWD */
        static const int want[10] = {0, 3, 7, 10, 12, 10, 7, 3, 0, 3};
        setup(4, WL_FWD, 64, 0);
        trk_note_on(&trk[0], 57, 100);
        steps(10, s);
        snprintf(w, sizeof w, "STEPS: 1/16 at 120 BPM, each at its pitch, round again (%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f)",
                 s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9]);
        ok(same_steps(s, want, 10, 0.3), w);
    }
    {   /* DORIAN (14): 0 2 3 5 7 9 10 12; BACK from the first: 0 12 10 9 7 5 */
        static const int wb[6] = {0, 12, 10, 9, 7, 5};
        setup(14, WL_BACK, 64, 0);
        trk_note_on(&trk[0], 57, 100);
        steps(6, s);
        snprintf(w, sizeof w, "LOOP BACK: the steps backwards (%.1f %.1f %.1f %.1f %.1f %.1f)", s[0], s[1], s[2], s[3], s[4], s[5]);
        ok(same_steps(s, wb, 6, 0.3), w);
    }
    {   /* FIFTHS (3): 0 7 12 19, PING: 0 7 12 19 12 7 0 7 */
        static const int want[8] = {0, 7, 12, 19, 12, 7, 0, 7};
        setup(3, WL_PING, 64, 0);
        trk_note_on(&trk[0], 57, 100);
        steps(8, s);
        snprintf(w, sizeof w, "LOOP PING: back and forth, the ends once (%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f)",
                 s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
        ok(same_steps(s, want, 8, 0.3), w);
    }
    {   /* ONCE: 0 7 12 19 19 19 */
        static const int want[6] = {0, 7, 12, 19, 19, 19};
        setup(3, WL_ONCE, 64, 0);
        trk_note_on(&trk[0], 57, 100);
        steps(6, s);
        snprintf(w, sizeof w, "LOOP ONCE: stays on the last step (%.1f %.1f %.1f %.1f %.1f %.1f)", s[0], s[1], s[2], s[3], s[4], s[5]);
        ok(same_steps(s, want, 6, 0.3), w);
    }
    {   /* PITCH 0 and 127 */
        static const int z[4] = {0, 0, 0, 0}, dbl[4] = {0, 14, 24, 38};
        setup(3, WL_FWD, 0, 0);
        trk_note_on(&trk[0], 57, 100);
        steps(4, s);
        ok(same_steps(s, z, 4, 0.3), "PITCH 0: every step at the note");
        setup(3, WL_FWD, 127, 0);
        trk_note_on(&trk[0], 45, 100);
        steps(4, s);
        for (i = 0; i < 4u; i++)
            s[i] += 12;                                   /* (the note an octave down) */
        snprintf(w, sizeof w, "PITCH 127: about twice the intervals (%.1f %.1f %.1f %.1f)", s[0], s[1], s[2], s[3]);
        ok(same_steps(s, dbl, 4, 0.8), w);
    }
    {   /* START 64 of MINOR's 8: from step 5 (12) */
        setup(4, WL_FWD, 64, 64);
        trk_note_on(&trk[0], 57, 100);
        steps(2, s);
        snprintf(w, sizeof w, "START: each note from its step (%.1f %.1f: 12, 10)", s[0], s[1]);
        ok(fabs(s[0] - 12) < 0.3 && fabs(s[1] - 10) < 0.3, w);
    }
    {   /* XFADE 127 on MORPH (10, one level): the level as smooth as a held note; GATE (7) cut: no click */
        double lo = 1e9, hi = 0, jump = 0, ref = 0;
        setup(10, WL_FWD, 64, 0);
        trk[0].p[P_E2] = 127;
        trk_note_on(&trk[0], 57, 100);
        run(6u * STEP / CTL);
        for (i = 1; i < 5u; i++) {
            double r = rms(i * STEP - 1000, 2000);
            lo = r < lo ? r : lo, hi = r > hi ? r : hi;
        }
        snprintf(w, sizeof w, "XFADE 127: across the step changes the level holds (%.0f .. %.0f)", lo, hi);
        ok(hi < lo * 1.6, w);
        setup(7, WL_FWD, 64, 0);
        trk[0].p[P_E0] = 7, trk[0].p[P_E2] = 0;
        trk_note_on(&trk[0], 45, 100);
        run(6u * STEP / CTL);
        for (i = 1; i < ncap; i++) {
            double d = fabs(cap[i] - cap[i - 1]);
            if (i % STEP < 64u || i % STEP > STEP - 64u)
                jump = d > jump ? d : jump;
            else
                ref = d > ref ? d : ref;
        }
        snprintf(w, sizeof w, "GATE, cut steps: the level glides (largest step at a change %.0f, inside a step %.0f)", jump, ref);
        ok(jump < ref * 1.5, w);
    }
    {   /* every preset: bounded, finite; the voices end */
        const engine_t *e = ENGINES[ENGI_WSEQ];
        uint32_t k, bad = 0, stuck = 0;
        double pk = 0;
        for (k = 0; k < e->npresets; k++) {
            track_t *t = &trk[0];
            uint32_t nn;
            quiet();
            host_preset(t, ENGI_WSEQ, k);
            for (nn = 0; nn < 6u; nn++)
                trk_note_on(t, 48 + 4 * nn, 120);
            run(400);
            for (i = 0; i < ncap; i++) {
                bad += !(cap[i] == cap[i]);
                pk = fabs(cap[i]) > pk ? fabs(cap[i]) : pk;
            }
            if (argc > 1) {
                char path[512];
                FILE *f;
                snprintf(path, sizeof path, "%s/wseq-%02u.raw", argv[1], k);
                f = fopen(path, "wb");
                if (f) {
                    static int16_t s16[NCAP];
                    for (i = 0; i < ncap; i++)
                        s16[i] = (int16_t)(cap[i] / 4 > 32767 ? 32767 : cap[i] / 4 < -32767 ? -32767 : cap[i] / 4);
                    fwrite(s16, 2, ncap, f);
                    fclose(f);
                }
            }
            for (nn = 0; nn < 6u; nn++)
                trk_note_off(t, 48 + 4 * nn);
            for (i = 0; i < 6u; i++)                     /* (the longest release: ~8 s) */
                run(2000);
            for (i = 0; i < NVOICE; i++)
                stuck += t->v[i].active;
        }
        snprintf(w, sizeof w, "every preset with a 6-note chord: bounded (peak %.0f), finite (%u bad); every voice ends (%u stuck)", pk, bad, stuck);
        ok(!bad && pk < 8388607.0 && !stuck, w);
    }
    printf(fails ? "wseq: %d FAILED\n" : "wseq: all passed\n", fails);
    return fails;
}
