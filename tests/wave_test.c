/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper 0.2 WAVE engine (firmware/src/eng_wave.c, tools/gen_wavetables.py), CLIPPER_SYNTH 1:
 *   build/host/wave_test [DEMODIR]
 * 1. the band-limited copies: every factory table builds; a copy's harmonics are the spectrum's (level 0 against
 *    WT_SPEC, by a DFT), and above its limit there is nothing (each level)
 * 2. no aliasing: the brightest frames (a saw, a narrow pulse, sync, FM, fold) at 2.4 kHz, below 15 kHz: < -60 dB
 *    of full scale (a full-scale sine: a frame left with little at that pitch, as sync x8, is not penalised)
 * 3. building: silent while the table is built, sounding when it is ready; TABL turned: silent a moment, then the
 *    new table; the voices' phases keep running (no click when it comes back: tested as bounded)
 * 4. POS: HARMON gets brighter from 0 to 127; ENV -> SHP moves it (a sweep)
 * 5. DTN and SUB: two oscillators beat, the sub adds an octave down
 * 6. a user table (USR1, a RAM image of the slot): plays its spectrum; an empty slot is silent; a broken header
 *    is not a table
 * 7. bounded and finite: every preset, eight-note chords, random POS / DTN / SUB / RES / DRV / MODL
 * 8. cost: host instructions a voice-sample (callgrind), against HYBRID INIT, reported
 * DEMODIR: every preset playing a phrase (WAV). Exit status: the number of failed checks. */
static unsigned char host_slots[4][0x14000];
#define SMP_USER_XIP(k) ((const uint8_t *)host_slots[(k) & 3u])
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <math.h>

static int bad;
static void check(const char *what, int ok)
{
    printf("wave: %-94s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

static void build(int32_t table)
{
    int guard = 0;
    trk[0].eng_req = ENGI_WAVE;
    trk[0].p[P_E0] = (int16_t)table;
    while (wt_have != table && guard++ < 100)
        wt_service();
}

/* |harmonic h| of N points (DFT) */
static double harm(const int16_t *x, uint32_t N, uint32_t h, double *re_out, double *im_out)
{
    double re = 0, im = 0;
    uint32_t n;
    for (n = 0; n < N; n++)
        re += x[n] * cos(2 * M_PI * h * n / N), im += x[n] * sin(2 * M_PI * h * n / N);
    re *= 2.0 / N, im *= 2.0 / N;
    if (re_out)
        *re_out = re, *im_out = im;
    return sqrt(re * re + im * im);
}

static void t_mips(void)
{
    uint32_t t, worst_err = 0, l, f;
    double worst_above = 0;
    char w[140];
    for (t = 0; t < WT_NFACTORY; t++) {
        build((int32_t)t);
        for (f = 0; f < WT_NF; f += 5) {
            for (l = 0; l < WT_LEVELS; l++) {
                uint32_t N = 1024u >> l, H = 128u >> l, h;
                const int16_t *x = &wt_mip[f][WT_OFF[l]];
                double tot = 0, above = 0;
                for (h = 1; h < N / 2; h++) {
                    double re, im, a = harm(x, N, h, &re, &im);
                    tot += a * a;
                    if (h > H)
                        above += a * a;
                    else if (l == 0 && (h == 1 || h == 7 || h == 50)) {   /* the spectrum, as stored */
                        int32_t ec = abs((int32_t)lrint(re) - WT_SPEC[t][f][h - 1][0]);
                        int32_t es = abs((int32_t)lrint(im) - WT_SPEC[t][f][h - 1][1]);
                        if ((uint32_t)(ec > es ? ec : es) > worst_err)
                            worst_err = (uint32_t)(ec > es ? ec : es);
                    }
                }
                if (above / (32767.0 * 32767.0 / 2) > worst_above)   /* (dBFS: a nearly silent copy is not penalised) */
                    worst_above = above / (32767.0 * 32767.0 / 2);
                (void)tot;
            }
        }
    }
    snprintf(w, sizeof w, "16 tables built: level 0 = the stored spectrum (worst %u / 32767), above each level's limit %.1f dBFS",
             worst_err, 10 * log10(worst_above + 1e-30));
    check(w, worst_err < 40 && worst_above < 1e-8);
}

/* aliasing: as tests/alias_test.c, through wave_osc at exactly K cycles in NA samples */
#define NA 8192
#define K 449
static double re_[NA], im_[NA];
static void fft(double *xr, double *xi, int n)
{
    int i, j, k, m;
    for (i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) {
            double q = xr[i]; xr[i] = xr[j]; xr[j] = q;
            q = xi[i]; xi[i] = xi[j]; xi[j] = q;
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
static double alias_db(uint32_t table, int32_t posq)
{
    static int32_t y[NA], pos[CTL];
    uint32_t inc = (uint32_t)(((uint64_t)K << 32) / NA), ph = 0, i, k;
    double sig = 0, al = 0, mean = 0;
    build((int32_t)table);
    for (i = 0; i < CTL; i++)
        pos[i] = posq;
    for (i = 0; i < NA; i += CTL) {
        for (k = 0; k < CTL; k++)
            y[i + k] = 0;
        wave_osc(y + i, CTL, &ph, inc, pos, 32767);
    }
    for (i = 0; i < NA; i++)
        mean += y[i];
    mean /= NA;
    for (i = 0; i < NA; i++)
        re_[i] = y[i] - mean, im_[i] = 0;
    fft(re_, im_, NA);
    for (i = 1; i < NA / 2; i++) {
        double e = re_[i] * re_[i] + im_[i] * im_[i];
        if (i % K == 0)
            sig += e;
        else if (i * (double)FS / NA < 15000)
            al += e;
    }
    (void)sig;
    return 10 * log10(al / (32767.0 * 32767.0 / 2 * ((double)NA * NA / 4)) + 1e-30);   /* (dBFS: the FFT's scale) */
}

static void t_alias(void)
{
    /* (FM index 8, FOLD x6: their strongest partials sit high in the band at 2.4 kHz; the cubic's images of them
     * stay ~55 dB under them, -64 .. -70 dBFS each) */
    static const struct { uint32_t t; int32_t pos; const char *name; double lim; } C[] = {
        {0, 8 << 8, "BASIC saw", -60}, {0, 15 << 8, "BASIC narrow pulse", -60}, {6, 15 << 8, "SYNC x8", -60},
        {7, 15 << 8, "FM index 8", -55}, {9, 15 << 8, "FOLD x6", -55}, {2, 15 << 8, "HARMON 64", -60},
        {14, 7 << 8, "CHIP", -60}, {0, (8 << 8) + 128, "BASIC between frames", -60}};
    unsigned k;
    for (k = 0; k < sizeof C / sizeof C[0]; k++) {
        double a = alias_db(C[k].t, C[k].pos);
        char w[120];
        snprintf(w, sizeof w, "%-22s at 2.4 kHz: alias below 15 kHz %6.1f dBFS (<= %.0f)", C[k].name, a, C[k].lim);
        check(w, a <= C[k].lim);
    }
}

/* the WAVE part: preset pi, POLY, flat envelope, no sends */
static void setup(uint32_t pi)
{
    memset(trk, 0, sizeof trk);
    host_tracks_init();
    host_preset(&trk[0], ENGI_WAVE, pi);
    trk[0].p[P_VOICE] = V_POLY;
    trk[0].p[P_ATK] = 0, trk[0].p[P_DEC] = 64, trk[0].p[P_SUS] = 127, trk[0].p[P_REL] = 10;
    trk[0].p[P_CHOR] = trk[0].p[P_DLY] = trk[0].p[P_REV] = trk[0].p[P_DIST] = 0;
    trk[0].p[P_ED_FLT] = trk[0].p[P_ED_SHP] = trk[0].p[P_LD_SHP] = trk[0].p[P_LD_PIT] = trk[0].p[P_LD_FLT] = 0;
}
/* blocks of track_render into y (a held note, from note-on); returns the rms; bright: rms of the difference */
static double play(uint32_t note, uint32_t blocks, double *bright, int32_t *peak, int service)
{
    static int32_t b[CTL];
    uint32_t i, k;
    double e = 0, d = 0;
    int32_t prev = 0, mx = 0;
    trk_note_on(&trk[0], note, 100);
    for (i = 0; i < blocks; i++) {
        if (service)
            wt_service();
        track_render(&trk[0], b, CTL);
        for (k = 0; k < CTL; k++) {
            if (abs(b[k]) > mx)
                mx = abs(b[k]);
            if (i >= blocks / 4)
                e += (double)b[k] * b[k], d += (double)(b[k] - prev) * (b[k] - prev);
            prev = b[k];
        }
    }
    trk_note_off(&trk[0], note);
    for (i = 0; i < 40; i++)
        track_render(&trk[0], b, CTL);
    if (bright)
        *bright = sqrt(d / (blocks * CTL)) / (sqrt(e / (blocks * CTL)) + 1e-9);
    if (peak)
        *peak = mx;
    return sqrt(e / ((blocks - blocks / 4) * CTL));
}

static void t_building(void)
{
    double r0, r1, r2;
    char w[140];
    setup(0);
    wt_have = -1, wt_building = -1;
    trk[0].p[P_E0] = 0;
    r0 = play(57, 8, 0, 0, 0);                       /* nothing built, no service: silent */
    build(0);
    r1 = play(57, 40, 0, 0, 0);
    trk[0].p[P_E0] = 5;                              /* TABL turned: the next service starts RESO */
    wt_service();
    r2 = play(57, 4, 0, 0, 0);
    snprintf(w, sizeof w, "silent before its table is built (%.0f), sounds after (%.0f), silent while the next builds (%.0f)", r0, r1, r2);
    check(w, r0 == 0 && r1 > 500 && r2 == 0);
    build(5);
    r2 = play(57, 40, 0, 0, 0);
    snprintf(w, sizeof w, "the new table sounds once built (%.0f)", r2);
    check(w, r2 > 500);
}

static void t_pos(void)
{
    double b0, b1, bs0, bs1;
    char w[140];
    setup(0);
    trk[0].p[P_E0] = 2, trk[0].p[P_E4] = 127, trk[0].p[P_E5] = 0;   /* HARMON, the filter open */
    build(2);
    trk[0].p[P_E1] = 0;
    play(45, 60, &b0, 0, 1);
    trk[0].p[P_E1] = 127;
    play(45, 60, &b1, 0, 1);
    snprintf(w, sizeof w, "POS on HARMON: brighter at 127 than at 0 (%.3f vs %.3f)", b1, b0);
    check(w, b1 > 3 * b0);
    trk[0].p[P_E1] = 0, trk[0].p[P_ED_SHP] = 63, trk[0].p[P_SUS] = 0, trk[0].p[P_DEC] = 90;   /* ENV -> SHP */
    {
        static int32_t b[CTL];
        uint32_t i, k;
        double e1 = 0, d1 = 0, e2 = 0, d2 = 0;
        int32_t prev = 0;
        trk_note_on(&trk[0], 45, 100);
        for (i = 0; i < 400; i++) {
            track_render(&trk[0], b, CTL);
            for (k = 0; k < CTL; k++) {
                double x = b[k], dx = b[k] - prev;
                if (i < 40)
                    e1 += x * x, d1 += dx * dx;
                else if (i > 300)
                    e2 += x * x, d2 += dx * dx;
                prev = b[k];
            }
        }
        trk_note_off(&trk[0], 45);
        bs0 = sqrt(d1 / e1), bs1 = sqrt(d2 / (e2 + 1e-9));
        snprintf(w, sizeof w, "ENV -> SHP sweeps POS: bright at the attack (%.3f), darker as it decays (%.3f)", bs0, bs1);
        check(w, bs0 > 1.5 * bs1);
    }
}

static void t_dtn_sub(void)
{
    double r_one, r_sub;
    char w[140];
    static int32_t b[CTL];
    setup(0);
    trk[0].p[P_E0] = 0, trk[0].p[P_E1] = 0, trk[0].p[P_E4] = 127, trk[0].p[P_E5] = 0;   /* a sine */
    build(0);
    trk[0].p[P_E2] = 0, trk[0].p[P_E3] = 0;
    r_one = play(57, 60, 0, 0, 1);
    trk[0].p[P_E3] = 127;
    r_sub = play(57, 60, 0, 0, 1);
    snprintf(w, sizeof w, "SUB 127 adds the octave below (rms %.0f -> %.0f)", r_one, r_sub);
    check(w, r_sub > r_one * 1.05);              /* (the sub at half the level) */
    {   /* DTN 20 cents: the envelope beats (the rms of 10 ms windows varies) */
        uint32_t i, k;
        double lo = 1e18, hi = 0;
        trk[0].p[P_E3] = 0, trk[0].p[P_E2] = 50;
        trk_note_on(&trk[0], 69, 100);
        for (i = 0; i < 1400; i++) {
            double e = 0;
            track_render(&trk[0], b, CTL);
            for (k = 0; k < CTL; k++)
                e += (double)b[k] * b[k];
            if (i > 200 && i % 1 == 0) {
                static double acc;
                static int cnt;
                acc += e, cnt++;
                if (cnt == 14) {                     /* ~10 ms */
                    if (acc < lo) lo = acc;
                    if (acc > hi) hi = acc;
                    acc = 0, cnt = 0;
                }
            }
        }
        trk_note_off(&trk[0], 69);
        snprintf(w, sizeof w, "DTN 50: two oscillators beat (10 ms energy from %.2g to %.2g)", lo, hi);
        check(w, hi > 4 * lo);
    }
}

static void t_user(void)
{
    wt_user_hdr_t *h = (wt_user_hdr_t *)host_slots[0];
    wt_user_t *u = (wt_user_t *)(host_slots[0] + SMP_USER_DATA);
    double r;
    uint32_t f;
    char w[140];
    memset(host_slots, 0xFF, sizeof host_slots);
    wt_user_scan(0);
    for (f = 0; f < NELEM(WAVE_PRESETS) && strcmp(WAVE_PRESETS[f].name, "USER WT"); f++)
        ;
    setup(f);                                        /* USER WT: USR1 */
    trk[0].p[P_E4] = 127;
    build(WT_NFACTORY);
    r = play(57, 40, 0, 0, 1);
    snprintf(w, sizeof w, "an empty user slot: silent (rms %.0f), no crash", r);
    check(w, r < 1);
    memset(h, 0, sizeof *h);
    memset(u, 0, sizeof *u);
    h->magic = WT_USER_MAGIC, h->version = 1, h->nf = WT_NF, h->nh = WT_NH, h->data_len = WT_USER_LEN;
    for (f = 0; f < WT_NF; f++)
        u->spec[f][2][1] = 29000;                    /* the 3rd harmonic alone */
    wt_user_scan(0);
    build(WT_NFACTORY);
    {
        double re, im, a3 = harm(&wt_mip[3][0], 1024, 3, &re, &im), a1 = harm(&wt_mip[3][0], 1024, 1, 0, 0);
        snprintf(w, sizeof w, "USR1 with a wavetable: its spectrum built (3rd harmonic %.0f, 1st %.0f)", a3, a1);
        check(w, fabs(a3 - 29000) < 50 && a1 < 5);
    }
    r = play(57, 40, 0, 0, 1);
    snprintf(w, sizeof w, "USR1 plays (rms %.0f)", r);
    check(w, r > 500);
    h->nh = 99;                                      /* a broken header */
    wt_user_scan(0);
    snprintf(w, sizeof w, "a broken header is not a table (and the built one is dropped)");
    check(w, !wt_user_ok[0] && wt_have != WT_NFACTORY);
}

static void t_bounded(void)
{
    uint32_t pi, i, k, j;
    int32_t worst = 0, nz = 4242;
    int finite = 1;
    static int32_t b[CTL];
    for (pi = 0; pi < NELEM(WAVE_PRESETS); pi++) {
        setup(pi);
        for (j = 0; j < 6; j++) {
            uint32_t r = noise32(&nz);
            trk[0].p[P_E1] = (int16_t)(r % 128u);
            trk[0].p[P_E2] = (int16_t)((r >> 8) % 51u);
            trk[0].p[P_E3] = (int16_t)((r >> 14) % 128u);
            trk[0].p[P_E5] = (int16_t)(j == 5 ? 127 : (r >> 20) % 128u);
            trk[0].p[P_E6] = (int16_t)((r >> 3) % 128u);
            trk[0].p[P_FMODEL] = (int16_t)(1 + (r >> 27) % 2u);
            for (k = 0; k < 8; k++)
                trk_note_on(&trk[0], 36 + k * 5 + j, 127);
            for (i = 0; i < 120; i++) {
                wt_service();
                track_render(&trk[0], b, CTL);
                for (k = 0; k < CTL; k++)
                    if (abs(b[k]) > worst)
                        worst = abs(b[k]);
            }
            for (k = 0; k < 8; k++)
                trk_note_off(&trk[0], 36 + k * 5 + j);
            for (i = 0; i < 60; i++)
                track_render(&trk[0], b, CTL);
        }
    }
    {
        char w[140];
        snprintf(w, sizeof w, "every preset, eight-note chords, random POS / DTN / SUB / RES / DRV / MODL: peak %d", worst);
        check(w, finite && worst < 8 * 100000);
    }
}

static void demos(const char *dir)
{
    uint32_t pi;
    for (pi = 0; pi < NELEM(WAVE_PRESETS); pi++) {
        char path[256], name[32];
        FILE *f;
        uint32_t i, k, b;
        static int32_t buf[CTL];
        static const uint8_t PH[] = {45, 52, 57, 60, 64, 57, 52, 48};
        if (!strcmp(WAVE_PRESETS[pi].name, "USER WT"))
            continue;                                /* (an empty slot: nothing to hear) */
        setup(pi);
        snprintf(name, sizeof name, "%s", WAVE_PRESETS[pi].name);
        for (i = 0; name[i]; i++)
            if (name[i] == ' ')
                name[i] = '_';
        snprintf(path, sizeof path, "%s/%02u_%s.wav", dir, pi, name);
        f = fopen(path, "wb");
        if (!f)
            return;
        wav_hdr(f, 8u * 20u * CTL * 5u);
        build(trk[0].p[P_E0] % WT_NALL);
        for (k = 0; k < 8; k++) {
            trk_note_on(&trk[0], PH[k] - (WAVE_PRESETS[pi].mono ? 12 : 0), 100);
            for (b = 0; b < 100; b++) {
                track_render(&trk[0], buf, CTL);
                for (i = 0; i < CTL; i++)
                    wav_put(f, buf[i] / 2, buf[i] / 2);
            }
            trk_note_off(&trk[0], PH[k] - (WAVE_PRESETS[pi].mono ? 12 : 0));
        }
        fclose(f);
    }
}

int main(int argc, char **argv)
{
    t_mips();
    t_alias();
    t_building();
    t_pos();
    t_dtn_sub();
    t_user();
    t_bounded();
    if (argc > 1)
        demos(argv[1]);
    printf(bad ? "wave: %d FAILED\n" : "wave test passed\n", bad);
    return bad;
}
