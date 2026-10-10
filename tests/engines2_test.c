/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper 0.2 engines ACID, STRINGS and MACRO (firmware/src/eng_acid.c, eng_strings.c, eng_macro.c), CLIPPER_SYNTH 1:
 *   build/host/engines2_test [DEMODIR]
 * ACID     one voice; the pitch of its key; the filter envelope decays (bright -> dark); an accent (velocity 127) is
 *          louder and brighter; successive accents push the cutoff (SWP); a slide glides and does not restart the
 *          envelope
 * STRINGS  8' at the key, 16' an octave down, 4' up; the ensemble moves (ENS 127 against 0)
 * MACRO    CHORD M: the root, the third and the fifth; SWARM: HARM spreads it; ADDTV: HARM raises the centroid, nothing
 *          above 20 kHz on a high note; SPEECH: I and U differ where F2 is; the gate (LPG) closes a held note
 * all      every preset, random EDIT values, eight notes (ACID: one), model switches: bounded and finite
 * DEMODIR: every preset playing a phrase (WAV). Exit status: the number of failed checks. */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <math.h>

static int bad;
static void check(const char *what, int ok)
{
    printf("eng2: %-94s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

#define NB 4096
static double y[NB * 8];

static void setup(uint32_t eng, uint32_t pi)
{
    memset(trk, 0, sizeof trk);
    memset(mac_t, 0, sizeof mac_t);
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

static void t_acid(void)
{
    char w[140];
    uint32_t act = 0, k;
    double b0, b1, r0, r1, rb0, rb1;
    setup(ENGI_ACID, 0);
    trk[0].p[P_E1] = 127, trk[0].p[P_E2] = 0, trk[0].p[P_E3] = 0;    /* open, no envelope: the oscillator */
    trk_note_on(&trk[0], 45, 100);
    trk_note_on(&trk[0], 52, 100);
    render(60, 0);
    for (k = 0; k < NVOICE; k++)
        act += trk[0].v[k].active && trk[0].v[k].gate;
    snprintf(w, sizeof w, "ACID: one voice (two keys held: %u sounding), at the last key's pitch (%.1f Hz, %.1f)", act, freq(256, 60 * CTL), nf(52));
    check(w, act == 1 && fabs(freq(256, 60 * CTL) / nf(52) - 1) < 0.02);
    trk_note_off(&trk[0], 45), trk_note_off(&trk[0], 52), render(20, 0);
    setup(ENGI_ACID, 0);                             /* the envelope: CUT 30, ENV 90, DEC short */
    trk[0].p[P_E1] = 30, trk[0].p[P_E3] = 100, trk[0].p[P_E4] = 30, trk[0].p[P_E5] = 0, trk[0].p[P_E7] = 0;
    trk_note_on(&trk[0], 45, 100);
    render(160, 0);
    trk_note_off(&trk[0], 45);
    b0 = bright(256, 2048), b1 = bright(4096, 5120);
    r0 = rms(256, 2048);
    snprintf(w, sizeof w, "ACID: the filter envelope decays (brightness %.3f at the start, %.3f later)", b0, b1);
    check(w, b0 > 1.8 * b1);
    render(30, 0);
    trk_note_on(&trk[0], 45, 127);                   /* an accent */
    trk[0].p[P_E5] = 127;
    render(160, 0);
    trk_note_off(&trk[0], 45);
    r1 = rms(256, 2048);
    snprintf(w, sizeof w, "ACID: an accent is louder (%.0f vs %.0f) and brighter (%.3f vs %.3f)", r1, r0, bright(256, 2048), b0);
    check(w, r1 > 1.15 * r0 && bright(256, 2048) > b0);
    {   /* the accent sweep: eight accented notes, 1/8 apart, SWP 127 vs 0: the eighth brighter with the sweep */
        uint32_t s, nn;
        for (s = 0; s < 2u; s++) {
            setup(ENGI_ACID, 0);
            trk[0].p[P_E1] = 24, trk[0].p[P_E3] = 60, trk[0].p[P_E4] = 30, trk[0].p[P_E5] = 127, trk[0].p[P_E7] = s ? 127 : 0;
            for (nn = 0; nn < 8u; nn++) {
                trk_note_on(&trk[0], 45, 127);
                render(30, 0);
                trk_note_off(&trk[0], 45);
                render(8, 0);
            }
            if (!s)
                rb0 = bright(0, 30 * CTL);
            else
                rb1 = bright(0, 30 * CTL);
        }
        snprintf(w, sizeof w, "ACID: a run of accents pushes the cutoff with SWP 127 (brightness %.3f vs %.3f at SWP 0)", rb1, rb0);
        check(w, rb1 > rb0 * 1.1);
    }
    {   /* a slide: legato, the pitch glides, the envelope goes on (no new attack of brightness) */
        double f0, f1, bb0, bb1;
        setup(ENGI_ACID, 0);
        trk[0].p[P_VOICE] = V_LEGATO, trk[0].p[P_GLMODE] = 1, trk[0].p[P_GLIDE] = 50;
        trk[0].p[P_E1] = 40, trk[0].p[P_E3] = 90, trk[0].p[P_E4] = 50, trk[0].p[P_E5] = 0;
        trk_note_on(&trk[0], 45, 100);
        render(100, 0);
        bb0 = bright(80 * CTL, 100 * CTL);
        f0 = freq(60 * CTL, 100 * CTL);
        trk_note_on(&trk[0], 52, 100);              /* held over: a slide */
        trk_note_off(&trk[0], 45);
        render(100, 0);
        bb1 = bright(2 * CTL, 20 * CTL);
        f1 = freq(60 * CTL, 100 * CTL);
        trk_note_off(&trk[0], 52);
        snprintf(w, sizeof w, "ACID: a slide reaches the next note (%.0f -> %.0f Hz) without a new filter attack (%.3f -> %.3f)", f0, f1, bb0, bb1);
        check(w, fabs(f1 / nf(52) - 1) < 0.03 && bb1 < bb0 * 1.25);
    }
}

static void t_strings(void)
{
    char w[140];
    double f8, f16, f4;
    setup(ENGI_STRINGS, 0);
    trk[0].p[P_E3] = 0, trk[0].p[P_E4] = 127, trk[0].p[P_E6] = 0;
    trk[0].p[P_ATK] = 0;
    trk[0].p[P_E0] = 0, trk[0].p[P_E1] = 127, trk[0].p[P_E2] = 0;
    trk_note_on(&trk[0], 57, 100), render(200, 0), trk_note_off(&trk[0], 57), f8 = freq(2048, 200 * CTL), render(30, 0);
    trk[0].p[P_E0] = 127, trk[0].p[P_E1] = 0;
    trk_note_on(&trk[0], 57, 100), render(200, 0), trk_note_off(&trk[0], 57), f16 = freq(2048, 200 * CTL), render(30, 0);
    trk[0].p[P_E0] = 0, trk[0].p[P_E2] = 127;
    trk_note_on(&trk[0], 57, 100), render(200, 0), trk_note_off(&trk[0], 57), f4 = freq(2048, 200 * CTL), render(30, 0);
    snprintf(w, sizeof w, "STRINGS: 8' at the key (%.1f, %.1f), 16' an octave down (%.1f), 4' up (%.1f)", f8, nf(57), f16, f4);
    check(w, fabs(f8 / nf(57) - 1) < 0.02 && fabs(f16 / f8 - 0.5) < 0.02 && fabs(f4 / f8 - 2) < 0.04);
    {   /* the ensemble: 20 ms energies vary with ENS 127, steady at 0 */
        double v[2];
        uint32_t s;
        for (s = 0; s < 2u; s++) {
            double lo = 1e30, hi = 0;
            uint32_t i;
            setup(ENGI_STRINGS, 0);
            trk[0].p[P_ATK] = 0, trk[0].p[P_E3] = s ? 127 : 0, trk[0].p[P_E5] = 80;
            trk_note_on(&trk[0], 57, 100);
            for (i = 0; i < 40; i++) {               /* ~1.2 s: windows of 28 blocks (~20 ms) */
                double e;
                render(28, 0);
                e = rms(0, 28 * CTL);
                if (i > 4) {
                    if (e < lo) lo = e;
                    if (e > hi) hi = e;
                }
            }
            trk_note_off(&trk[0], 57);
            v[s] = hi / lo;
        }
        snprintf(w, sizeof w, "STRINGS: the ensemble moves (20 ms level from x%.2f at ENS 0 to x%.2f at 127)", v[0], v[1]);
        check(w, v[1] > v[0] * 1.05);
    }
}

static void t_macro(void)
{
    char w[160];
    setup(ENGI_MACRO, 0);
    trk[0].p[P_E0] = 0, trk[0].p[P_E1] = 127, trk[0].p[P_E2] = 0, trk[0].p[P_E3] = 0;   /* CHORD M, saw */
    trk[0].p[P_E4] = 127, trk[0].p[P_E7] = 0, trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127;
    trk_note_on(&trk[0], 60, 100);
    render(640, 0);
    trk_note_off(&trk[0], 60);
    {
        double r = band(NB, 4 * NB, nf(60)), t3 = band(NB, 4 * NB, nf(64)), f5 = band(NB, 4 * NB, nf(67)), m3 = band(NB, 4 * NB, nf(63));
        snprintf(w, sizeof w, "MACRO CHORD M: root %.0f, major third %.0f, fifth %.0f, not the minor third (%.0f)", r, t3, f5, m3);
        check(w, t3 > 0.3 * r && f5 > 0.3 * r && m3 < 0.1 * t3);
    }
    render(40, 0);
    {   /* SWARM: HARM 0 one pitch, HARM 127 spread: less of the energy at the note itself */
        double nar = 0, wid = 0;
        uint32_t s;
        for (s = 0; s < 2u; s++) {
            double on, e = 0;
            uint32_t i;
            setup(ENGI_MACRO, 0);
            trk[0].p[P_E0] = 1, trk[0].p[P_E1] = s ? 127 : 0, trk[0].p[P_E2] = 0, trk[0].p[P_E3] = 127;   /* sines */
            trk[0].p[P_E4] = 127, trk[0].p[P_E7] = 0, trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127;
            trk_note_on(&trk[0], 69, 100);
            render(640, 0);
            trk_note_off(&trk[0], 69);
            on = band(NB, 4 * NB, nf(69));
            for (i = NB; i < 5 * NB; i++)
                e += y[i] * y[i];
            if (s)
                wid = on / sqrt(e / (4 * NB) + 1e-9);
            else
                nar = on / sqrt(e / (4 * NB) + 1e-9);
        }
        snprintf(w, sizeof w, "MACRO SWARM: HARM spreads it (the note's share of the level: %.3f at 0, %.3f at 127)", nar, wid);
        check(w, wid < 0.5 * nar);
    }
    {   /* ADDTV: HARM raises the brightness; a high note keeps nothing above 20 kHz (bounded brightness) */
        double b0, b1;
        setup(ENGI_MACRO, 0);
        trk[0].p[P_E0] = 2, trk[0].p[P_E2] = 20, trk[0].p[P_E3] = 64, trk[0].p[P_E4] = 127, trk[0].p[P_E7] = 0;
        trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127;
        trk[0].p[P_E1] = 0, trk_note_on(&trk[0], 45, 100), render(80, 0), trk_note_off(&trk[0], 45), b0 = bright(2048, 80 * CTL), render(30, 0);
        trk[0].p[P_E1] = 127, trk_note_on(&trk[0], 45, 100), render(80, 0), trk_note_off(&trk[0], 45), b1 = bright(2048, 80 * CTL), render(30, 0);
        snprintf(w, sizeof w, "MACRO ADDTV: HARM moves the bump up (brightness %.3f -> %.3f)", b0, b1);
        check(w, b1 > 3 * b0);
        trk[0].p[P_E1] = 127;
        trk_note_on(&trk[0], 108, 100), render(200, 0), trk_note_off(&trk[0], 108);
        {
            double hi = band(2048, 4096, FS - 6 * nf(108)) + band(2048, 4096, 7 * nf(108) - FS + 0.0 * FS)
                        + band(2048, 4096, FS - 7 * nf(108) > 0 ? FS - 7 * nf(108) : 7 * nf(108) - FS), f = band(2048, 4096, nf(108));
            snprintf(w, sizeof w, "MACRO ADDTV at C8: nothing folded back (an alias's place %.1f vs the note %.0f)", hi, f);
            check(w, hi < f * 0.01);
        }
        render(30, 0);
    }
    {   /* SPEECH: "AEIOU" held at I and at U: I's F2 (~2.3 kHz) against U's (~0.9 kHz) */
        double i23, i09, u23, u09;
        setup(ENGI_MACRO, 0);
        trk[0].p[P_E0] = 3, trk[0].p[P_E1] = 100, trk[0].p[P_E2] = 64, trk[0].p[P_E7] = 0;   /* AEIOU */
        trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127;
        trk[0].p[P_E3] = 31, trk_note_on(&trk[0], 48, 100), render(120, 0), trk_note_off(&trk[0], 48);   /* I */
        i23 = band(1024, 2048, 2250) + band(1024, 2048, 2350), i09 = band(1024, 2048, 850) + band(1024, 2048, 900);
        render(30, 0);
        trk[0].p[P_E3] = 63, trk_note_on(&trk[0], 48, 100), render(120, 0), trk_note_off(&trk[0], 48);   /* U */
        u23 = band(1024, 2048, 2250) + band(1024, 2048, 2350), u09 = band(1024, 2048, 850) + band(1024, 2048, 900);
        snprintf(w, sizeof w, "MACRO SPEECH: I has its F2 high (2.3k / 0.9k = %.2f), U low (%.2f)", i23 / (i09 + 1e-9), u23 / (u09 + 1e-9));
        check(w, i23 / (i09 + 1e-9) > 4 * (u23 / (u09 + 1e-9)));
        render(30, 0);
    }
    {   /* the gate: a held note, SUS 127, LPG 127, DEC short: it closes */
        double a, b;
        setup(ENGI_MACRO, 0);
        trk[0].p[P_E0] = 0, trk[0].p[P_E7] = 127, trk[0].p[P_E6] = 20, trk[0].p[P_ATK] = 0, trk[0].p[P_SUS] = 127;
        trk_note_on(&trk[0], 57, 100);
        render(200, 0);
        a = rms(0, 2048), b = rms(150 * CTL, 200 * CTL);
        trk_note_off(&trk[0], 57);
        snprintf(w, sizeof w, "MACRO LPG 127: a held note closes (%.0f at the start, %.0f later)", a, b);
        check(w, b < a * 0.05);
    }
}

static void t_bounded(void)
{
    static const uint32_t E[3] = {ENGI_ACID, ENGI_STRINGS, ENGI_MACRO};
    uint32_t e, pi, j, k;
    double worst = 0;
    int32_t nz = 99;
    int finite = 1;
    for (e = 0; e < 3u; e++)
        for (pi = 0; pi < ENGINES[E[e]]->npresets; pi++) {
            setup(E[e], pi);
            for (j = 0; j < 5u; j++) {
                for (k = 0; k < 8u; k++) {
                    uint32_t r = noise32(&nz);
                    if (j && k < 8u)
                        trk[0].p[P_E0 + k] = (int16_t)(ENGINES[E[e]]->edit[k].min + (int32_t)(r % (uint32_t)(ENGINES[E[e]]->edit[k].max - ENGINES[E[e]]->edit[k].min + 1)));
                }
                if (j == 4)
                    trk[0].p[P_E2] = 127, trk[0].p[P_E5] = 127;
                for (k = 0; k < 8u; k++)
                    trk_note_on(&trk[0], 30 + k * 9 + j, k == 0 ? 127 : 100);
                render(100, 0);
                if (E[e] == ENGI_MACRO)              /* a model switch with the notes sounding */
                    trk[0].p[P_E0] = (int16_t)((trk[0].p[P_E0] + 1) % 4), render(40, 0);
                for (k = 0; k < 8u; k++)
                    trk_note_off(&trk[0], 30 + k * 9 + j);
                render(60, 0);
                for (k = 0; k < 100u * CTL; k++) {
                    if (!isfinite(y[k]))
                        finite = 0;
                    if (fabs(y[k]) > worst)
                        worst = fabs(y[k]);
                }
            }
        }
    {
        char w[140];
        snprintf(w, sizeof w, "every preset, random EDIT values, eight notes, model switches: finite, peak %.0f", worst);
        check(w, finite && worst < 8 * 100000.0);
    }
}

static void demos(const char *dir)
{
    static const uint32_t E[3] = {ENGI_ACID, ENGI_STRINGS, ENGI_MACRO};
    static const uint8_t ACIDLINE[16] = {45, 45, 57, 45, 48, 45, 52, 45, 45, 55, 45, 50, 45, 45, 57, 52};
    static const uint8_t ACC[16] = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0};
    static const uint8_t SLIDE[16] = {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0};
    static const uint8_t CH[4][3] = {{48, 52, 55}, {45, 48, 52}, {41, 45, 48}, {43, 47, 50}};
    uint32_t e, pi;
    for (e = 0; e < 3u; e++)
        for (pi = 0; pi < ENGINES[E[e]]->npresets; pi++) {
            char path[256], name[24];
            FILE *f;
            uint32_t i, b, s;
            static int32_t buf[CTL];
            setup(E[e], pi);
            snprintf(name, sizeof name, "%s", ENGINES[E[e]]->presets[pi].name);
            for (i = 0; name[i]; i++)
                if (name[i] == ' ' || name[i] == '/')
                    name[i] = '_';
            snprintf(path, sizeof path, "%s/%s_%02u_%s.wav", dir, ENGINES[E[e]]->name, pi, name);
            f = fopen(path, "wb");
            if (!f)
                return;
            wav_hdr(f, E[e] == ENGI_ACID ? 32u * 22u * CTL : 4u * 110u * CTL);
            if (E[e] == ENGI_ACID) {                 /* two bars of 16ths at ~125 BPM, accents and slides */
                uint32_t held = 0;
                for (s = 0; s < 32u; s++) {
                    uint32_t nt = ACIDLINE[s % 16u];
                    trk_note_on(&trk[0], nt, ACC[s % 16u] ? 127 : 100);
                    if (held)                        /* the slide: the last note let go once the next is down */
                        trk_note_off(&trk[0], held), held = 0;
                    for (b = 0; b < 22u; b++) {
                        if (b == 12u && !SLIDE[s % 16u])
                            trk_note_off(&trk[0], nt);
                        track_render(&trk[0], buf, CTL);
                        for (i = 0; i < CTL; i++)
                            wav_put(f, buf[i] / 2, buf[i] / 2);
                    }
                    if (SLIDE[s % 16u])
                        held = nt;
                }
            } else {                                 /* four chords (MACRO CHORD: their roots) */
                for (s = 0; s < 4u; s++) {
                    uint32_t nn = E[e] == ENGI_MACRO && trk[0].p[P_E0] == 0 ? 1u : 3u, k;
                    for (k = 0; k < nn; k++)
                        trk_note_on(&trk[0], CH[s][k] + (E[e] == ENGI_MACRO && trk[0].p[P_E0] == 3 ? 0 : 12), 100);
                    for (b = 0; b < 110u; b++) {
                        if (b == 90u)
                            for (k = 0; k < nn; k++)
                                trk_note_off(&trk[0], CH[s][k] + (E[e] == ENGI_MACRO && trk[0].p[P_E0] == 3 ? 0 : 12));
                        track_render(&trk[0], buf, CTL);
                        for (i = 0; i < CTL; i++)
                            wav_put(f, buf[i] / 3, buf[i] / 3);
                    }
                }
            }
            fclose(f);
        }
}

int main(int argc, char **argv)
{
    t_acid();
    t_strings();
    t_macro();
    t_bounded();
    if (argc > 1)
        demos(argv[1]);
    printf(bad ? "eng2: %d FAILED\n" : "engines2 test passed\n", bad);
    return bad;
}
