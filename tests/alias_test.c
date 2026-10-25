/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* Aliasing of the oscillators (SynthMaven 0.2): each oscillator renders exactly K cycles in N samples (K = 449, a prime:
 * its harmonics land on the bins m K, what folds back over Nyquist lands between them), an FFT, then the energy on
 * the bins that are not harmonics against the harmonics' ("alias / signal", dB). Below 15 kHz too: what an ear
 * notices most, the folded partials that fall in the band and sound out of tune.
 *   build/host/alias_test            prints the table, checks the limits below
 *   build/host/alias_test --raw      the table only
 * HYBRID's oscillators through hy_osc_run (the engine's own loop), ANALOG's waves through dsp.c (osc_*).
 * Limits: every band-limited oscillator under ALIAS_MAX in the band (the polyBLEP saw is the reference). */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <math.h>

#define N 8192
#define K 449                    /* cycles in N: f0 = K FS / N = 2417 Hz */
#define ALIAS_MAX -40.0          /* dB of the signal, below 15 kHz */

static int bad, raw;
static double re[N], im[N];

static void fft(double *xr, double *xi, int n)
{
    int i, j, k, m;
    for (i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) {
            double t = xr[i]; xr[i] = xr[j]; xr[j] = t;
            t = xi[i]; xi[i] = xi[j]; xi[j] = t;
        }
    }
    for (m = 2; m <= n; m <<= 1) {
        double a = -2 * M_PI / m;
        for (k = 0; k < n; k += m)
            for (j = 0; j < m / 2; j++) {
                double wr = cos(a * j), wi = sin(a * j);
                double tr = wr * xr[k + j + m / 2] - wi * xi[k + j + m / 2];
                double ti = wr * xi[k + j + m / 2] + wi * xr[k + j + m / 2];
                xr[k + j + m / 2] = xr[k + j] - tr, xi[k + j + m / 2] = xi[k + j] - ti;
                xr[k + j] += tr, xi[k + j] += ti;
            }
    }
}

/* alias / signal (dB) of y[N]: all bins, and the bins below 15 kHz */
static void measure(const int32_t *y, double *all, double *band)
{
    int i;
    double sig = 0, al = 0, alb = 0, mean = 0;
    for (i = 0; i < N; i++)
        mean += y[i];
    mean /= N;
    for (i = 0; i < N; i++)
        re[i] = y[i] - mean, im[i] = 0;
    fft(re, im, N);
    for (i = 1; i < N / 2; i++) {
        double e = re[i] * re[i] + im[i] * im[i];
        if (i % K == 0)
            sig += e;
        else {
            al += e;
            if (i * (double)FS / N < 15000)
                alb += e;
        }
    }
    *all = 10 * log10(al / sig + 1e-30);
    *band = 10 * log10(alb / sig + 1e-30);
}

static void row(const char *name, const int32_t *y, double limit)
{
    double a, b;
    measure(y, &a, &b);
    if (raw || limit > 0)
        printf("alias: %-24s all %7.1f dB   below 15 kHz %7.1f dB\n", name, a, b);
    else {
        char w[100];
        snprintf(w, sizeof w, "%-22s alias below 15 kHz %6.1f dB (<= %.0f), all %6.1f dB", name, b, limit, a);
        printf("alias: %-92s %s\n", w, b <= limit ? "ok" : "FAIL");
        bad += b > limit;
    }
}

static void hy_one(uint32_t type, uint32_t shape, const char *name, double limit)
{
    static int32_t y[N], master[CTL];
    static hy_voice_t hv;
    hy_osc_t o = {(uint8_t)type, 0, 0, 127, (uint8_t)shape, 0};
    uint32_t inc = (uint32_t)(((uint64_t)K << 32) / N), i, k = type == HO_SYNC ? 1u : 0u;
    uint32_t m1inc = inc, slinc = inc;
    memset(&hv, 0, sizeof hv);
    if (type == HO_SYNC)                             /* the slave a fifth and a bit over its master (SHP's job in
                                                      * hybrid_render: here the increment directly), synced at K */
        slinc = (uint32_t)(inc * 1.7);
    for (i = 0; i < N; i += CTL) {
        uint32_t m1ph = hv.ph[0], j;
        for (j = 0; j < CTL; j++)                    /* (FM: a sine of the same pitch modulates) */
            master[j] = osc_sine(inc * (i + j));
        hy_osc_run(&o, &hv, k, slinc, (int32_t)shape << 8, y + i, CTL, master, m1ph, m1inc);
        if (k)
            hv.ph[0] += m1inc * CTL;
    }
    row(name, y, limit);
}

static void dsp_one(int w, const char *name, double limit)
{
    static int32_t y[N];
    uint32_t inc = (uint32_t)(((uint64_t)K << 32) / N), ph = 0, i;
    for (i = 0; i < N; i++, ph += inc)
        y[i] = w == 0 ? osc_saw(ph, inc) : w == 1 ? osc_tri(ph) : osc_tri_bl(ph, inc);
    row(name, y, limit);
}

int main(int argc, char **argv)
{
    raw = argc > 1 && !strcmp(argv[1], "--raw");
    if (!raw)
        printf("alias: f0 %.0f Hz (%d cycles in %d samples)\n", (double)K * FS / N, K, N);
    dsp_one(0, "saw (polyBLEP, ref.)", ALIAS_MAX);
    dsp_one(1, "triangle, naive", 1);
    dsp_one(2, "triangle (polyBLAMP)", -55.0);
    hy_one(HO_SAW, 0, "HYBRID SAW", ALIAS_MAX);
    hy_one(HO_PULSE, 64, "HYBRID PULSE", ALIAS_MAX);
    hy_one(HO_TRI, 0, "HYBRID TRI", -55.0);
    hy_one(HO_MORPH, 30, "HYBRID MORPH (sin-tri)", ALIAS_MAX);
    hy_one(HO_MORPH, 64, "HYBRID MORPH (tri-saw)", ALIAS_MAX);
    hy_one(HO_SYNC, 0, "HYBRID SYNC", -26.0);          /* (2-point polyBLEP's limit at 2.4 kHz) */
    hy_one(HO_FOLD, 40, "HYBRID FOLD 40", ALIAS_MAX);
    hy_one(HO_FOLD, 127, "HYBRID FOLD 127", ALIAS_MAX);
    hy_one(HO_SINE, 64, "HYBRID SINE fb 64", ALIAS_MAX);
    hy_one(HO_SINE, 127, "HYBRID SINE fb 127", ALIAS_MAX);
    hy_one(HO_FM, 64, "HYBRID FM 64", ALIAS_MAX);
    hy_one(HO_FM, 127, "HYBRID FM 127", ALIAS_MAX);
    hy_one(HO_SUPER, 64, "HYBRID SUPER (detuned: n/a)", 1);
    if (!raw)
        printf(bad ? "alias: %d FAILED\n" : "alias test passed\n", bad);
    return raw ? 0 : bad;
}
