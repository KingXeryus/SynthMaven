/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Shared DSP building blocks for the Felucca engines (all fixed point).
 * Voice output convention: add sample * amp to out[], where a full-scale
 * oscillator at amp = 1.0 (Q15 32767) contributes VOICE_FS. */
#define VOICE_FS 24000           /* per-voice level: one voice peaks near -6 dBFS before the master */

static inline int32_t mulq15(int32_t a, int32_t b) { return (a * b) >> 15; }
/* a * k / 65536 without 64-bit: a up to 2^31, k Q16 */
static inline int32_t mulq16(int32_t a, uint32_t k)
{
    return (int32_t)(((a >> 16) * (int32_t)k) + (int32_t)(((uint32_t)(a & 0xFFFF) * k) >> 16));
}
static inline int32_t clamp(int32_t v, int32_t lo, int32_t hi) { return v < lo ? lo : v > hi ? hi : v; }

/* sine, linearly interpolated between the 1024 table points (plain lookup: THD -55 dB) */
static inline int32_t sine_i(uint32_t ph)
{
    uint32_t i = ph >> 22;
    int32_t a = SINE[i], b = SINE[(i + 1u) & 1023u];
    return a + (((b - a) * (int32_t)((ph >> 7) & 0x7FFFu)) >> 15);
}
static inline int32_t osc_sine(uint32_t ph) { return sine_i(ph); }

/* polyBLEP residual (Q15) around a wrap of a phase accumulator */
static inline int32_t blep(uint32_t ph, uint32_t inc)
{
    uint32_t d = inc >> 15;
    int32_t x;
    if (!d)
        return 0;
    if (ph < inc) {
        x = (int32_t)(ph / d);                       /* 0..32767 */
        return x + x - ((x * x) >> 15) - 32768;
    }
    if (ph > 0xFFFFFFFFu - inc) {
        x = -(int32_t)((0xFFFFFFFFu - ph) / d);       /* -32767..0 */
        return ((x * x) >> 15) + x + x + 32768;
    }
    return 0;
}

static inline int32_t osc_saw(uint32_t ph, uint32_t inc)
{
    return ((int32_t)(ph >> 16) - 32768) - blep(ph, inc);
}

static inline int32_t osc_pulse(uint32_t ph, uint32_t inc, uint32_t pw)
{
    return (osc_saw(ph, inc) - osc_saw(ph + pw, inc)) >> 1;
}

static inline int32_t osc_tri(uint32_t ph)
{
    int32_t s = (int32_t)(ph >> 15);                 /* 0..131071 */
    return s < 65536 ? s - 32768 : 98303 - s;
}

/* Clipper 0.2: a band-limited triangle. Its corners (the slope turning at phase 0 and 1/2) get the polyBLAMP
 * residual (the integral of polyBLEP's: (1 - x)^3 / 6 on the two samples around a corner, x their distance from
 * it), scaled by the change of slope, 8 x 32768 x inc / 2^32 a sample. The naive one folds its partials back at
 * -12 dB an octave, audible from the 4th octave up; this one is as clean as the polyBLEP saw. */
static inline int32_t blamp_res(uint32_t dist, uint32_t d)       /* (1 - x)^3 / 6 Q15, x = dist / (d << 15) */
{
    int32_t c = 32768 - (int32_t)(dist / d);                     /* 1 - x, Q15 */
    c = (((c * c) >> 15) * c) >> 15;
    return (c * 5461) >> 15;                                     /* / 6 */
}
static inline int32_t osc_tri_bl(uint32_t ph, uint32_t inc)
{
    int32_t s = osc_tri(ph), D = (int32_t)(inc >> 14);           /* the change of slope at a corner */
    uint32_t d = inc >> 15, q;
    if (!d)
        return s;
    q = ph;                                                      /* the bottom corner (slope up: + D) */
    if (q < inc)
        s += (D * blamp_res(q, d)) >> 15;
    else if (~q < inc)
        s += (D * blamp_res(~q, d)) >> 15;
    q = ph - 0x80000000u;                                        /* the top corner (slope down: - D) */
    if (q < inc)
        s -= (D * blamp_res(q, d)) >> 15;
    else if (~q < inc)
        s -= (D * blamp_res(~q, d)) >> 15;
    return s;
}

/* tanh(x / 32768) * 32767, linearly interpolated (a plain lookup gives
 * 256-unit steps, audible on quiet signals) */
static inline int32_t softclip(int32_t x)
{
    int32_t a = x < 0 ? -x : x, i = a >> 8, y;
    if (i >= 256)
        y = TANH_Q15[256];                           /* continuous with the table */
    else
        y = TANH_Q15[i] + (((TANH_Q15[i + 1] - TANH_Q15[i]) * (a & 255)) >> 8);
    return x < 0 ? -y : y;
}

static inline uint32_t noise32(int32_t *st)
{
    uint32_t s = (uint32_t)*st;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    *st = (int32_t)s;
    return s;
}

/* Trapezoidal SVF (A. Simper), unconditionally stable. Coefficients per
 * block in Q13; signals stay within +-150000 so products fit in 32 bits. */
typedef struct { int32_t a1, a2, a3; } tsvf_t;
static inline void tsvf_coef(tsvf_t *c, int32_t cut, int32_t reso)   /* cut: 0..127 << 8 */
{
    int32_t i, g, den, k = 8192 - reso * 7600 / 127;   /* damping 2.0 .. ~0.15 (Q12) */
    cut = clamp(cut, 0, 127 << 8);
    i = cut >> 8;
    g = SVF_G[i];
    if (i < 127)                                       /* between table points: sweeps without 128 steps */
        g += ((SVF_G[i + 1] - g) * (cut & 255)) >> 8;
    den = 4096 + ((g * (g + k)) >> 12);
    c->a1 = (int32_t)((4096u << 13) / (uint32_t)den);
    c->a2 = (c->a1 * g) >> 12;
    c->a3 = (c->a2 * g) >> 12;
}

static inline int32_t tsvf_lp(const tsvf_t *c, int32_t in, int32_t *ic1, int32_t *ic2)
{
    int32_t v3 = in - *ic2;
    int32_t v1 = (c->a1 * *ic1 + c->a2 * v3) >> 13;
    int32_t v2 = *ic2 + ((c->a2 * *ic1 + c->a3 * v3) >> 13);
    *ic1 = clamp(2 * v1 - *ic1, -150000, 150000);
    *ic2 = clamp(2 * v2 - *ic2, -150000, 150000);
    return v2;
}

/* Clipper: a four-pole transistor ladder (the Moog type), after V. Zavalishin's TPT ladder ("The Art of VA
 * Filter Design", ch. 5): four one-pole trapezoidal low-passes, the feedback solved in the same sample (zero-delay
 * feedback: the cutoff and the resonance stay right up to the top), and a tanh at the ladder's input (its
 * differential pair): a hot signal drives it warm, the resonance saturates softly, and from k = 4 it sings on its
 * own. The passband is partly kept at high resonance (the input x (1 + k / 2)), as on the later Moogs.
 * Inside: signals Q12 (no dead band even at 30 Hz, where G is 1/500), coefficients Q16, products and sums in
 * 64 bits (as FM6, PHYS), rounded; G <= 0.75 (g <= 3, 17.6 kHz): the states stay within 3 times the tanh's
 * ceiling (65536), 8e8 in Q12. */
typedef struct { int32_t G, H, G4, inv, k; } ladder_t;   /* G, 1 - G, G^4, 1 / (1 + k G^4): Q16; k: Q12 */
static inline int32_t mul_sh(int32_t a, int32_t b, int sh) { return (int32_t)(((int64_t)a * b) >> sh); }
static inline void ladder_coef(ladder_t *c, int32_t cut, int32_t res)   /* cut 0..127 << 8, res 0..127 */
{
    int32_t i, g, G, G2;
    cut = clamp(cut, 0, 127 << 8);
    i = cut >> 8;
    g = SVF_G[i];
    if (i < 127)
        g += ((SVF_G[i + 1] - g) * (cut & 255)) >> 8;
    g = clamp(g, 1, 3 * 4096);
    G = (int32_t)(((uint32_t)g << 16) / (uint32_t)(4096 + g));   /* g / (1 + g) */
    G2 = mul_sh(G, G, 16);
    c->G = G;
    c->H = 65536 - G;
    c->G4 = mul_sh(G2, G2, 16);
    c->k = clamp(res, 0, 127) * 16900 / 127;                     /* 0 .. 4.13 */
    c->inv = (int32_t)(((uint64_t)1 << 32) / (uint32_t)(65536 + mul_sh(c->k, c->G4, 12)));
}
/* one sample: x (clamped to +-131072); s[4] the stages (Q12) */
static inline int32_t ladder_tick(const ladder_t *c, int32_t x, int32_t *s)
{
    int64_t in = (int64_t)clamp(x, -131072, 131072) << 12, S, y, u;
    int32_t G = c->G, uq, v, j;
    in += (in * c->k) >> 13;                                     /* x (1 + k / 2) */
    S = ((int64_t)G * s[0] >> 16) + s[1];
    S = ((G * S) >> 16) + s[2];
    S = ((G * S) >> 16) + s[3];
    S = (c->H * S) >> 16;
    y = ((((int64_t)c->G4 * in) >> 16) + S) * c->inv >> 16;    /* the ladder's output, solved */
    u = in - ((c->k * y) >> 12);                                 /* the ladder's input (Q12) */
    {                                                            /* tanh at 65536: on u / 2 (softclip's range), the
                                                                  * bits below a unit added back (its slope ~1 there) */
        int64_t h = u >> 13;
        if (h <= -65536 || h >= 65536)
            uq = softclip(h < 0 ? -65536 : 65536) << 13;
        else
            uq = (softclip((int32_t)h) << 13) + (int32_t)(u - (h << 13));
    }
    for (j = 0; j < 4; j++) {
        v = (int32_t)(((int64_t)G * (uq - s[j]) + 32768) >> 16);
        uq = v + s[j];
        s[j] = uq + v;
    }
    return (uq + 2048) >> 12;
}

/* amplitude ramp over the block. Blocks are always CTL long, so x / CTL is a
 * shift rounded towards zero (-Os would keep a hardware divide per sample) */
#define CTL_LOG2 5
#if (1 << CTL_LOG2) != CTL
#error "CTL_LOG2 does not match CTL"
#endif
static inline int32_t amp_at(const vmod_t *m, uint32_t i)
{
    int32_t x = (m->amp1 - m->amp0) * (int32_t)i;
    return m->amp0 + ((x + ((x >> 31) & (CTL - 1))) >> CTL_LOG2);
}
