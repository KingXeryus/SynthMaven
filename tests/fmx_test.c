/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper 0.6 FMX (CLIPPER_SYNTH 1): six operators, waves, PM / FM / RING, any routing.
 *   PRESETS   the tonal ones in tune (A3 = 220 Hz); every one bounded and finite with an 8-note chord; every voice
 *             ends after its release (the carriers' envelopes: engine_t.done)
 *   WAVES     each wave on one carrier: in tune (but NOISE), each its own spectrum; SAW at C7 (the 8-harmonic copy):
 *             no aliasing above -40 dB
 *   MODES     the same patch in PM, FM and RING: three sounds; RING on a 1.5x modulator: no 220 Hz left (the
 *             sidebands only); FM's index grows with the amount as PM's does
 *   ROUTING   more modulation: brighter (in steps); feedback: brighter; every one of the 40 algorithms on a 6-operator
 *             patch: sound, bounded, its carriers
 *   PEG       FM KICK: the pitch falls (the first 20 ms higher than the next 100)
 *   BANK      an FMX patch stored in a bank slot comes back exactly (PTCH Bn); FM6 sees that slot empty; a DX7 voice
 *             in a slot plays on FMX converted (in tune, bounded); FM6's own write clears the FMX mark
 *   DX7       each FM6 factory patch converted: the pitch FM6 plays it at
 *   COST      instructions per voice (the host's: a ratio to HYBRID's)
 *   fmx_test [DEMODIR]: every preset as a WAV */
#define main hostsim_main
#include "hostsim.c"
#undef main
static uint8_t nor[0x100000];
static int st_read(uint32_t off, void *dst, uint32_t n) { memcpy(dst, nor + off, n); return 0; }
static int st_erase(uint32_t off) { memset(nor + off, 0xFF, 4096); return 0; }
static int st_prog(uint32_t off, const void *src, uint32_t n)
{
    const uint8_t *q = src;
    uint32_t i;
    for (i = 0; i < n; i++)
        nor[off + i] &= q[i];
    return 0;
}
#include "../firmware/src/storage.c"
static uint8_t flash_ok = 1;
static union { uint8_t b[3840]; } proj_tmp;
#define FM6_BANK_XIP(copy) (nor + st_sector(OBJ_FM6BANK, copy) + ST_PAYLOAD_OFF)
#include "../firmware/src/fm6_bank.c"

static int fails;
static void ok(int c, const char *what)
{
    printf("fmx: %-96s %s\n", what, c ? "ok" : "FAIL");
    fails += !c;
}

#define NCAP (4096u * 12u)
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
static double rms(uint32_t a, uint32_t n)
{
    double e = 0;
    uint32_t i;
    for (i = a; i < a + n && i < ncap; i++)
        e += cap[i] * cap[i];
    return sqrt(e / n);
}
/* the fundamental by normalised autocorrelation over [a, a + n): the shortest lag (40 Hz .. 2.5 kHz) that is a peak
 * within 10 % of the best one, refined by a parabola */
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
/* the energy at f (Goertzel) over [a, a + n), relative to the whole */
static double at_freq(uint32_t a, uint32_t n, double f)
{
    double w = 2 * M_PI * f / FS, c = 2 * cos(w), s1 = 0, s2 = 0, e = 0;
    uint32_t i;
    for (i = a; i < a + n; i++) {
        double h = 0.5 - 0.5 * cos(2 * M_PI * (i - a) / n), s = cap[i] * h + c * s1 - s2;
        s2 = s1, s1 = s;
        e += cap[i] * cap[i] * h * h;
    }
    return (s1 * s1 + s2 * s2 - c * s1 * s2) / (e * n / 2 + 1e-9);
}
static double bright(uint32_t a, uint32_t n)
{
    double e = 0, d = 0;
    uint32_t i;
    for (i = a + 1; i < a + n && i < ncap; i++)
        e += cap[i] * cap[i], d += (cap[i] - cap[i - 1]) * (cap[i] - cap[i - 1]);
    return sqrt(d / (e + 1e-9));
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
static void dry(track_t *t)
{
    t->p[P_DIST] = t->p[P_CHOR] = t->p[P_DLY] = t->p[P_REV] = 0;
    t->p[P_LD_PIT] = t->p[P_LD_FLT] = t->p[P_LD_SHP] = t->p[P_LD_AMP] = 0;
    t->p[P_MWDST] = t->p[P_ATDST] = MD_OFF;
}
/* part 0 on FMX playing patch p (as the editor or the pages leave it) */
static void play_patch(const fmx_patch_t *p)
{
    track_t *t = &trk[0];
    quiet();
    host_preset(t, ENGI_FMX, 15);                     /* (INIT: PTCH 15, ALG PAT) */
    dry(t);
    t->p[P_E1] = 0;
    run(2);
    fmx_cur[0] = *p;
    fmx_dirty[0] = 1;
}
static fmx_patch_t one(uint32_t wave, uint32_t mode)  /* a carrier, sustained */
{
    fmx_patch_t p = FMX_FACTORY[FMX_NFACTORY - 1u];
    p.op[0].wave = (uint8_t)wave, p.op[0].mode = (uint8_t)mode;
    p.op[0].adsr[0] = 0, p.op[0].adsr[1] = 0, p.op[0].adsr[2] = 127, p.op[0].adsr[3] = 20;
    return p;
}
static fmx_patch_t two(uint32_t mode, uint32_t ratio, uint32_t amt)   /* 2 > 1 */
{
    fmx_patch_t p = one(FXW_SINE, mode);
    p.op[1] = p.op[0];
    p.op[1].mode = FXM_PM, p.op[1].coarse = (uint8_t)ratio, p.op[1].level = 127;
    p.mod[0][1] = (uint8_t)amt;
    p.out[1] = 0;
    return p;
}

static void t_presets(const char *demo)
{
    const engine_t *e = ENGINES[ENGI_FMX];
    uint32_t k, bad = 0, stuck = 0;
    double worst = 0;
    char w[200];
    for (k = 0; k < e->npresets; k++) {
        track_t *t = &trk[0];
        uint32_t i, n;
        double pk = 0, f;
        int tonal = strcmp(e->presets[k].name, "FMX BELL") && strcmp(e->presets[k].name, "RING BELLS") &&
                    strcmp(e->presets[k].name, "METAL HIT") && strcmp(e->presets[k].name, "FM KICK");
        quiet();
        host_preset(t, ENGI_FMX, k);
        dry(t);
        t->p[P_VOICE] = V_POLY;
        trk_note_on(t, 57, 100);
        run(150);
        f = pitch(2048, 2048);
        if (tonal) {
            double r = f / 220.0;
            double dev = fabs(r - 1.0);
            if (fabs(r - 0.5) < dev) dev = fabs(r - 0.5) * 2;   /* (an octave below: the 0.5x operators) */
            if (fabs(r - 2.0) < dev) dev = fabs(r - 2.0) / 2;
            worst = dev > worst ? dev : worst;
            if (dev > 0.03) {
                snprintf(w, sizeof w, "preset %s: in tune (%.1f Hz for 220)", e->presets[k].name, f);
                ok(0, w);
            }
        }
        if (demo) {
            char path[512];
            FILE *fh;
            snprintf(path, sizeof path, "%s/fmx-%02u.raw", demo, k);
            fh = fopen(path, "wb");
            if (fh) {
                static int16_t s16[NCAP];
                for (i = 0; i < ncap; i++)
                    s16[i] = (int16_t)(cap[i] / 4 > 32767 ? 32767 : cap[i] / 4 < -32767 ? -32767 : cap[i] / 4);
                fwrite(s16, 2, ncap, fh);
                fclose(fh);
            }
        }
        trk_note_off(t, 57);
        quiet();
        for (n = 0; n < 8u; n++)                         /* a chord */
            trk_note_on(t, 48 + 3 * n, 127);
        run(200);
        for (i = 0; i < ncap; i++) {
            if (!(cap[i] == cap[i]))
                bad++;
            pk = fabs(cap[i]) > pk ? fabs(cap[i]) : pk;
        }
        bad += pk >= 8388607.0;
        for (n = 0; n < 8u; n++)
            trk_note_off(t, 48 + 3 * n);
        run(3000);
        for (i = 0; i < NVOICE; i++)
            stuck += t->v[i].active;
    }
    snprintf(w, sizeof w, "presets: the tonal ones in tune (worst %.2f %%), all bounded and finite with chords", worst * 100);
    ok(worst <= 0.03 && !bad, w);
    ok(!stuck, "presets: every voice ends after its release (the carriers' envelopes)");
}

static void t_waves(void)
{
    uint32_t w, good = 1, distinct = 1;
    double prev[FMX_NWAVE];
    char m[200];
    for (w = 0; w < FMX_NWAVE; w++) {
        fmx_patch_t p = one(w, FXM_PM);
        play_patch(&p);
        trk_note_on(&trk[0], 57, 100);
        run(100);
        prev[w] = bright(2048, 4096);
        if (w != FXW_NOISE) {
            double f = pitch(2048, 2048);
            if (fabs(f / 220 - 1) > 0.01) {
                snprintf(m, sizeof m, "wave %s: in tune (%.1f Hz)", N_FMX_WAVE[w], f);
                ok(0, m);
                good = 0;
            }
        }
    }
    for (w = 1; w < FMX_NWAVE; w++)
        distinct &= fabs(prev[w] - prev[0]) > 0.002 || w == FXW_QUART;
    ok(good && distinct && prev[FXW_SAW] > prev[FXW_SINE] && prev[FXW_NOISE] > prev[FXW_SAW],
       "waves: each in tune on a carrier (but NOISE), each its own spectrum (SAW brighter than SINE, NOISE than SAW)");
    {   /* SAW at C7 (2093 Hz): the 8-harmonic copy, nothing between the harmonics */
        fmx_patch_t p = one(FXW_SAW, FXM_PM);
        double f0 = 440.0 * pow(2, (96 - 69) / 12.0), alias = 0, harm = 0;
        uint32_t k;
        play_patch(&p);
        trk_note_on(&trk[0], 96, 100);
        run(100);
        for (k = 1; f0 * k < FS / 2; k++)
            harm += at_freq(2048, 4096, f0 * k);
        for (k = 1; f0 * k + f0 / 2 < FS / 2; k++)
            alias = fmax(alias, at_freq(2048, 4096, f0 * k + f0 / 2));
        snprintf(m, sizeof m, "SAW at C7: no aliasing between the harmonics (worst %.1f dB)", 10 * log10(alias / harm + 1e-12));
        ok(10 * log10(alias / harm + 1e-12) < -40, m);
    }
}

static void t_modes(void)
{
    double b[3], e220[3], f1, f2;
    uint32_t md;
    char w[200];
    for (md = 0; md < 3u; md++) {
        fmx_patch_t p = two(md, 1, 80);
        if (md == FXM_RING)
            p.op[1].coarse = 1, p.op[1].fine = 50;        /* 1.5x */
        play_patch(&p);
        trk_note_on(&trk[0], 57, 100);
        run(100);
        b[md] = bright(2048, 4096);
        e220[md] = at_freq(2048, 4096, 220.0);
    }
    snprintf(w, sizeof w, "modes: PM, FM and RING three sounds (brightness %.3f %.3f %.3f)", b[0], b[1], b[2]);
    ok(fabs(b[0] - b[1]) > 0.005 && fabs(b[0] - b[2]) > 0.005 && fabs(b[1] - b[2]) > 0.005, w);
    snprintf(w, sizeof w, "RING on a 1.5x modulator: no 220 Hz left (%.4f of the energy; PM: %.3f)", e220[2], e220[0]);
    ok(e220[2] < 0.01 && e220[0] > 0.02, w);
    {
        fmx_patch_t p = two(FXM_FM, 1, 20), q = two(FXM_FM, 1, 90);
        play_patch(&p);
        trk_note_on(&trk[0], 57, 100);
        run(100);
        f1 = bright(2048, 4096);
        play_patch(&q);
        trk_note_on(&trk[0], 57, 100);
        run(100);
        f2 = bright(2048, 4096);
        snprintf(w, sizeof w, "FM (true): more amount, brighter (%.3f -> %.3f), still in tune (%.1f Hz)", f1, f2, pitch(2048, 2048));
        ok(f2 > f1 * 1.2 && fabs(pitch(2048, 2048) / 220 - 1) < 0.02, w);
    }
}

static void t_routing(void)
{
    double b[4];
    uint32_t i, a, good = 1, carriers_ok = 1;
    char w[200];
    for (i = 0; i < 4u; i++) {
        fmx_patch_t p = two(FXM_PM, 2, 20 + 35 * i);
        play_patch(&p);
        trk_note_on(&trk[0], 57, 100);
        run(100);
        b[i] = bright(2048, 4096);
        good &= !i || b[i] > b[i - 1];
    }
    snprintf(w, sizeof w, "routing: more modulation, brighter (%.3f %.3f %.3f %.3f)", b[0], b[1], b[2], b[3]);
    ok(good, w);
    {
        fmx_patch_t p = one(FXW_SINE, FXM_PM), q = p;
        double x, y;
        q.mod[0][0] = 110;
        play_patch(&p);
        trk_note_on(&trk[0], 57, 100);
        run(100);
        x = bright(2048, 4096);
        play_patch(&q);
        trk_note_on(&trk[0], 57, 100);
        run(100);
        y = bright(2048, 4096);
        snprintf(w, sizeof w, "feedback: brighter (%.3f -> %.3f), in tune (%.1f Hz)", x, y, pitch(2048, 2048));
        ok(y > x * 1.3 && fabs(pitch(2048, 2048) / 220 - 1) < 0.02, w);
    }
    for (a = 1, good = 1; a <= FMX_NALG; a++) {           /* every algorithm on six sounding operators */
        fmx_patch_t p = FMX_FACTORY[3];
        double pk = 0, r;
        uint32_t k, nc = 0;
        for (k = 0; k < 6u; k++)
            p.op[k] = FMX_FACTORY[3].op[k % 4u], p.op[k].coarse = (uint8_t)(1 + k % 3);
        play_patch(&p);
        trk[0].p[P_E1] = (int16_t)a;
        trk_note_on(&trk[0], 57, 100);
        run(60);
        r = rms(1024, 512);
        for (k = 0; k < ncap; k++)
            pk = fabs(cap[k]) > pk ? fabs(cap[k]) : pk;
        for (k = 0; k < 6u; k++)
            nc += (FMX_ALG[a - 1u][6] >> k) & 1u;
        carriers_ok &= fmx_eff[0].carriers == FMX_ALG[a - 1u][6];
        if (!(r > 100 && pk < 8388607.0 && nc)) {
            snprintf(w, sizeof w, "algorithm %u: sound (rms %.0f), bounded (peak %.0f)", a, r, pk);
            ok(0, w);
            good = 0;
        }
    }
    ok(good && carriers_ok, "the 40 algorithms on six operators: sound, bounded, their carriers to the output");
}

static void t_peg(void)
{
    track_t *t = &trk[0];
    double a, b;
    char w[200];
    quiet();
    host_preset(t, ENGI_FMX, 10);                       /* FM KICK */
    dry(t);
    trk_note_on(t, 36, 120);
    run(100);
    a = pitch(64, 700);
    b = pitch(1500, 1500);
    snprintf(w, sizeof w, "PEG: FM KICK falls (%.0f Hz in its first 20 ms, %.0f after)", a, b);
    ok(a > b * 1.3 && b > 0, w);
}

static void t_bank(void)
{
    fmx_patch_t p = FMX_FACTORY[3], q;
    uint8_t pk[FM6_PACKED];
    uint32_t i;
    double f;
    char w[200];
    memset(nor, 0xFF, sizeof nor);
    fm6_bank_boot();
    p.op[1].wave = FXW_TRI, p.op[2].mode = FXM_RING, p.mod[3][3] = 77, p.out[2] = 33;
    memcpy(p.name, "MY FMX    ", 10);
    ok(fmx_bank_store && !fmx_bank_store(3, (const uint8_t *)&p) && ((fm6_bank_fmx >> 3) & 1u),
       "bank: an FMX patch stored in slot 4 (marked as FMX's)");
    fmx_slot_patch(FMX_NFACTORY + 3u, &q);
    ok(!memcmp(&p, &q, sizeof p), "bank: PTCH B4 on FMX brings it back exactly");
    fm6_slot_get(FM6_NFACTORY + 3u, pk);
    ok(!memcmp(pk, FM6_INIT, FM6_PACKED), "bank: FM6 sees that slot empty (its init voice)");
    ok(!fm6_bank_put(5, FM6_FACTORY[1]) && !((fm6_bank_fmx >> 5) & 1u) && ((fm6_bank_fmx >> 3) & 1u),
       "bank: a DX7 voice in slot 6 (FM6's write), slot 4 still FMX's");
    {
        track_t *t = &trk[0];
        double pk2 = 0;
        quiet();
        host_preset(t, ENGI_FMX, 15);
        dry(t);
        t->p[P_E0] = (int16_t)(FMX_NFACTORY + 5u);
        t->p[P_E1] = 0;
        run(2);
        trk_note_on(t, 57, 100);
        run(150);
        f = pitch(2048, 2048);
        for (i = 0; i < ncap; i++)
            pk2 = fabs(cap[i]) > pk2 ? fabs(cap[i]) : pk2;
        snprintf(w, sizeof w, "bank: the DX7 voice in slot 6 plays on FMX, converted (%.1f Hz, peak %.0f)", f, pk2);
        ok(rms(2048, 2048) > 100 && pk2 < 8388607.0 && fmx_cur[0].alg >= 1u, w);
    }
    ok(!fm6_bank_put(3, FM6_FACTORY[2]) && !((fm6_bank_fmx >> 3) & 1u), "bank: FM6's write over slot 4 clears its FMX mark");
}

/* each FM6 factory patch converted: the pitch FM6 plays it at */
static void t_dx7(void)
{
    uint32_t k, good = 1;
    char w[200];
    for (k = 0; k < FM6_NFACTORY; k++) {
        uint8_t v[FP_SIZE + 1u];
        fmx_patch_t p;
        double fa, fb;
        track_t *t = &trk[0];
        quiet();
        host_preset(t, ENGI_FM6, 0);
        dry(t);
        t->p[P_E7] = (int16_t)k;
        fm6_track_loaded(t);
        run(4);
        trk_note_on(t, 57, 100);
        run(150);
        fa = pitch(2048, 2048);
        fm6_unpack(FM6_FACTORY[k], v);
        fmx_from_dx7(v, &p);
        play_patch(&p);
        trk_note_on(&trk[0], 57, 100);
        run(150);
        fb = pitch(2048, 2048);
        if (fa > 0 && fabs(fb / fa - 1) > 0.03 && fabs(fb / fa - 2) > 0.06 && fabs(fb / fa - 0.5) > 0.015) {
            snprintf(w, sizeof w, "DX7 %u converted: FM6 %.1f Hz, FMX %.1f Hz", k, fa, fb);
            ok(0, w);
            good = 0;
        }
    }
    ok(good, "DX7: FM6's factory patches converted play at FM6's pitch");
}

/* the factory patches for the web editor's test (web/test_web.mjs fmxPatch): build/host/fmx_factory.json */
static void factory_json(void)
{
    FILE *f = fopen("build/host/fmx_factory.json", "w");
    uint32_t k, i;
    if (!f)
        return;
    fprintf(f, "{\"size\": %u, \"factory\": [", (unsigned)sizeof(fmx_patch_t));
    for (k = 0; k < FMX_NFACTORY; k++) {
        const uint8_t *b = (const uint8_t *)&FMX_FACTORY[k];
        fprintf(f, "%s[", k ? ", " : "");
        for (i = 0; i < sizeof(fmx_patch_t); i++)
            fprintf(f, "%s%u", i ? "," : "", b[i]);
        fprintf(f, "]");
    }
    fprintf(f, "], \"alg\": [");
    for (k = 0; k < FMX_NALG; k++) {
        fprintf(f, "%s[", k ? ", " : "");
        for (i = 0; i < 7u; i++)
            fprintf(f, "%s%u", i ? "," : "", FMX_ALG[k][i]);
        fprintf(f, "]");
    }
    fprintf(f, "], \"waves\": [");
    for (k = 0; k < FMX_NWAVE; k++)
        fprintf(f, "%s\"%s\"", k ? ", " : "", N_FMX_WAVE[k]);
    fprintf(f, "]}\n");
    fclose(f);
}

int main(int argc, char **argv)
{
    uint32_t i;
    factory_json();
    host_tracks_init();
    for (i = 0; i < NPART; i++)
        host_preset(&trk[i], i ? 1u : ENGI_FMX, 0);
    clipper_proj_neutral();
    trk[1].p[P_LYMODE] = LY_OFF;
    memset(nor, 0xFF, sizeof nor);
    fm6_bank_boot();
    t_presets(argc > 1 ? argv[1] : 0);
    t_waves();
    t_modes();
    t_routing();
    t_peg();
    t_bank();
    t_dx7();
    printf(fails ? "fmx: %d FAILED\n" : "fmx: all passed\n", fails);
    return fails;
}
