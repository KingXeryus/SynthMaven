/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* HYBRID engine test (firmware/src/eng_hybrid.c, hybrid_patches.h) on the host, through hostsim.c.
 *   build/host/hybrid_test [DEMODIR]
 * 1. oscillators: every type sounds at the pitch of its key (zero crossings), bounded; COARSE +12 an octave
 *    up, FINE +50 a quarter tone; SYNC follows its master; NOISE has no pitch but is not silent.
 * 2. filters: through LP12 / LP24 / BP the noise gets darker, through HP brighter; LP24 darker than LP12;
 *    NOTCH and PEAK bounded; the resonance at the top rings but stays bounded.
 * 3. modulation: ENV2 -> CUT (brighter at the start), M1 -> CUT, LFO2 -> PITCH (vibrato), its VIA (the mod
 *    wheel at 0: no vibrato), LFO2 synced to the tempo (its period), the RAND source differs note to note.
 * 4. controllers: the pitch bend (2 semitones on ANALOG, the patch's range on HYBRID), the sustain pedal
 *    (CC 64: held, then released and free), channel pressure (AT -> CUT), CC 16..19 (the macros).
 * 5. patches: hy_set_patch (the editor's) is played from the next block, PTCH takes over again; random patches
 *    (sanitised) stay bounded and finite and free their voices; UNISON on a HYBRID patch.
 * 6. cost: instructions per sample and voice of each oscillator type, each filter, each factory patch, counted
 *    under callgrind (--toggle-collect, as regress.c), with the device estimate (1.7 % per 100, the FM6
 *    calibration of tests/fm6_test.c); the factory patches must fit HY_COST_MAX.
 * 7. demos into DEMODIR: every factory patch playing a phrase (WAV). */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <math.h>

#define HY_COST_MAX 400.0         /* host instructions per sample and voice, the heaviest factory patch (a regression
                                   * guard: SYNTHMAVEN's two SUPERs are ~390, ~6.6 % each; ANALOG's WARM PAD ~170) */
#define DEV_PER_100 1.7
/* the most a voice can give by construction: the soft knee's ceiling (2 x 32384), LEVEL 127 (x 1.27),
 * VOICE_FS, the final x 2 (as ANALOG) */
#define HY_VOICE_MAX (64768.0 * 41600 / 32768 * VOICE_FS / 32768 * 2)           /* device CPU % per 100 host instructions a sample (tests/fm6_test.c) */

static int bad;
static void check(const char *what, int ok)
{
    printf("hybrid: %-92s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

/* ------------------------------------------------------------ helpers --- */
static hy_patch_t init_patch(void)
{
    hy_patch_t p = HY_FACTORY[HY_INIT];               /* INIT: one saw, LP12 open */
    memset(p.mod, 0, sizeof p.mod);
    p.flt[0].type = HF_OFF;
    p.drift = 0;
    return p;
}

/* part 0 = HYBRID with patch p, POLY, a flat envelope, no sends */
static void setup(const hy_patch_t *p)
{
    memset(trk, 0, sizeof trk);
    memset(eng_arena, 0, sizeof eng_arena);
    memset(eng_arena_own, 0, sizeof eng_arena_own);
    memset(part_ctl, 0, sizeof part_ctl);
    memset(hy_gen_seen, 0, sizeof hy_gen_seen);
    memset((void *)hy_gen, 0, sizeof hy_gen);
    memset(hy_slot_seen, 0, sizeof hy_slot_seen);
    host_tracks_init();
    host_preset(&trk[0], ENGI_HYBRID, HY_INIT);
    trk[0].p[P_VOICE] = V_POLY;
    trk[0].p[P_ATK] = 0;
    trk[0].p[P_DEC] = 64;
    trk[0].p[P_SUS] = 127;
    trk[0].p[P_REL] = 30;
    trk[0].p[P_CHOR] = trk[0].p[P_DLY] = trk[0].p[P_REV] = trk[0].p[P_DIST] = 0;
    trk[0].p[P_ED_FLT] = 0;
    if (p)
        hy_set_patch(0, p);
}

/* the part's voices (track_render) for n samples from a note-on of note; released at rel (0: never) */
static void render(uint32_t note, uint32_t vel, double *y, uint32_t n, uint32_t rel)
{
    static int32_t b[CTL];
    uint32_t i, k;
    trk_note_on(&trk[0], note, vel);
    for (i = 0; i < n; i += CTL) {
        if (rel && i == (rel / CTL) * CTL)
            trk_note_off(&trk[0], note);
        track_render(&trk[0], b, CTL);
        for (k = 0; k < CTL && i + k < n; k++)
            y[i + k] = b[k];
    }
}

static double peak(const double *y, uint32_t a, uint32_t b)
{
    double m = 0;
    for (; a < b; a++)
        m = fabs(y[a]) > m ? fabs(y[a]) : m;
    return m;
}
static double rms(const double *y, uint32_t a, uint32_t b)
{
    double s = 0;
    uint32_t i;
    for (i = a; i < b; i++)
        s += y[i] * y[i];
    return b > a ? sqrt(s / (b - a)) : 0;
}
/* the frequency from the rising zero crossings of the signal less its mean (Hz) */
static double freq(const double *y, uint32_t a, uint32_t b)
{
    double m = 0, first = -1, last = -1;
    uint32_t i, n = 0;
    for (i = a; i < b; i++)
        m += y[i];
    m /= (b - a);
    for (i = a + 1u; i < b; i++)
        if (y[i - 1] - m < 0 && y[i] - m >= 0) {
            double t = i - 1 + (m - y[i - 1]) / (y[i] - y[i - 1]);
            if (first < 0)
                first = t;
            last = t;
            n++;
        }
    return n > 1 ? (n - 1) * FS / (last - first) : 0;
}
/* brightness: rms of the first difference over rms (0 .. 2): higher = more highs */
static double bright(const double *y, uint32_t a, uint32_t b)
{
    double d = 0, s = 0;
    uint32_t i;
    for (i = a + 1u; i < b; i++) {
        d += (y[i] - y[i - 1]) * (y[i] - y[i - 1]);
        s += y[i] * y[i];
    }
    return s > 0 ? sqrt(d / s) : 0;
}
static int finite_all(const double *y, uint32_t n)
{
    uint32_t i;
    for (i = 0; i < n; i++)
        if (!isfinite(y[i]))
            return 0;
    return 1;
}
static uint32_t voices_on(void)
{
    uint32_t i, n = 0;
    for (i = 0; i < NVOICE; i++)
        n += trk[0].v[i].active;
    return n;
}
static void run_blocks(uint32_t nb)
{
    static int32_t b[CTL];
    while (nb--)
        track_render(&trk[0], b, CTL);
}
static double hz_of(uint32_t note) { return 440.0 * pow(2.0, ((double)note - 69.0) / 12.0); }

#define N1 (FS)
static double Y[N1 * 2], Z[N1 * 2];

/* --------------------------------------------------------- oscillators --- */
static void t_osc(void)
{
    static const uint8_t SHAPES[HO_N] = {0, 0, 40, 0, 30, 64, 0, 40, 0, 50, 0};
    uint32_t ty;
    char what[160];
    for (ty = HO_SAW; ty < HO_N; ty++) {
        hy_patch_t p = init_patch();
        double f, pk, want = hz_of(57);
        p.osc[0].type = (uint8_t)ty;
        p.osc[0].shape = SHAPES[ty];
        p.osc[0].level = 127;
        if (ty == HO_FM) {                             /* osc 2 FM from osc 1 (a silent sine modulator) */
            p.osc[0].type = HO_SINE;
            p.osc[0].level = 0;
            p.osc[1].type = HO_FM;
            p.osc[1].level = 127;
            p.osc[1].shape = 30;
        } else if (ty == HO_SYNC) {                    /* osc 2 synced to osc 1, an octave and a half above */
            p.osc[0].level = 0;
            p.osc[1].type = HO_SYNC;
            p.osc[1].level = 127;
            p.osc[1].shape = 50;
        }
        setup(&p);
        render(57, 127, Y, N1, 0);
        f = freq(Y, FS / 4, N1);
        pk = peak(Y, 0, N1);
        if (ty == HO_NOISE) {
            snprintf(what, sizeof what, "osc NOISE: sounds (rms %.0f), bounded (peak %.0f)", rms(Y, FS / 4, N1), pk);
            check(what, rms(Y, FS / 4, N1) > 500 && pk < 2.2 * VOICE_FS && finite_all(Y, N1));
            continue;
        }
        if (ty == HO_FOLD || ty == HO_FM || ty == HO_SINE || ty == HO_SYNC) {   /* (harmonics cross zero more often:
                                                                               * the period, by autocorrelation) */
            double best = 0, c = 0;
            uint32_t lag, lo = (uint32_t)(FS / want * 0.8), hi = (uint32_t)(FS / want * 1.25), i;
            for (lag = lo; lag <= hi; lag++) {
                c = 0;
                for (i = FS / 4; i < FS / 4 + 8192u; i++)
                    c += Y[i] * Y[i + lag];
                if (c > best) {
                    best = c;
                    f = (double)FS / lag;
                }
            }
        }
        snprintf(what, sizeof what, "osc %-5s: A3 at %.1f Hz (want %.1f), peak %.0f (bounded)", N_HY_OSC[ty], f, want, pk);
        check(what, fabs(f / want - 1) < (ty == HO_FOLD || ty == HO_FM || ty == HO_SINE || ty == HO_SYNC ? 0.01 : 0.003) &&
                    pk > 3000 && pk < 2.2 * VOICE_FS && finite_all(Y, N1));
    }
    {   /* COARSE +12, FINE +50 */
        hy_patch_t p = init_patch();
        double f0, f1, f2;
        setup(&p);
        render(57, 127, Y, N1, 0);
        f0 = freq(Y, FS / 4, N1);
        p.osc[0].coarse = 12;
        setup(&p);
        render(57, 127, Y, N1, 0);
        f1 = freq(Y, FS / 4, N1);
        p.osc[0].coarse = 0;
        p.osc[0].fine = 50;
        setup(&p);
        render(57, 127, Y, N1, 0);
        f2 = freq(Y, FS / 4, N1);
        snprintf(what, sizeof what, "COARSE +12: x%.4f (2), FINE +50: x%.4f (%.4f)", f1 / f0, f2 / f0, pow(2, 50 / 1200.0));
        check(what, fabs(f1 / f0 - 2) < 0.004 && fabs(f2 / f0 - pow(2, 50 / 1200.0)) < 0.003);
    }
    {   /* SUPER: wider with SHP (the beat: the envelope's spread), still at its pitch */
        hy_patch_t p = init_patch();
        double b0, b1;
        p.osc[0].type = HO_SUPER;
        p.osc[0].shape = 0;
        setup(&p);
        render(45, 127, Y, N1, 0);
        b0 = rms(Y, FS / 4, N1);
        p.osc[0].shape = 120;
        setup(&p);
        render(45, 127, Z, N1, 0);
        b1 = rms(Z, FS / 4, N1);
        snprintf(what, sizeof what, "SUPER: detune 0 rms %.0f, detune 120 rms %.0f (both sound, bounded %.0f)", b0, b1,
                 peak(Z, 0, N1));
        check(what, b0 > 2000 && b1 > 2000 && peak(Z, 0, N1) < 2.2 * VOICE_FS);
    }
}

/* -------------------------------------------------------------- filters --- */
static void t_filters(void)
{
    hy_patch_t p = init_patch();
    double b_open, b[HF_N], pk[HF_N];
    uint32_t ty;
    char what[200];
    p.osc[0].type = HO_NOISE;
    p.osc[0].shape = 0;
    setup(&p);
    render(60, 127, Y, N1 / 2, 0);
    b_open = bright(Y, FS / 8, N1 / 2);
    for (ty = HF_LP12; ty < HF_N; ty++) {
        p.flt[0].type = (uint8_t)ty;
        p.flt[0].cut = 64;
        p.flt[0].res = 40;
        p.flt[0].key = 0;
        setup(&p);
        render(60, 127, Y, N1 / 2, 0);
        b[ty] = bright(Y, FS / 8, N1 / 2);
        pk[ty] = peak(Y, 0, N1 / 2);
    }
    snprintf(what, sizeof what, "filters on noise, brightness: open %.3f LP12 %.3f LP24 %.3f BP %.3f HP %.3f", b_open,
             b[HF_LP12], b[HF_LP24], b[HF_BP], b[HF_HP]);
    check(what, b[HF_LP12] < b_open * 0.5 && b[HF_LP24] < b[HF_LP12] && b[HF_BP] < b_open && b[HF_HP] > b[HF_LP12] * 2);
    snprintf(what, sizeof what, "NOTCH %.3f, PEAK %.3f: bounded (peaks %.0f %.0f)", b[HF_NOTCH], b[HF_PEAK], pk[HF_NOTCH],
             pk[HF_PEAK]);
    check(what, pk[HF_NOTCH] < 2.2 * VOICE_FS && pk[HF_PEAK] < 2.2 * VOICE_FS && b[HF_NOTCH] > 0 && b[HF_PEAK] > 0);
    {   /* the top resonance on a saw sweep: rings, bounded */
        hy_patch_t q = init_patch();
        q.flt[0].type = HF_LP24;
        q.flt[0].cut = 20;
        q.flt[0].res = 127;
        q.flt[0].env = 63;
        q.env2[0] = 0;
        q.env2[1] = 100;
        q.env2[2] = 0;
        setup(&q);
        render(36, 127, Y, N1, 0);
        snprintf(what, sizeof what, "LP24 at full resonance, swept by ENV2: bounded (peak %.0f), finite", peak(Y, 0, N1));
        check(what, peak(Y, 0, N1) < 2.2 * VOICE_FS && finite_all(Y, N1));
    }
    {   /* routing: PAR with BAL at each end = that filter alone; SPLIT sums the two paths */
        hy_patch_t q = init_patch();
        double lp, hp;
        q.osc[0].type = HO_NOISE;
        q.route = HR_PAR;
        q.flt[0] = (hy_flt_t){HF_LP12, 50, 0, 0, 0, 0};
        q.flt[1] = (hy_flt_t){HF_HP, 90, 0, 0, 0, 0};
        q.fbal = 0;
        setup(&q);
        render(60, 127, Y, N1 / 2, 0);
        lp = bright(Y, FS / 8, N1 / 2);
        q.fbal = 127;
        setup(&q);
        render(60, 127, Y, N1 / 2, 0);
        hp = bright(Y, FS / 8, N1 / 2);
        snprintf(what, sizeof what, "PAR: BAL 0 (F1 LP) brightness %.3f < BAL 127 (F2 HP) %.3f", lp, hp);
        check(what, lp * 3 < hp);
    }
}

/* ----------------------------------------------------------- modulation --- */
/* the instantaneous frequency's spread (Hz) over the zero-crossing periods of y[a..b) */
static double fspread(const double *y, uint32_t a, uint32_t b)
{
    double lo = 1e9, hi = 0, prev = -1;
    uint32_t i;
    for (i = a + 1u; i < b; i++)
        if (y[i - 1] < 0 && y[i] >= 0) {
            double t = i - 1 + (-y[i - 1]) / (y[i] - y[i - 1]);
            if (prev >= 0) {
                double f = FS / (t - prev);
                lo = f < lo ? f : lo;
                hi = f > hi ? f : hi;
            }
            prev = t;
        }
    return hi - lo;
}

static void t_mod(void)
{
    char what[200];
    {   /* ENV2 -> CUT */
        hy_patch_t p = init_patch();
        double b0, b1;
        p.flt[0] = (hy_flt_t){HF_LP24, 30, 20, 60, 0, 0};
        p.env2[0] = 0;
        p.env2[1] = 80;
        p.env2[2] = 0;
        setup(&p);
        render(48, 127, Y, N1, 0);
        b0 = bright(Y, 512, 4096);
        b1 = bright(Y, FS / 2, N1);
        snprintf(what, sizeof what, "ENV2 -> CUT: brightness %.3f at the start, %.3f after the decay", b0, b1);
        check(what, b0 > b1 * 1.5);
    }
    {   /* M1 -> CUT (the macro knob) */
        hy_patch_t p = init_patch();
        double b0, b1;
        p.flt[0] = (hy_flt_t){HF_LP12, 40, 10, 0, 0, 0};
        p.mod[0] = (hy_slot_t){HS_M1, HS_NONE, HD_CUT, 60};
        setup(&p);
        trk[0].p[P_E4] = 0;
        render(48, 127, Y, N1 / 2, 0);
        b0 = bright(Y, FS / 8, N1 / 2);
        setup(&p);
        trk[0].p[P_E4] = 127;
        render(48, 127, Y, N1 / 2, 0);
        b1 = bright(Y, FS / 8, N1 / 2);
        snprintf(what, sizeof what, "M1 -> CUT: brightness %.3f at M1 0, %.3f at M1 127", b0, b1);
        check(what, b1 > b0 * 2);
    }
    {   /* LFO2 -> PITCH via MW: a vibrato with the wheel up, none with it down */
        hy_patch_t p = init_patch();
        double s0, s1;
        p.osc[0].type = HO_SINE;
        p.osc[0].shape = 0;
        p.lfo[0] = (hy_lfo_t){HW_SINE, 87, 0, 0};
        p.mod[0] = (hy_slot_t){HS_LFO2, HS_MW, HD_PITCH, 8};
        setup(&p);
        part_ctl[0].mw = 0;
        render(69, 127, Y, N1, 0);
        s0 = fspread(Y, FS / 4, N1);
        setup(&p);
        part_ctl[0].mw = 127;
        render(69, 127, Y, N1, 0);
        s1 = fspread(Y, FS / 4, N1);
        /* amount 8 = 8 x 8 x 8 / 32768 x 24 semitones ~ +-0.375 semitone: 440 Hz +-9.6 Hz */
        snprintf(what, sizeof what, "LFO2 -> PITCH via MW: spread %.2f Hz with the wheel down, %.1f Hz up (~19)", s0, s1);
        check(what, s0 < 1.0 && s1 > 14 && s1 < 24);
    }
    {   /* a synced LFO: 1/4 note at 120 BPM = 2 Hz (rate 55) as a square on the level */
        hy_patch_t p = init_patch();
        uint32_t i, flips = 0;
        int last = -1;
        p.lfo[0] = (hy_lfo_t){HW_SQR, 55, HL_SYNC | HL_KEY, 0};
        p.mod[0] = (hy_slot_t){HS_LFO2, HS_NONE, HD_AMP, -63};
        setup(&p);
        song.g[G_BPM] = 120;
        render(60, 127, Y, 2 * N1, 0);
        for (i = 0; i + 256u < 2u * N1; i += 256u) {
            int on = rms(Y, i, i + 256u) > 1500;
            if (last >= 0 && on != last)
                flips++;
            last = on;
        }
        snprintf(what, sizeof what, "LFO2 synced (rate 55 = a quarter note at 120 BPM): %u level flips in 2 s (want 8)", flips);
        check(what, flips >= 7 && flips <= 9);
    }
    {   /* RAND: another value each note */
        hy_patch_t p = init_patch();
        double f[6];
        uint32_t i, j, diff = 0;
        p.mod[0] = (hy_slot_t){HS_RAND, HS_NONE, HD_PITCH, 30};
        for (i = 0; i < 6u; i++) {
            setup(&p);
            run_blocks(1);
            render(57, 127, Y, N1 / 2, 0);
            f[i] = freq(Y, FS / 8, N1 / 2);
        }
        for (i = 0; i < 6u; i++)                    /* pairs of notes at another pitch (of 15) */
            for (j = i + 1u; j < 6u; j++)
                diff += fabs(f[i] - f[j]) > 0.5;
        snprintf(what, sizeof what, "RAND -> PITCH: %.1f %.1f %.1f %.1f %.1f %.1f Hz (another pitch each note)", f[0], f[1],
                 f[2], f[3], f[4], f[5]);
        check(what, diff >= 12);
    }
}

/* ---------------------------------------------------------- controllers --- */
static void t_ctl(void)
{
    char what[200];
    {   /* the bend: +max = +2 semitones on ANALOG */
        double f0, f1;
        memset(trk, 0, sizeof trk);
        memset(part_ctl, 0, sizeof part_ctl);
        host_tracks_init();
        host_preset(&trk[0], 0, 21);                   /* ANALOG SINE PAD */
        trk[0].p[P_ATK] = 0;
        trk[0].p[P_LD_PIT] = 0;
        trk[0].p[P_E0] = 3;                            /* (a sine) */
        trk[0].p[P_E1] = 0;
        trk[0].p[P_E3] = 0;
        render(57, 127, Y, N1 / 2, 0);
        f0 = freq(Y, FS / 8, N1 / 2);
        part_ctl_event(&trk[0], 0xE0u, 127, 127);
        run_blocks(64);
        {
            static int32_t b[CTL];
            uint32_t i, k;
            for (i = 0; i < N1 / 2; i += CTL) {
                track_render(&trk[0], b, CTL);
                for (k = 0; k < CTL; k++)
                    Y[i + k] = b[k];
            }
        }
        f1 = freq(Y, FS / 8, N1 / 2);
        snprintf(what, sizeof what, "bend up full on ANALOG: x%.4f (2 semitones: %.4f)", f1 / f0, pow(2, 2 / 12.0));
        check(what, fabs(f1 / f0 - pow(2, 2 / 12.0)) < 0.003);
    }
    {   /* on HYBRID: the patch's range (12) */
        hy_patch_t p = init_patch();
        double f0, f1;
        p.bend = 12;
        setup(&p);
        render(57, 127, Y, N1 / 2, 0);
        f0 = freq(Y, FS / 8, N1 / 2);
        part_ctl_event(&trk[0], 0xE0u, 0, 0);          /* down full: -12 */
        setup(&p);
        part_ctl_event(&trk[0], 0xE0u, 0, 0);
        render(57, 127, Y, N1 / 2, 0);
        f1 = freq(Y, FS / 8, N1 / 2);
        snprintf(what, sizeof what, "bend down full on HYBRID, range 12: x%.4f (0.5)", f1 / f0);
        check(what, fabs(f1 / f0 - 0.5) < 0.002);
    }
    {   /* the sustain pedal */
        hy_patch_t p = init_patch();
        uint32_t held, after;
        setup(&p);
        trk[0].p[P_REL] = 20;
        trk_note_on(&trk[0], 60, 100);
        trk_note_on(&trk[0], 64, 100);
        run_blocks(10);
        part_ctl_cc(&trk[0], 64, 127);
        trk_note_off(&trk[0], 60);
        trk_note_off(&trk[0], 64);
        run_blocks(FS / CTL);                           /* a second, pedal down */
        held = voices_on();
        part_ctl_cc(&trk[0], 64, 0);
        run_blocks(FS / CTL);
        after = voices_on();
        snprintf(what, sizeof what, "sustain pedal: %u voices held a second after the keys, %u after it comes up", held, after);
        check(what, held == 2 && after == 0);
    }
    {   /* a key pressed again under the pedal, then the pedal up while it is down: it keeps sounding */
        hy_patch_t p = init_patch();
        setup(&p);
        part_ctl_cc(&trk[0], 64, 127);
        trk_note_on(&trk[0], 60, 100);
        trk_note_off(&trk[0], 60);
        trk_note_on(&trk[0], 60, 100);
        part_ctl_cc(&trk[0], 64, 0);
        run_blocks(FS / CTL);
        check("sustain: a key still down when the pedal comes up keeps its note", voices_on() >= 1);
        trk_note_off(&trk[0], 60);
        run_blocks(2 * FS / CTL);
        check("sustain: ... and lets it go at its own note-off", voices_on() == 0);
    }
    {   /* channel pressure -> CUT */
        hy_patch_t p = init_patch();
        double b0, b1;
        p.flt[0] = (hy_flt_t){HF_LP12, 40, 10, 0, 0, 0};
        p.mod[0] = (hy_slot_t){HS_AT, HS_NONE, HD_CUT, 60};
        setup(&p);
        render(48, 127, Y, N1 / 2, 0);
        b0 = bright(Y, FS / 8, N1 / 2);
        setup(&p);
        part_ctl_event(&trk[0], 0xD0u, 127, 0);
        render(48, 127, Y, N1 / 2, 0);
        b1 = bright(Y, FS / 8, N1 / 2);
        snprintf(what, sizeof what, "channel pressure -> CUT: brightness %.3f without, %.3f at full pressure", b0, b1);
        check(what, b1 > b0 * 2);
    }
    {   /* CC 16..19 are the macros on HYBRID, CC 1 the wheel */
        hy_patch_t p = init_patch();
        setup(&p);
        midi_cc(&trk[0], 16, 100);
        midi_cc(&trk[0], 19, 7);
        midi_cc(&trk[0], 1, 90);
        check("CC 16 -> M1, CC 19 -> M4, CC 1 -> mod wheel", trk[0].p[P_E4] == 100 && trk[0].p[P_E7] == 7 && part_ctl[0].mw == 90);
    }
}

/* -------------------------------------------------------------- patches --- */
static uint32_t rs = 12345u;
static uint32_t rnd32(void)
{
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static void t_patches(void)
{
    char what[200];
    {   /* the editor's patch: from the next block; PTCH turned: the factory's */
        hy_patch_t p = init_patch();
        p.osc[0].type = HO_SINE;
        p.level = 77;
        setup(0);
        trk[0].p[P_E3] = 0;
        run_blocks(1);
        check("PTCH 0 loads SYNTHMAVEN", !memcmp(&hy_cur[0], &HY_FACTORY[0], sizeof(hy_patch_t)));
        hy_set_patch(0, &p);
        run_blocks(1);
        check("the editor's patch is played from the next block", hy_cur[0].level == 77 && hy_cur[0].osc[0].type == HO_SINE);
        trk[0].p[P_E3] = 5;
        run_blocks(1);
        check("PTCH turned: the factory patch again", !memcmp(&hy_cur[0], &HY_FACTORY[5], sizeof(hy_patch_t)));
    }
    {   /* random patches: sanitised, bounded, finite, voices free */
        uint32_t k, i, fails = 0;
        double worst = 0;
        for (k = 0; k < 300u; k++) {
            hy_patch_t p;
            uint8_t *b = (uint8_t *)&p;
            for (i = 0; i < sizeof p; i++)
                b[i] = (uint8_t)rnd32();
            setup(&p);
            trk[0].p[P_E4] = (int16_t)(rnd32() & 127u);
            trk[0].p[P_E0] = (int16_t)((int32_t)(rnd32() & 127u) - 64);
            trk[0].p[P_E1] = (int16_t)((int32_t)(rnd32() & 127u) - 64);
            part_ctl[0].mw = (uint8_t)(rnd32() & 127u);
            part_ctl[0].bend = (int16_t)rnd32();
            trk[0].p[P_REL] = 30;
            {
                static int32_t bb[CTL];
                uint32_t nb = (FS / 4) / CTL, j, kk;
                double pk = 0;
                trk_note_on(&trk[0], 24 + rnd32() % 80u, 1 + rnd32() % 127u);
                trk_note_on(&trk[0], 24 + rnd32() % 80u, 1 + rnd32() % 127u);
                for (j = 0; j < nb; j++) {
                    track_render(&trk[0], bb, CTL);
                    for (kk = 0; kk < CTL; kk++)
                        pk = fabs((double)bb[kk]) > pk ? fabs((double)bb[kk]) : pk;
                }
                trk_all_off(&trk[0]);
                run_blocks(12 * FS / CTL);
                if (pk > HY_VOICE_MAX * 2 || voices_on())    /* (no wrap-around anywhere; the voices end) */
                    fails++;
                worst = pk > worst ? pk : worst;
            }
        }
        snprintf(what, sizeof what, "300 random patches: no wrap-around (worst peak %.0f of %.0f, 2 voices), voices free after", worst,
                 HY_VOICE_MAX * 2);
        check(what, !fails);
    }
    {   /* UNISON on SYNTHMAVEN */
        setup(0);
        trk[0].p[P_E3] = 0;
        trk[0].p[P_VOICE] = V_UNISON;
        trk[0].p[P_DETUNE] = 40;
        run_blocks(1);
        render(48, 127, Y, N1 / 2, N1 / 4);
        snprintf(what, sizeof what, "UNISON on SYNTHMAVEN (8 voices on a note): no wrap-around (peak %.0f)", peak(Y, 0, N1 / 2));
        check(what, peak(Y, 0, N1 / 2) < 2 * HY_VOICE_MAX && finite_all(Y, N1 / 2));
    }
}

/* ----------------------------------------------------------------- cost --- */
/* the counted part (under callgrind: --toggle-collect=hy_counted*); hostsim.c empties __attribute__ */
#pragma GCC push_options
#pragma GCC optimize("no-inline")
static void hy_counted(uint32_t nb)
{
    static int32_t o[CTL];
    uint32_t k;
    for (k = 0; k < nb; k++)
        track_render(&trk[0], o, CTL);
}
#pragma GCC pop_options

/* one cost job: "P<n>" factory patch n, "O<t>" osc type t alone, "F<t>" a saw through filter t, "-" idle */
static void cost_job(const char *job, uint32_t voices)
{
    static const uint8_t NOTES[8] = {48, 52, 55, 59, 60, 64, 67, 71};
    hy_patch_t p = init_patch();
    uint32_t i;
    if (job[0] == 'P') {
        p = HY_FACTORY[atoi(job + 1) % HY_NFACTORY];
    } else if (job[0] == 'O') {
        p.osc[0].type = (uint8_t)atoi(job + 1);
        p.osc[0].shape = 64;
    } else if (job[0] == 'F') {
        p.flt[0] = (hy_flt_t){(uint8_t)atoi(job + 1), 70, 40, 0, 0, 0};
    } else if (job[0] == 'X') {                       /* the heaviest: three SUPERs, LP24 + LP24, every slot */
        for (i = 0; i < 3u; i++)
            p.osc[i] = (hy_osc_t){HO_SUPER, (int8_t)(i * 7), 0, 90, 80, 0};
        p.flt[0] = (hy_flt_t){HF_LP24, 70, 40, 20, 20, 20};
        p.flt[1] = (hy_flt_t){HF_LP24, 90, 40, 20, 20, 20};
        p.drive = 40;
        p.ring = 50;
        p.drift = 50;
        for (i = 0; i < HY_NSLOT; i++)
            p.mod[i] = (hy_slot_t){(uint8_t)(1 + i % (HS_N - 1)), HS_MW, (uint8_t)(1 + i % (HD_N - 1)), 5};
    }
    setup(&p);
    trk[0].p[P_SUS] = 127;
    trk[0].p[P_E3] = 0;
    hy_slot_seen[0] = 1;                             /* (the patch above, not PTCH 0) */
    for (i = 0; i < voices && job[0] != '-'; i++)
        trk_note_on(&trk[0], NOTES[i], 100);
    run_blocks(FS / 2 / CTL);
    hy_counted(FS / CTL);
}

static double cost_callgrind(const char *self, const char *job, uint32_t voices)
{
    char cmd[1024], line[512];
    double got = -1;
    FILE *f;
    snprintf(cmd, sizeof cmd,
             "HY_JOB='%s' HY_VOICES=%u valgrind --tool=callgrind --collect-atstart=no '--toggle-collect=hy_counted*' "
             "--callgrind-out-file=/dev/null '%s' 2>&1 >/dev/null", job, voices, self);
    if (!(f = popen(cmd, "r")))
        return -1;
    while (fgets(line, sizeof line, f)) {
        const char *c = strstr(line, "Collected :");
        if (c)
            got = atof(c + 11);
    }
    pclose(f);
    return got < 0 ? -1 : got / ((double)(FS / CTL) * CTL);
}

static void t_cost(const char *self)
{
    double idle, c, worst = 0;
    uint32_t i;
    char job[16], what[200];
    if (system("valgrind --version >/dev/null 2>&1")) {
        printf("hybrid: cost: no valgrind on this host; not measured\n");
        return;
    }
    idle = cost_callgrind(self, "-", 0);
    printf("hybrid: cost, host instructions per sample and voice (8 voices; device %% at %.1f %% per 100):\n", DEV_PER_100);
    for (i = HO_SAW; i < HO_N; i++) {
        snprintf(job, sizeof job, "O%u", i);
        c = (cost_callgrind(self, job, 8) - idle) / 8;
        printf("hybrid:   osc %-6s (one osc, no filter) %5.0f  ~%.2f %%\n", N_HY_OSC[i], c, c * DEV_PER_100 / 100);
    }
    for (i = HF_LP12; i < HF_N; i++) {
        snprintf(job, sizeof job, "F%u", i);
        c = (cost_callgrind(self, job, 8) - idle) / 8;
        printf("hybrid:   saw + %-6s               %5.0f  ~%.2f %%\n", N_HY_FLT[i], c, c * DEV_PER_100 / 100);
    }
    for (i = 0; i < HY_NFACTORY; i++) {
        snprintf(job, sizeof job, "P%u", i);
        c = (cost_callgrind(self, job, 8) - idle) / 8;
        printf("hybrid:   patch %-10s              %5.0f  ~%.2f %%  (8 voices ~%.0f %%)\n", N_HY_PATCH[i], c,
               c * DEV_PER_100 / 100, c * DEV_PER_100 * 8 / 100);
        worst = c > worst ? c : worst;
    }
    c = (cost_callgrind(self, "X", 8) - idle) / 8;
    printf("hybrid:   the heaviest possible (3 SUPER, 2 x LP24, 16 slots) %5.0f  ~%.2f %%\n", c, c * DEV_PER_100 / 100);
    snprintf(what, sizeof what, "cost: factory patches at most %.0f instructions a sample and voice (~%.1f %%; limit %.0f)",
             worst, worst * DEV_PER_100 / 100, HY_COST_MAX);
    check(what, worst <= HY_COST_MAX);
}

/* ---------------------------------------------------------------- demos --- */
static void demos(const char *dir)
{
    static const uint8_t PH[][3] = {   /* note, start (1/8 s), length (1/8 s) */
        {48, 0, 6}, {55, 0, 6}, {60, 0, 6}, {64, 0, 6}, {50, 8, 6}, {57, 8, 6}, {62, 8, 6}, {65, 8, 6},
        {72, 16, 2}, {74, 18, 2}, {76, 20, 2}, {79, 22, 4}, {36, 28, 3}, {36, 32, 2}, {48, 34, 2}, {43, 36, 6},
    };
    uint32_t pi, b, i, total = (FS * 6) / CTL;
    char path[512];
    for (pi = 0; pi < HY_NFACTORY; pi++) {
        static int32_t o[2 * CTL];
        FILE *f;
        uint32_t k;
        memset(trk, 0, sizeof trk);
        memset(eng_arena, 0, sizeof eng_arena);
        memset(eng_arena_own, 0, sizeof eng_arena_own);
        memset(part_ctl, 0, sizeof part_ctl);
        memset(hy_slot_seen, 0, sizeof hy_slot_seen);
        host_tracks_init();
        host_preset(&trk[0], ENGI_HYBRID, pi);
        snprintf(path, sizeof path, "%s/%02u_%s.wav", dir, pi, N_HY_PATCH[pi]);
        for (k = 0; path[k]; k++)
            if (path[k] == ' ')
                path[k] = '_';
        if (!(f = fopen(path, "wb")))
            return;
        wav_hdr(f, total * CTL);
        for (b = 0; b < total; b++) {
            for (i = 0; i < NELEM(PH); i++) {
                if (b == (uint32_t)PH[i][1] * (FS / 8) / CTL)
                    trk_note_on(&trk[0], PH[i][0], 100);
                if (b == (uint32_t)(PH[i][1] + PH[i][2]) * (FS / 8) / CTL)
                    trk_note_off(&trk[0], PH[i][0]);
            }
            mix_block(o, CTL);
            for (k = 0; k < CTL; k++)
                wav_put(f, o[2 * k], o[2 * k + 1]);
        }
        fclose(f);
    }
    printf("hybrid: demos: %u patches into %s\n", HY_NFACTORY, dir);
}

int main(int argc, char **argv)
{
    if (getenv("HY_JOB")) {                          /* a cost job (cost_callgrind's child) */
        cost_job(getenv("HY_JOB"), (uint32_t)atoi(getenv("HY_VOICES") ? getenv("HY_VOICES") : "8"));
        return 0;
    }
    t_osc();
    t_filters();
    t_mod();
    t_ctl();
    t_patches();
    if (!getenv("SKIP_COST"))
        t_cost(argv[0]);
    if (argc > 1)
        demos(argv[1]);
    printf("hybrid: %s\n", bad ? "FAILED" : "all passed");
    return bad != 0;
}
