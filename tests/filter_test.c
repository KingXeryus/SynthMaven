/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* SynthMaven 0.2 filters (firmware/src/dsp.c ladder_*, eng_hybrid.c hsvf_sat, HF_MOOG, ANALOG's P_FMODEL):
 *   build/host/filter_test
 * 1. the ladder's response, resonance 0: flat well below the cutoff, about -12 dB at it (four poles of -3 dB),
 *    24 dB an octave above it (-40 dB or less two octaves up), at cutoffs from 100 Hz to 8 kHz
 * 2. no dead band: at the lowest cutoff a 2 kHz tone is gone (< -70 dB), a 20 Hz one passes
 * 3. resonance: at the top it sings on its own at the cutoff (an impulse, then silence: still ringing a second
 *    later, its pitch within a semitone of the cutoff, bounded by the tanh); below k = 4 it dies away
 * 4. bounded: a minute of full-scale noise with the cutoff and resonance jumping every block, the extremes too:
 *    every output finite and within range, the states within 3 x the tanh's ceiling (run it under UBSan too)
 * 5. the saturation: a hot input comes out compressed (tanh), a quiet one clean (THD < 1 %)
 * 6. HYBRID: a MOOG patch sounds, is darker than open, bounded; switching LP12 -> MOOG -> LP12 under a held note
 *    does not burst; the SVF's soft states: a screaming LP12 / PEAK stays bounded and finite
 * 7. ANALOG with MODL = MOOG: darker than SVF at the same cutoff above it (24 vs 12 dB), bounded
 * Exit status: the number of failed checks. */
#define VOICE_MAX 125000                  /* a voice's ceiling out of track_render (HYBRID: hybrid_test HY_VOICE_MAX) */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <math.h>

static int bad;
static void check(const char *what, int ok)
{
    printf("filter: %-92s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

static double db(double x) { return 20.0 * log10(x > 1e-12 ? x : 1e-12); }

static int cut_index(double hz)                  /* the cutoff value (0..127 << 8) nearest hz */
{
    int i, best = 0;
    for (i = 0; i < 128; i++)
        if (fabs(log(CUTOFF_HZ[i] / hz)) < fabs(log(CUTOFF_HZ[best] / hz)))
            best = i;
    return best;
}

/* the ladder's gain (dB) for a sine of hz at amplitude amp, the cutoff (0..127 << 8) and resonance */
static double ladder_gain(int32_t cut, int32_t res, double hz, double amp)
{
    ladder_t c;
    int32_t s[4] = {0, 0, 0, 0};
    double e_in = 0, e_out = 0;
    int i, n = FS;
    ladder_coef(&c, cut, res);
    for (i = 0; i < n; i++) {
        int32_t x = (int32_t)lrint(amp * sin(2 * M_PI * hz * i / FS)), y = ladder_tick(&c, x, s);
        if (i >= n / 2) {
            e_in += (double)x * x;
            e_out += (double)y * y;
        }
    }
    return db(sqrt(e_out / e_in));
}

/* zero crossings -> frequency of buf[n] */
static double pitch_of(const int32_t *b, int n)
{
    int i, z = 0, first = -1, last = -1;
    for (i = 1; i < n; i++)
        if (b[i - 1] < 0 && b[i] >= 0) {
            if (first < 0)
                first = i;
            last = i;
            z++;
        }
    return z > 1 ? (double)(z - 1) * FS / (last - first) : 0;
}

static void t_response(void)
{
    static const double FC[] = {100, 400, 1000, 3000, 8000};
    unsigned k;
    char w[120];
    for (k = 0; k < sizeof FC / sizeof FC[0]; k++) {
        int ci = cut_index(FC[k]);
        double fc = CUTOFF_HZ[ci];
        double lo = ladder_gain(ci << 8, 0, fc / 8, 16000), at = ladder_gain(ci << 8, 0, fc, 16000);
        double up = fc * 4 < FS * 0.45 ? ladder_gain(ci << 8, 0, fc * 4, 16000) : -99;
        snprintf(w, sizeof w, "ladder at %5.0f Hz: fc/8 %+5.1f dB, fc %+5.1f dB (-12), 4 fc %+6.1f dB (<= -40)", fc, lo, at, up);
        check(w, fabs(lo) < 1.0 && at < -9.0 && at > -15.0 && up <= -40.0);
    }
}

static void t_deadband(void)
{
    double a = ladder_gain(0, 0, 2000, 30000), b = ladder_gain(0, 0, 20, 30000);
    char w[120];
    snprintf(w, sizeof w, "lowest cutoff (%d Hz): 2 kHz %+.1f dB (< -70), 20 Hz %+.1f dB (passes, > -10)", CUTOFF_HZ[0], a, b);
    check(w, a < -70.0 && b > -10.0);          /* (20 Hz at a 27 Hz four-pole cutoff: about -7 dB) */
}

static void t_resonance(void)
{
    int ci = cut_index(440), i, n = FS * 2;
    ladder_t c;
    int32_t s[4] = {0, 0, 0, 0}, mx = 0, tail = 0;
    static int32_t y[FS * 2];
    double f;
    char w[120];
    ladder_coef(&c, ci << 8, 127);
    for (i = 0; i < n; i++) {
        y[i] = ladder_tick(&c, i < 32 ? 20000 : 0, s);
        if (abs(y[i]) > mx)
            mx = abs(y[i]);
        if (i > FS && abs(y[i]) > tail)
            tail = abs(y[i]);
    }
    f = pitch_of(y + FS, FS);
    snprintf(w, sizeof w, "RES 127 at %d Hz: sings on its own (peak %d, a second later %d), at %.0f Hz (12 * log2 %.2f st)",
             CUTOFF_HZ[ci], mx, tail, f, f > 0 ? 12 * log2(f / CUTOFF_HZ[ci]) : 99.0);
    check(w, tail > 4000 && mx < 140000 && f > 0 && fabs(12 * log2(f / CUTOFF_HZ[ci])) < 1.0);
    ladder_coef(&c, ci << 8, 100);
    memset(s, 0, sizeof s);
    tail = 0;
    for (i = 0; i < n; i++) {
        int32_t v = ladder_tick(&c, i < 32 ? 20000 : 0, s);
        if (i > FS && abs(v) > tail)
            tail = abs(v);
    }
    snprintf(w, sizeof w, "RES 100 (k < 4): rings, then dies away (a second later %d)", tail);
    check(w, tail < 50);
}

static void t_bounded(void)
{
    ladder_t c;
    int32_t s[4] = {0, 0, 0, 0}, nz = 12345, mx = 0, smx = 0;
    long i, blocks = 60L * FS / CTL;
    int ok = 1;
    for (i = 0; i < blocks; i++) {
        uint32_t r = noise32(&nz), j;
        int32_t cut = (int32_t)(r % (128u << 8)), res = (int32_t)((r >> 16) % 128u);
        if (i % 7 == 0)
            cut = (i & 8) ? 127 << 8 : 0, res = 127;
        ladder_coef(&c, cut, res);
        for (j = 0; j < CTL; j++) {
            int32_t x = (int32_t)(noise32(&nz) % 400001u) - 200000, y = ladder_tick(&c, x, s), k;
            if (abs(y) > mx)
                mx = abs(y);
            for (k = 0; k < 4; k++)
                if (abs(s[k]) > smx)
                    smx = abs(s[k]);
        }
    }
    ok = mx < 400000 && smx < 1000000000;         /* (Q12: 3 x the tanh's 65536 is 8e8) */
    {
        char w[120];
        snprintf(w, sizeof w, "a minute of +-200000 noise, cutoff and resonance jumping: output peak %d, states %d (Q12)", mx, smx);
        check(w, ok);
    }
}

static void t_saturation(void)
{
    int ci = cut_index(8000);
    ladder_t c;
    int32_t s[4] = {0, 0, 0, 0};
    double hot_in = 0, hot_out = 0, h1 = 0, h3c = 0, h3s = 0, h1s = 0;
    int i, n = FS;
    char w[120];
    ladder_coef(&c, ci << 8, 0);
    for (i = 0; i < n; i++) {                        /* quiet: 220 Hz at 4000, the 3rd harmonic's share */
        double ph = 2 * M_PI * 220 * i / FS;
        int32_t y = ladder_tick(&c, (int32_t)lrint(4000 * sin(ph)), s);
        if (i >= n / 2) {
            h1 += y * cos(ph), h1s += y * sin(ph);
            h3c += y * cos(3 * ph), h3s += y * sin(3 * ph);
        }
    }
    {
        double thd = sqrt(h3c * h3c + h3s * h3s) / sqrt(h1 * h1 + h1s * h1s);
        memset(s, 0, sizeof s);
        for (i = 0; i < n; i++) {                    /* hot: 220 Hz at 190000 */
            int32_t x = (int32_t)lrint(190000 * sin(2 * M_PI * 220 * i / FS)), y = ladder_tick(&c, x, s);
            if (i >= n / 2)
                hot_in += (double)x * x, hot_out += (double)y * y;
        }
        snprintf(w, sizeof w, "a quiet tone clean (3rd harmonic %.2f %%), a hot one compressed (%+.1f dB)",
                 thd * 100, db(sqrt(hot_out / hot_in)));
        check(w, thd < 0.01 && db(sqrt(hot_out / hot_in)) < -3.0);
    }
}

/* ------------------------------------------------------------ engines --- */
static hy_patch_t init_patch(void)
{
    hy_patch_t p = HY_FACTORY[HY_INIT];               /* INIT: one saw, LP12 open */
    memset(p.mod, 0, sizeof p.mod);
    p.drift = 0;
    return p;
}
static void setup_engine(uint32_t eng, uint32_t preset)
{
    memset(trk, 0, sizeof trk);
    memset(eng_arena, 0, sizeof eng_arena);
    memset(eng_arena_own, 0, sizeof eng_arena_own);
    memset(part_ctl, 0, sizeof part_ctl);
    memset(hy_gen_seen, 0, sizeof hy_gen_seen);
    memset((void *)hy_gen, 0, sizeof hy_gen);
    memset(hy_slot_seen, 0, sizeof hy_slot_seen);
    host_tracks_init();
    host_preset(&trk[0], eng, preset);
    trk[0].p[P_VOICE] = V_POLY;
    trk[0].p[P_ATK] = 0, trk[0].p[P_DEC] = 64, trk[0].p[P_SUS] = 127, trk[0].p[P_REL] = 10;
    trk[0].p[P_DIST] = trk[0].p[P_CHOR] = trk[0].p[P_DLY] = trk[0].p[P_REV] = 0;
    trk[0].p[P_LD_PIT] = trk[0].p[P_LD_FLT] = trk[0].p[P_LD_SHP] = trk[0].p[P_LD_AMP] = 0;
}
static void hy_play(const hy_patch_t *p)
{
    setup_engine(ENGI_HYBRID, HY_INIT);
    hy_set_patch(0, p);
}

/* frames of the master into L[], a note held; returns the peak; *hf: the energy of the first difference (bright) */
static int32_t play(uint32_t note, uint32_t blocks, double *rms, double *hf, int32_t *L)
{
    uint32_t b, i;
    int32_t mx = 0, prev = 0;
    double e = 0, d = 0;
    trk_note_on(&trk[0], note, 100);
    for (b = 0; b < blocks; b++) {
        static int32_t l[CTL];
        track_render(&trk[0], l, CTL);
        for (i = 0; i < CTL; i++) {
            int32_t x = l[i];
            if (L)
                L[b * CTL + i] = x;
            if (abs(x) > mx)
                mx = abs(x);
            if (b > blocks / 4)
                e += (double)x * x, d += (double)(x - prev) * (x - prev);
            prev = x;
        }
    }
    trk_note_off(&trk[0], note);
    if (rms)
        *rms = sqrt(e / (blocks * CTL));
    if (hf)
        *hf = sqrt(d / (blocks * CTL));
    return mx;
}

static void t_hybrid(void)
{
    hy_patch_t p = init_patch();
    double r_open, h_open, r_moog, h_moog;
    int32_t mx;
    char w[140];
    p.flt[0].type = HF_LP12, p.flt[0].cut = 127, p.flt[0].res = 0;
    hy_play(&p);
    play(48, 200, &r_open, &h_open, 0);
    p.flt[0].type = HF_MOOG, p.flt[0].cut = 60, p.flt[0].res = 40;
    hy_play(&p);
    mx = play(48, 200, &r_moog, &h_moog, 0);
    snprintf(w, sizeof w, "HYBRID MOOG: sounds (rms %.0f), darker than open (brightness %.2f vs %.2f of the level), peak %d",
             r_moog, h_moog / r_moog, h_open / r_open, mx);
    check(w, r_moog > 300 && h_moog / r_moog < 0.5 * h_open / r_open && mx < VOICE_MAX);
    {   /* LP12 -> MOOG -> LP12 under a held note, resonant: no burst */
        static int32_t L[400 * CTL];
        uint32_t b, i;
        int32_t before = 0, after = 0;
        p.flt[0].type = HF_LP12, p.flt[0].cut = 70, p.flt[0].res = 120;
        hy_play(&p);
        trk_note_on(&trk[0], 48, 100);
        for (b = 0; b < 400; b++) {
            static int32_t l[CTL];
            if (b == 150 || b == 300) {
                p.flt[0].type = b == 150 ? HF_MOOG : HF_LP12;
                hy_set_patch(0, &p);
            }
            track_render(&trk[0], l, CTL);
            for (i = 0; i < CTL; i++) {
                L[b * CTL + i] = l[i];
                if (b > 50 && b < 150 && abs(l[i]) > before)
                    before = abs(l[i]);
                if (b >= 150 && abs(l[i]) > after)
                    after = abs(l[i]);
            }
        }
        trk_note_off(&trk[0], 48);
        snprintf(w, sizeof w, "LP12 -> MOOG -> LP12 under a held resonant note: no burst (peak %d after, %d before)", after, before);
        check(w, after < VOICE_MAX && after < 3 * before + 2000);
    }
    {   /* the SVF's soft states: a screaming LP12 and PEAK with a hot drive */
        static const uint8_t T[2] = {HF_LP12, HF_PEAK};
        unsigned k;
        for (k = 0; k < 2; k++) {
            double r;
            p = init_patch();
            p.osc[0].type = HO_SUPER, p.osc[0].level = 127, p.osc[0].shape = 127;
            p.osc[1].type = HO_SAW, p.osc[1].level = 127;
            p.drive = 127;
            p.flt[0].type = T[k], p.flt[0].cut = 50, p.flt[0].res = 127;
            hy_play(&p);
            mx = play(36, 300, &r, 0, 0);
            snprintf(w, sizeof w, "%s at RES 127, two hot oscillators, drive 127: bounded (peak %d), not silent (rms %.0f)",
                     N_HY_FLT[T[k]], mx, r);
            check(w, mx <= VOICE_MAX && r > 100 && isfinite(r));
        }
    }
}

static void t_analog(void)
{
    double r_svf, h_svf, r_moog, h_moog;
    int32_t mx;
    char w[140];
    setup_engine(0, 0);                              /* (ENGINES[0]: ANALOG) */
    trk[0].p[P_E0] = 0, trk[0].p[P_E2] = 0, trk[0].p[P_E3] = 0;   /* one saw */
    trk[0].p[P_E4] = 60, trk[0].p[P_E5] = 20, trk[0].p[P_E6] = 0, trk[0].p[P_E7] = 0;
    trk[0].p[P_ED_FLT] = 0, trk[0].p[P_FMODEL] = FM_SVF;
    play(48, 200, &r_svf, &h_svf, 0);
    trk[0].p[P_FMODEL] = FM_MOOG;
    mx = play(48, 200, &r_moog, &h_moog, 0);
    snprintf(w, sizeof w, "ANALOG MODL MOOG: darker than SVF at the same cutoff (brightness %.3f vs %.3f), sounds, peak %d",
             h_moog / r_moog, h_svf / r_svf, mx);
    check(w, r_moog > 300 && h_moog / r_moog < h_svf / r_svf && mx < VOICE_MAX);
    trk[0].p[P_E5] = 127, trk[0].p[P_E6] = 127, trk[0].p[P_E4] = 40;
    {
        double r;
        mx = play(36, 300, &r, 0, 0);
        snprintf(w, sizeof w, "ANALOG MOOG at RES 127, DRV 127: bounded (peak %d), finite", mx);
        check(w, mx <= VOICE_MAX && isfinite(r));
    }
}

int main(void)
{
    t_response();
    t_deadband();
    t_resonance();
    t_bounded();
    t_saturation();
    t_hybrid();
    t_analog();
    printf(bad ? "filter: %d FAILED\n" : "filter test passed\n", bad);
    return bad;
}
