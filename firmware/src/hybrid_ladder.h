/* SPDX-License-Identifier: GPL-3.0-only
 * Portions Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * Extracted from SynthMaven firmware/src/dsp.c, donor e49239e.
 * Retains the donor file notice and SynthMaven algorithm provenance below. */
/* SynthMaven: a four-pole transistor ladder (the Moog type), after V. Zavalishin's TPT ladder ("The Art of VA
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
    c->inv = (int32_t)(0xFFFFFFFFu / (uint32_t)(65536 + mul_sh(c->k, c->G4, 12)));   /* (32 bits: the FM-1 has
                                                                                      * no 64-bit divide) */
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

/* SynthMaven 0.2: a band-limited triangle. Its corners (the slope turning at phase 0 and 1/2) get the polyBLAMP
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

