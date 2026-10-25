/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* SynthMaven 0.5 engines VPS and TERRAIN (firmware/src/eng_vps.c, eng_terrain.c; Felucca discussions #145, #146),
 * SYNTHMAVEN_SYNTH 1:   build/host/engines3_test [DEMODIR]
 * VPS      the pitch of its key; V up: brighter, a formant around V x f0, the same pitch; D moves the timbre; no DC; V at
 *          its top on C7 stays in the harmonics (no aliasing); V swept through whole cycles by VENV: no step
 * TERRAIN  every surface pitched at its key; RAD 0 silent, a wider orbit brighter; DRIFT moves the timbre, the pitch
 *          stays; the widest orbit on C7 stays in the harmonics
 * all      every preset, random EDIT values, chords: bounded and finite; 8 voices cost less than HYBRID's
 * DEMODIR: every preset playing a phrase (vps-terrain.wav). Exit status: the number of failed checks. */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <math.h>

static int bad;
static void check(const char *what, int ok)
{
    printf("eng3: %-94s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

#define NB 4096
static double y[NB * 8];

static void setup(uint32_t eng, uint32_t pi)
{
    memset(trk, 0, sizeof trk);
    host_tracks_init();
    host_preset(&trk[0], eng, pi);
    trk[0].p[P_CHOR] = trk[0].p[P_DLY] = trk[0].p[P_REV] = trk[0].p[P_DIST] = 0;
    trk[0].p[P_LD_PIT] = trk[0].p[P_LD_FLT] = trk[0].p[P_LD_SHP] = trk[0].p[P_LD_AMP] = 0;
}
/* blocks of track_render into y[], from y[off] */
static void render(uint32_t blocks, uint32_t off)
{
    static int32_t b[CTL];
    uint32_t i, k;
    for (i = 0; i < blocks; i++) {
        track_render(&trk[0], b, CTL);
        for (k = 0; k < CTL; k++)
            if (off + i * CTL + k < NB * 8u)
                y[off + i * CTL + k] = b[k];
    }
}
static double rms(uint32_t a, uint32_t b)
{
    double e = 0;
    uint32_t i;
    for (i = a; i < b; i++)
        e += y[i] * y[i];
    return sqrt(e / (b - a));
}
static double bright(uint32_t a, uint32_t b)        /* rms of the first difference / rms */
{
    double e = 0, d = 0;
    uint32_t i;
    for (i = a + 1; i < b; i++)
        e += y[i] * y[i], d += (y[i] - y[i - 1]) * (y[i] - y[i - 1]);
    return sqrt(d / (e + 1e-9));
}
static double freq(uint32_t a, uint32_t b)          /* the pitch: the autocorrelation's first strong peak */
{
    uint32_t lag, n = b - a, lo = FS / 2000, hi = FS / 25, best = 0;
    double r0 = 0, r[FS / 25 + 2], mx = 0;
    if (hi > n / 2)
        hi = n / 2;
    for (lag = 0; lag <= hi + 1; lag++) {
        double e = 0;
        uint32_t i;
        for (i = a; i + lag < b; i++)
            e += y[i] * y[i + lag];
        r[lag] = e / (n - lag);
        if (!lag)
            r0 = r[0];
    }
    for (lag = lo; lag <= hi; lag++)
        if (r[lag] > mx)
            mx = r[lag];
    for (lag = lo; lag <= hi; lag++)                 /* the first local peak within 10 % of the highest */
        if (r[lag] >= 0.9 * mx && r[lag] >= r[lag - 1] && r[lag] >= r[lag + 1]) {
            best = lag;
            break;
        }
    if (!best || r0 <= 0)
        return 0;
    {
        double d = r[best - 1] - 2 * r[best] + r[best + 1];
        double off = d != 0 ? 0.5 * (r[best - 1] - r[best + 1]) / d : 0;
        return FS / (best + off);
    }
}
static double band(uint32_t a, uint32_t n, double f)   /* |DFT| at f over n samples (Hann) */
{
    double re = 0, im = 0;
    uint32_t i;
    for (i = 0; i < n; i++) {
        double w = 0.5 - 0.5 * cos(2 * M_PI * i / n);
        re += y[a + i] * w * cos(2 * M_PI * f * i / FS);
        im += y[a + i] * w * sin(2 * M_PI * f * i / FS);
    }
    return sqrt(re * re + im * im) / n;
}
static double nf(uint32_t note) { return 440.0 * pow(2, (note - 69.0) / 12); }


/* the share of the energy in the harmonics of f0 (aliases fall between them): a 4-term Hann DFT at every harmonic */
static double purity(uint32_t a, uint32_t n, double f0)
{
    double e = 0, h = 0;
    uint32_t i, k;
    for (i = a; i < a + n; i++)
        e += y[i] * y[i];
    e /= n;
    for (k = 1; k * f0 < FS / 2.0; k++) {
        double b = band(a, n, k * f0) * 4;              /* (Hann: a sine of amplitude A reads A / 4) */
        h += b * b / 2;
    }
    return h / (e + 1e-9);
}
static uint32_t used_cost;

static void t_vps(void)
{
    char w[160];
    double f, b0, b1, b2, m = 0, pu;
    uint32_t i;
    setup(ENGI_VPS, 0);
    trk[0].p[P_E0] = 64, trk[0].p[P_E1] = 24, trk[0].p[P_E2] = 0, trk[0].p[P_E3] = 0, trk[0].p[P_E4] = 127,
    trk[0].p[P_E5] = 0, trk[0].p[P_E7] = 0, trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127, trk[0].p[P_VOICE] = V_POLY;
    trk[0].p[P_ED_FLT] = 0;
    trk_note_on(&trk[0], 57, 100), render(400, 0);
    f = freq(NB, NB * 3);
    snprintf(w, sizeof w, "VPS: the pitch of its key (A3 %.2f Hz)", f);
    check(w, fabs(f - 220) < 1.5);
    b0 = bright(NB, NB * 3);
    trk_all_off(&trk[0]), render(200, 0);
    trk[0].p[P_E1] = 100;                           /* V 6.25: a formant at ~6 x f0 */
    trk_note_on(&trk[0], 57, 100), render(400, 0);
    b1 = bright(NB, NB * 3), f = freq(NB, NB * 3);
    snprintf(w, sizeof w, "VPS: V up, brighter (%.3f -> %.3f), the same pitch (%.2f Hz)", b0, b1, f);
    check(w, b1 > b0 * 2 && fabs(f - 220) < 1.5);
    {   /* the formant: harmonics 5..7 above 1..2 */
        double lo = band(NB, NB * 2, 220) + band(NB, NB * 2, 440), hi = band(NB, NB * 2, 1320) + band(NB, NB * 2, 1540);
        snprintf(w, sizeof w, "VPS: V 6.25: a formant (harmonics 6, 7 %.0f above 1, 2 %.0f)", hi, lo);
        check(w, hi > lo);
    }
    trk_all_off(&trk[0]), render(200, 0);
    trk[0].p[P_E1] = 24;                             /* V 1.5 */
    trk_note_on(&trk[0], 57, 100), render(400, 0);
    b1 = bright(NB, NB * 3);
    trk_all_off(&trk[0]), render(200, 0);
    trk[0].p[P_E0] = 10;
    trk_note_on(&trk[0], 57, 100), render(400, 0);
    b2 = bright(NB, NB * 3);
    snprintf(w, sizeof w, "VPS: D moves the timbre (V 1.5: D 10 %%: %.3f, D 50 %%: %.3f)", b2, b1);
    check(w, fabs(b2 - b1) > 0.2 * b1);
    trk_all_off(&trk[0]), render(200, 0);
    trk[0].p[P_E0] = 4, trk[0].p[P_E1] = 0;          /* V 0, a short D: a DC, blocked */
    trk_note_on(&trk[0], 57, 100), render(800, 0);
    for (i = NB * 4; i < NB * 6; i++)
        m += y[i];
    m /= NB * 2;
    snprintf(w, sizeof w, "VPS: no DC (mean %.1f, rms %.0f)", m, rms(NB * 4, NB * 6));
    check(w, fabs(m) < 0.02 * rms(NB * 4, NB * 6) + 2);
    setup(ENGI_VPS, 0);                              /* V at its top on C7: held under ~8 kHz / f0 (alone) */
    trk[0].p[P_E0] = 64, trk[0].p[P_E1] = 127, trk[0].p[P_E2] = 0, trk[0].p[P_E3] = 0, trk[0].p[P_E4] = 127;
    trk[0].p[P_E5] = 0, trk[0].p[P_E7] = 0, trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127, trk[0].p[P_ED_FLT] = 0;
    trk_note_on(&trk[0], 96, 100), render(400, 0);
    pu = purity(NB, NB * 2, nf(96));
    snprintf(w, sizeof w, "VPS: V at its top on C7: %.1f %% of the energy in the harmonics (no aliasing)", pu * 100);
    check(w, pu > 0.97);
    setup(ENGI_VPS, 0);                              /* V crossing whole cycles under ENV (VENV): no step */
    trk[0].p[P_E0] = 64, trk[0].p[P_E1] = 40, trk[0].p[P_E2] = 63, trk[0].p[P_E3] = 0, trk[0].p[P_E4] = 127;
    trk[0].p[P_E5] = 0, trk[0].p[P_E7] = 0, trk[0].p[P_ATK] = 0, trk[0].p[P_DEC] = 60, trk[0].p[P_SUS] = 0;
    trk[0].p[P_ED_FLT] = 0, trk[0].p[P_GLIDE] = 0;
    trk_note_on(&trk[0], 45, 100), render(400, 0);
    {   /* the formant tops at ~1.4 kHz (6.4 cycles in half a 110 Hz period): ~0.2 of the peak a sample at most */
        double worst = 0, pk = 0, d;
        for (i = 1; i < NB * 3; i++) {
            if ((d = fabs(y[i] - y[i - 1])) > worst)
                worst = d;
            pk = fabs(y[i]) > pk ? fabs(y[i]) : pk;
        }
        snprintf(w, sizeof w, "VPS: V swept by VENV through whole cycles: no step (largest %.0f, peak %.0f)", worst, pk);
        check(w, worst < 0.3 * pk);
    }
}

static void t_terrain(void)
{
    char w[160];
    double f, r0, r1, b0, b1, lo = 1e9, hi = 0, pu;
    uint32_t k, i, pitched = 1;
    setup(ENGI_TERRAIN, 0);
    trk[0].p[P_E5] = 0, trk[0].p[P_E6] = 127, trk[0].p[P_E7] = 0, trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127;
    trk[0].p[P_ED_FLT] = 0, trk[0].p[P_VOICE] = V_POLY;
    for (k = 0; k < TER_COUNT; k++) {                /* every surface: pitched at its key */
        trk_all_off(&trk[0]), render(100, 0);
        trk[0].p[P_E0] = (int16_t)k, trk[0].p[P_E1] = 70, trk[0].p[P_E2] = 10, trk[0].p[P_E3] = 30, trk[0].p[P_E4] = 64;
        trk_note_on(&trk[0], 57, 100), render(400, 0);
        f = freq(NB, NB * 3);
        pitched &= fabs(f - 220) < 1.5 || fabs(f - 440) < 3 || fabs(f - 110) < 1;
        if (!pitched) {
            printf("  (terrain %u: %.2f Hz)\n", k, f);
            break;
        }
    }
    check("TERRAIN: every surface pitched at its key (or an octave: a symmetric orbit)", pitched);
    trk_all_off(&trk[0]), render(100, 0);
    trk[0].p[P_E0] = 1, trk[0].p[P_E1] = 0;
    trk_note_on(&trk[0], 57, 100), render(400, 0);
    r0 = rms(NB, NB * 3);
    trk_all_off(&trk[0]), render(100, 0);
    trk[0].p[P_E1] = 30;
    trk_note_on(&trk[0], 57, 100), render(400, 0);
    r1 = rms(NB, NB * 3), b0 = bright(NB, NB * 3);
    trk_all_off(&trk[0]), render(100, 0);
    trk[0].p[P_E1] = 110;
    trk_note_on(&trk[0], 57, 100), render(400, 0);
    b1 = bright(NB, NB * 3);
    snprintf(w, sizeof w, "TERRAIN: RAD 0 silent (%.0f vs %.0f), a wider orbit brighter (%.3f -> %.3f)", r0, r1, b0, b1);
    check(w, r0 < 0.01 * r1 && b1 > b0 * 1.3);
    trk_all_off(&trk[0]), render(100, 0);
    trk[0].p[P_E1] = 60, trk[0].p[P_E5] = 127;       /* DRIFT: the timbre moves, the pitch stays */
    trk_note_on(&trk[0], 57, 100);
    for (k = 0; k < 16u; k++) {
        double b;
        render(64, 0);
        b = bright(0, NB / 2);
        lo = b < lo ? b : lo, hi = b > hi ? b : hi;
    }
    f = freq(0, NB / 2);
    snprintf(w, sizeof w, "TERRAIN: DRIFT moves the timbre (brightness %.3f .. %.3f), the pitch stays (%.2f Hz)", lo, hi, f);
    check(w, hi > lo * 1.1 && (fabs(f - 220) < 1.5 || fabs(f - 440) < 3));
    setup(ENGI_TERRAIN, 0);                          /* the widest on C7 (alone: no release of the notes before) */
    trk[0].p[P_E6] = 127, trk[0].p[P_E7] = 0, trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127, trk[0].p[P_ED_FLT] = 0;
    trk[0].p[P_E5] = 0, trk[0].p[P_E1] = 127, trk[0].p[P_E0] = 5, trk[0].p[P_E4] = 127;
    trk_note_on(&trk[0], 96, 100), render(400, 0);
    pu = purity(NB, NB * 2, nf(96));
    snprintf(w, sizeof w, "TERRAIN: the widest orbit, FOLDS, figure eight on C7: %.1f %% in the harmonics", pu * 100);
    check(w, pu > 0.97);
    (void)i;
}

/* every preset, random EDIT values, chords: bounded, finite; the cost a voice against ANALOG's */
static void t_all(void)
{
    static const uint32_t E[2] = {ENGI_VPS, ENGI_TERRAIN};
    uint32_t e, pi, r, k, i, bad0 = 0;
    double pk = 0;
    srand(7);
    for (e = 0; e < 2u; e++)
        for (pi = 0; pi < ENGINES[E[e]]->npresets; pi++)
            for (r = 0; r < 4u; r++) {
                setup(E[e], pi);
                if (r)
                    for (k = 0; k < 8u; k++) {
                        const param_desc_t *d = &ENGINES[E[e]]->edit[k];
                        trk[0].p[P_E0 + k] = (int16_t)(d->min + rand() % (d->max - d->min + 1));
                    }
                trk[0].p[P_LD_SHP] = (int16_t)(rand() % 128 - 64), trk[0].p[P_LRATE] = 90;
                for (k = 0; k < 6u; k++)
                    trk_note_on(&trk[0], 30u + k * 11u, 30u + k * 19u);
                render(120, 0);
                for (i = 0; i < 120u * CTL; i++) {
                    if (y[i] != y[i])
                        bad0++;
                    pk = fabs(y[i]) > pk ? fabs(y[i]) : pk;
                }
            }
    {
        char w[160];
        snprintf(w, sizeof w, "VPS, TERRAIN: every preset, random EDIT values, six-note chords: bounded (peak %.0f), finite", pk);
        check(w, !bad0 && pk < 300000);
    }
    {   /* the cost: 8 voices, host time against ANALOG and HYBRID's */
        static const uint32_t C[4] = {0u, ENGI_HYBRID, ENGI_VPS, ENGI_TERRAIN};
        double t[4];
        uint32_t c;
        for (c = 0; c < 4u; c++) {
            uint64_t t0;
            setup(C[c], 0);
            trk[0].p[P_VOICE] = V_POLY;
            for (k = 0; k < 8u; k++)
                trk_note_on(&trk[0], 40u + k * 5u, 100);
            render(50, 0);
            t0 = now_ns();
            for (k = 0; k < 40u; k++)
                render(100, 0);
            t[c] = (double)(now_ns() - t0);
        }
        {
            char w[160];
            snprintf(w, sizeof w, "the cost of 8 voices: VPS x%.2f, TERRAIN x%.2f of ANALOG's (HYBRID x%.2f)", t[2] / t[0], t[3] / t[0], t[1] / t[0]);
            check(w, t[2] < t[1] && t[3] < t[1]);
        }
    }
    (void)used_cost;
}

static void demo(const char *dir)
{
    static const uint32_t E[2] = {ENGI_VPS, ENGI_TERRAIN};
    static const uint8_t PH[8] = {48, 55, 60, 63, 67, 60, 55, 51};
    char path[512];
    uint32_t e, pi, s, k;
    FILE *f;
    snprintf(path, sizeof path, "%s/vps-terrain.wav", dir);
    if (!(f = fopen(path, "wb")))
        return;
    wav_hdr(f, 0);
    for (e = 0; e < 2u; e++)
        for (pi = 0; pi < ENGINES[E[e]]->npresets; pi++) {
            setup(E[e], pi);
            trk[0].p[P_REV] = 40;
            for (s = 0; s < 8u; s++) {
                static int32_t b[CTL];
                trk_note_on(&trk[0], PH[s], 100);
                for (k = 0; k < 160u; k++) {
                    uint32_t i;
                    if (k == 120u)
                        trk_note_off(&trk[0], PH[s]);
                    track_render(&trk[0], b, CTL);
                    for (i = 0; i < CTL; i++)
                        wav_put(f, b[i] >> 2, b[i] >> 2);
                }
            }
        }
    fclose(f);
}

int main(int argc, char **argv)
{
    t_vps();
    t_terrain();
    t_all();
    if (argc > 1)
        demo(argv[1]);
    printf(bad ? "eng3: %d FAILED\n" : "eng3 test passed\n", bad);
    return bad;
}
