/* SPDX-License-Identifier: GPL-3.0-only
 * DSP helpers Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
static inline int32_t smc_blep(uint32_t ph, uint32_t inc)
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

static inline int32_t smc_osc_saw(uint32_t ph, uint32_t inc)
{
    return ((int32_t)(ph >> 16) - 32768) - smc_blep(ph, inc);
}

static inline int32_t smc_osc_pulse(uint32_t ph, uint32_t inc, uint32_t pw)
{
    return (smc_osc_saw(ph, inc) - smc_osc_saw(ph + pw, inc)) >> 1;
}
static inline void smc_tsvf_coef(tsvf_t *c, int32_t cut, int32_t reso)   /* cut: 0..127 << 8 */
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

/* SPDX-License-Identifier: GPL-3.0-only */
/* SynthMaven classic engine adapters. Original notices retained below.
 * Stored engine IDs append after HYBRID; retired Melodee IDs stay reserved. */
enum { FM_SVF=1, FM_MOOG, FM_MS20, FM_DIODE, FM_SEM, FM_NOTCH, FM_BP, FM_HP, FM_LAST=FM_HP };
#define SM_CLASSIC_FILTER P_FM4_LEVEL
typedef struct { int32_t G, H, G1, H1, PG, inv, k; } zdf_t;   /* Q16; k Q12 */
typedef struct {
    uint8_t model;
    tsvf_t sv;
    int32_t k;                                         /* the SVF's damping (Q12) */
    ladder_t lad;
    zdf_t z;
} fmod_t;
static inline int64_t clamp64(int64_t x, int64_t lo, int64_t hi) { return x < lo ? lo : x > hi ? hi : x; }
static inline int32_t zdf_sat(int64_t u)               /* the ladder's tanh, at 65536 (Q12) */
{
    int64_t h = u >> 13;
    if (h <= -65536 || h >= 65536)
        return softclip(h < 0 ? -65536 : 65536) << 13;
    return (softclip((int32_t)h) << 13) + (int32_t)(u - (h << 13));
}
static inline int32_t fmod_g(int32_t cut)              /* the prewarped gain g (Q12), as the ladder's */
{
    int32_t i, g;
    cut = clamp(cut, 0, 127 << 8);
    i = cut >> 8;
    g = SVF_G[i];
    if (i < 127)
        g += ((SVF_G[i + 1] - g) * (cut & 255)) >> 8;
    return clamp(g, 1, 3 * 4096);
}
static inline void fmod_coef(fmod_t *f, int32_t model, int32_t cut, int32_t res)   /* cut 0..127 << 8, res 0..127 */
{
    f->model = (uint8_t)model;
    res = clamp(res, 0, 127);
    if (model == FM_MOOG) {
        ladder_coef(&f->lad, cut, res);
    } else if (model == FM_MS20 || model == FM_DIODE) {
        zdf_t *z = &f->z;
        int32_t g = fmod_g(cut), G, G2;
        G = (int32_t)(((uint32_t)g << 16) / (uint32_t)(4096 + g));
        z->G = G, z->H = 65536 - G;
        G2 = mul_sh(G, G, 16);
        if (model == FM_MS20) {
            z->k = res * 9000 / 127;                   /* 0 .. 2.2: it sings from 2 */
            z->PG = G2;
        } else {
            int32_t g1 = g >> 1;
            z->G1 = (int32_t)(((uint32_t)g1 << 16) / (uint32_t)(4096 + g1));
            z->H1 = 65536 - z->G1;
            z->k = res * 19000 / 127;                  /* 0 .. 4.6 */
            z->PG = mul_sh(mul_sh(z->G1, G2, 16), G, 16);
        }
        z->inv = (int32_t)(0xFFFFFFFFu / (uint32_t)(65536 + mul_sh(z->k, z->PG, 12)));
    } else {
        int32_t r = model == FM_SEM ? res * 100 / 127 : res;   /* (the SEM: short of oscillating) */
        smc_tsvf_coef(&f->sv, cut, r);
        f->k = 8192 - r * 7600 / 127;
    }
}
static inline int32_t fmod_tick(const fmod_t *f, int32_t x, int32_t *sv, int32_t *ls)
{
    switch (f->model) {
    case FM_MOOG:
        return ladder_tick(&f->lad, x, ls) >> 1;
    case FM_MS20: {                                    /* two poles, the feedback saturated */
        const zdf_t *z = &f->z;
        int64_t in = (int64_t)clamp(x, -131072, 131072) << 12, S, y;
        int32_t u, v;
        in += (in * z->k) >> 14;                       /* (a little of the passband back) */
        S = ((((int64_t)z->G * ls[0]) >> 16) * z->H >> 16) + ((int64_t)z->H * ls[1] >> 16);
        y = ((((int64_t)z->PG * in) >> 16) + S) * z->inv >> 16;
        u = (int32_t)clamp64(in - (((int64_t)z->k * zdf_sat(y)) >> 12), -(int64_t)400000000, 400000000);
        v = (int32_t)(((int64_t)z->G * (u - ls[0]) + 32768) >> 16);
        u = v + ls[0], ls[0] = u + v;
        v = (int32_t)(((int64_t)z->G * (u - ls[1]) + 32768) >> 16);
        u = v + ls[1], ls[1] = u + v;
        return (u + 4096) >> 13;
    }
    case FM_DIODE: {                                   /* four poles, the first at half the cutoff */
        const zdf_t *z = &f->z;
        int64_t in = (int64_t)clamp(x, -131072, 131072) << 12, S, T, y;
        int32_t u, v, j;
        in += (in * z->k) >> 13;
        T = (((int64_t)z->G * ls[1]) >> 16) + ls[2];
        T = ((z->G * T) >> 16) + ls[3];
        T = (z->H * T) >> 16;
        S = mul_sh(mul_sh(mul_sh(z->G, z->G, 16), z->G, 16), z->H1, 16);
        S = ((S * (int64_t)ls[0]) >> 16) + T;
        y = ((((int64_t)z->PG * in) >> 16) + S) * z->inv >> 16;
        u = zdf_sat(in - ((z->k * y) >> 12));
        v = (int32_t)(((int64_t)z->G1 * (u - ls[0]) + 32768) >> 16);
        u = v + ls[0], ls[0] = u + v;
        for (j = 1; j < 4; j++) {
            v = (int32_t)(((int64_t)z->G * (u - ls[j]) + 32768) >> 16);
            u = v + ls[j], ls[j] = u + v;
        }
        return (u + 4096) >> 13;
    }
    case FM_SVF:
        return tsvf_lp(&f->sv, x >> 1, &sv[0], &sv[1]);
    default: {                                         /* SEM NOTCH BP HP: one SVF, its outputs */
        const tsvf_t *c = &f->sv;
        int32_t in = x >> 1, v1, v2, v3;
        if (f->model == FM_SEM) {                      /* (driven softly) */
            int32_t a = in < 0 ? -in : in;
            if (a > 12000)
                in = in < 0 ? -(12000 + (softclip((a - 12000) * 2) >> 1)) : 12000 + (softclip((a - 12000) * 2) >> 1);
        }
        v3 = in - sv[1];
        v1 = (c->a1 * sv[0] + c->a2 * v3) >> 13;
        v2 = sv[1] + ((c->a2 * sv[0] + c->a3 * v3) >> 13);
        sv[0] = clamp(2 * v1 - sv[0], -150000, 150000);
        sv[1] = clamp(2 * v2 - sv[1], -150000, 150000);
        switch (f->model) {
        case FM_NOTCH: return in - ((f->k * v1) >> 12);
        case FM_BP: return (f->k * v1) >> 12;
        case FM_HP: return in - ((f->k * v1) >> 12) - v2;
        default: return v2;
        }
    }
    }
}


static int32_t sm_classic_model(const track_t *t){return clamp(t->p[SM_CLASSIC_FILTER],FM_SVF,FM_LAST);}
/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* ANALOG: two band-limited oscillators (saw / square / tri / sine / PWM),
 * noise, drive and a trapezoidal low-pass. */
static const char *const smc_N_ANALOG_WAVE[] = {"SAW", "SQR", "TRI", "SIN", "PWM"};

static void smc_analog_note_on(track_t *t, voice_t *v)
{
    (void)t;
    if (!v->env && !v->env_out)
        v->ph[0] = 0;                                 /* a fresh note: from phase 0 (an 808 starts the same every time) */
    v->ph[1] = v->ph[0] + 0x40000000u;
    v->s[0] = v->s[1] = 0;                            /* filter */
    v->s[3] = v->s[4] = v->s[5] = v->s[6] = 0;        /* (MOOG: the ladder's four stages) */
    if (!v->s[2])
        v->s[2] = 0x1234567 + (int32_t)v->age;        /* noise state */
}

static void smc_analog_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    uint32_t wave = (uint32_t)p[P_E0], i;
    int32_t det = p[P_E1], mix = p[P_E2], noise = p[P_E3];
    int32_t cut = (p[P_E4] << 8) + m->cutoff + (p[P_E7] * (v->pitch16 - 60 * 16) >> 4);
    fmod_t fm;                                                   /* SynthMaven: the filter MODEL (P_FMODEL, dsp.c) */
    int32_t model = sm_classic_model(t);
    int32_t drive = 32768 + p[P_E6] * 768;                       /* 1x .. 4x */
    int32_t dw = p[P_E6] * 258;                                  /* DRV: dry -> driven (a clean range low) */
    uint32_t inc1 = m->inc;
    /* DTN in cents: whole 1/16 semitones from the table, the rest as a fine factor */
    int32_t d16 = det * 16 / 100, rem = det * 16 - d16 * 100;            /* rem: 1/1600 semitone */
    uint32_t inc2 = pitch_inc(clamp(m->pitch16 + d16, 0, 2047));
    inc2 += (uint32_t)((int32_t)(inc2 >> 12) * (rem * 2367 / 16000));
    uint32_t pw = 0x80000000u + (uint32_t)((m->shape - (64 << 8)) << 15);
    int32_t m2 = mix * 258, m1 = 32767 - m2;                    /* osc mix Q15 */
    int32_t nz = noise * 200, drv = p[P_E6];
    uint32_t ph0 = v->ph[0], ph1 = v->ph[1];                  /* state in locals: out[] may alias v->s[] */
    int32_t sv[2] = {v->s[0], v->s[1]}, nst = v->s[2];
    int32_t ls[4] = {v->s[3], v->s[4], v->s[5], v->s[6]};      /* (MOOG's stages, in locals too) */
    fmod_coef(&fm, model, cut, p[P_E5]);
    if (det == 0)
        inc2 = inc1;
    for (i = 0; i < n; i++) {
        int32_t a, b = 0, s;
        switch (wave) {
        case 1:
            a = smc_osc_pulse(ph0, inc1, 0x80000000u);
            if (m2)
                b = smc_osc_pulse(ph1, inc2, 0x80000000u);
            break;
        case 2:                                       /* (SynthMaven 0.2: band-limited, dsp.c osc_tri_bl) */
            a = osc_tri_bl(ph0, inc1);
            if (m2)
                b = osc_tri_bl(ph1, inc2);
            break;
        case 3:
            a = osc_sine(ph0);
            if (m2)
                b = osc_sine(ph1);
            break;
        case 4:
            a = smc_osc_pulse(ph0, inc1, pw);
            if (m2)
                b = smc_osc_pulse(ph1, inc2, pw);
            break;
        default:
            a = smc_osc_saw(ph0, inc1);
            if (m2)
                b = smc_osc_saw(ph1, inc2);
            break;
        }
        ph0 += inc1;
        ph1 += inc2;
        s = m2 ? mulq15(a, m1) + mulq15(b, m2) : a;  /* (MIX 0, the 808s and subs: one oscillator) */
        if (nz)
            s += mulq15((int32_t)(noise32(&nst) >> 17) - 16384, nz);
        if (drv)                                      /* pre-shifts: drive is up to 4x, no overflow */
            s += mulq15(softclip(((s >> 2) * (drive >> 2)) >> 11) - s, dw);
        {   /* filter: linear up to half scale, then a soft knee (only resonance peaks saturate) */
            int32_t y = model == FM_SVF ? tsvf_lp(&fm.sv, s >> 1, &sv[0], &sv[1]) : fmod_tick(&fm, s, sv, ls);
            int32_t a = y < 0 ? -y : y;
            if (a > 16000) {
                a = 16000 + (softclip((a - 16000) * 2) >> 1);
                y = y < 0 ? -a : a;
            }
            s = y << 1;
        }
        out[i] += mulq15(mulq15(s, amp_at(m, i)), VOICE_FS) << 1;
    }
    v->ph[0] = ph0;
    v->ph[1] = ph1;
    v->s[0] = sv[0];
    v->s[1] = sv[1];
    v->s[3] = ls[0], v->s[4] = ls[1], v->s[5] = ls[2], v->s[6] = ls[3];
    v->s[2] = nst;
}


static const preset_t SMC_ANALOG_PRESETS[] PRESET_TABLE ={
{"808 BOOM",{3,0,0,0,127,0,48,0},{0,112,0,52},0,1},
{"808 DIRTY",{3,0,0,0,96,12,118,30},{0,108,0,46},0,1},
{"SUB BASS",{3,0,0,0,127,0,10,0},{0,60,118,24},0,1},
{"808 SLIDE",{3,0,0,0,127,0,72,0},{0,118,0,60},0,1},
{"ACID 303",{0,0,0,0,36,112,44,64},{0,56,24,20},52,1},
{"PLUGG BASS",{2,0,0,0,92,0,30,0},{0,96,92,34},0,1},
{"REESE",{0,22,64,0,56,22,36,40},{0,70,118,28},0,1},
{"WOBBLE",{0,9,64,0,38,72,62,30},{0,64,127,22},0,1},
{"FUNK BASS",{1,6,50,0,50,30,22,64},{0,55,62,20},30,1},
{"G-FUNK LD",{2,0,0,0,92,0,10,64},{6,70,112,46},0,1},
{"TRAP PLUCK",{0,8,64,0,52,35,20,64},{0,80,24,45},48,0},
{"SYN BRASS",{0,10,64,0,45,20,16,64},{12,70,96,35},34,0},
{"SUPERSAW",{0,14,64,0,96,12,10,64},{4,70,112,46},10,1},
{"WARM PAD",{4,12,64,6,52,8,0,32},{75,90,115,90},0,0},
{"DARK STR",{0,18,64,0,48,6,0,32},{60,90,118,85},0,0},
{"ATMOS PAD",{4,25,64,20,60,15,0,32},{90,100,120,100},0,0},
{"PAN FLUTE",{2,0,0,30,70,10,0,64},{14,70,110,40},0,1},
{"FAT LEAD",{0,8,64,0,70,30,30,64},{2,70,110,30},30,1},
{"MONO BASS",{0,4,64,0,48,36,30,64},{0,60,70,20},44,1},
{"PWM LEAD",{4,0,0,0,80,20,10,64},{2,70,110,30},0,1},
{"SAW BASS",{0,6,64,0,52,30,40,64},{0,60,60,20},30,1},
{"SINE PAD",{3,10,64,4,90,0,0,32},{80,90,118,90},0,0},
{"MINI BASS",{0,6,64,0,42,60,40,64},{0,64,70,24},46,1},
{"MINI LEAD",{0,9,64,0,62,80,30,64},{2,70,110,40},26,1},
{"JUNO PAD",{4,14,64,4,62,10,0,40},{60,100,118,88},8,0},
{"OBX BRASS",{0,16,64,0,50,20,10,64},{14,74,100,40},34,1},
{"TAURUS",{0,6,64,0,40,40,50,64},{0,80,100,30},30,1},
{"PROPH LEAD",{1,10,64,0,74,40,20,64},{2,70,110,40},20,1},
{"UNI STAB",{0,20,64,0,70,30,10,64},{0,62,20,30},40,1},
{"SQR PLUCK",{1,4,64,0,48,40,0,80},{0,70,0,50},50,0},
{"DYN BASS",{0,6,64,0,40,50,30,64},{0,64,80,24},30,1},
{"WAH LEAD",{0,8,64,0,60,70,20,64},{2,70,110,40},0,1},
};
static const engine_t ENG_SMC_ANALOG={.pool=1,.name="ANALOG",.page_title={"OSC", "FLT"},.edit={
        {"WAVE", F_ENUM, 0, 4, 0, smc_N_ANALOG_WAVE, 0},
        {"DTN", F_INT, 0, 127, 10, 0, "ct"},
        {"MIX", F_PCT, 0, 127, 64, 0, 0},
        {"NOIS", F_PCT, 0, 127, 0, 0, 0},
        {"CUT", F_CUTOFF, 0, 127, 90, 0, 0},
        {"RES", F_PCT, 0, 127, 30, 0, 0},
        {"DRV", F_PCT, 0, 127, 0, 0, 0},
        {"KTR", F_PCT, 0, 127, 64, 0, 0},
    },.presets=SMC_ANALOG_PRESETS,.npresets=32,.knob={P_E4,P_E5,P_ATK,P_REL},.note_on=smc_analog_note_on,.render=smc_analog_render};
/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* DIGITAL: four-operator FM. */
/* Four sine operators, eight classic 4-operator algorithms (ALG = P_E0, EDIT 1 KNOB 1),
 * op 4 with feedback, one modulation INDEX shaped by a modulator envelope.
 * Phase modulation wraps naturally in the 32-bit phase. */
static const char *const smc_N_FMALG[] = {"1", "2", "3", "4", "5", "6", "7", "8"};
static const char *const smc_N_RATIO[] = {".5", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "14", "16"};
static const uint16_t smc_RATIO_Q8[15] = {128, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816,
                                      3072, 3584, 4096};

static void smc_digital_note_on(track_t *t, voice_t *v)
{
    (void)t;
    v->ph[0] = v->ph[1] = v->ph[2] = 0;
    v->s[7] = 0;                 /* op 4 phase */
    v->s[4] = 1 << 24;           /* modulator envelope, Q24 */
    v->s[5] = v->s[6] = 0;       /* feedback history */
    v->s[3] = -1;                /* the index of the block before (-1: none, a new note) */
}

/* operator increment = carrier * ratio (Q8) without 32-bit overflow; kept below Nyquist */
static inline uint32_t smc_fm_ratio_inc(uint32_t inc, uint32_t r)
{
    uint32_t lim = 0x73000000u / r;                      /* (inc >> 8) * r must stay < 0x73000000 */
    return (inc >> 8) > lim ? 0x73000000u : (inc >> 8) * r;
}

static inline uint32_t smc_digital_mod(int32_t x, int32_t idx) { return (uint32_t)(x * idx) * 2065u; }

static void smc_digital_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    uint32_t alg = (uint32_t)p[P_E0] & 7u, i;
    uint32_t i1 = m->inc;
    uint32_t i2 = smc_fm_ratio_inc(m->inc, smc_RATIO_Q8[p[P_E1] % 15]), i3 = smc_fm_ratio_inc(m->inc, smc_RATIO_Q8[p[P_E2] % 15]);
    uint32_t i4 = smc_fm_ratio_inc(m->inc, smc_RATIO_Q8[p[P_E3] % 15]);
    int32_t me, idx, fb = p[P_E6], fb1, fb2, didx, idq;      /* fb1, fb2: op 4 feedback history */
    uint32_t ph0, ph1, ph2, ph4;
    /* modulator envelope: decays (MODDEC) towards 25 %, per block */
    v->s[4] += mulq16((1 << 22) - v->s[4], ENV_EXP[p[P_E5] & 127]);
    me = v->s[4] >> 9;                                         /* Q15 */
    idx = (p[P_E4] * me) >> 15;                                 /* 0..127 */
    idx = clamp(idx + ((m->cutoff + m->shape - (64 << 8)) >> 8) + (v->vel - 96) / 4, 0, 127);
    {   /* (SynthMaven 0.9) anti-aliasing: the index held where the sidebands (Carson: (index + 1) x the modulator)
         * would pass ~16 kHz, from the fastest operator that modulates in this algorithm; a high note gets a
         * little less bright instead of folding back as noise */
        static const uint8_t MODS[8] = {14, 14, 14, 14, 10, 8, 8, 0};   /* bit 1: op 2, 2: op 3, 3: op 4 */
        uint32_t mk = MODS[alg], im = 0, fm;
        if (mk & 2u)
            im = i2;
        if ((mk & 4u) && i3 > im)
            im = i3;
        if ((mk & 8u) && i4 > im)
            im = i4;
        fm = ((im >> 16) * 44100u) >> 16;                      /* Hz */
        if (fm > 0u) {
            int32_t cap = (int32_t)(161760u / fm) - 10;        /* 16 kHz / fm - 1, in index steps (0.0989 rad) */
            idx = idx < cap ? idx : (cap > 0 ? cap : 0);
        }
    }
    {   /* (0.9) the index glides through the block from the last one: a new modulator envelope (a retriggered
         * mono bass) or a fast decay no longer steps the waveform at the block's edge */
        int32_t i0 = v->s[3] < 0 ? idx : v->s[3];
        v->s[3] = idx;
        idx = i0;
        didx = ((v->s[3] - i0) * 2048) / (int32_t)(n ? n : 1u);   /* Q11 per sample */
    }
    ph0 = v->ph[0];                                             /* state in locals: out[] may alias v->s[] */
    ph1 = v->ph[1];
    ph2 = v->ph[2];
    ph4 = (uint32_t)v->s[7];
    fb1 = v->s[5];
    fb2 = v->s[6];
    idq = idx << 11;
    for (i = 0; i < n; i++) {
        int32_t o1, o2, o3, o4, s;
        idx = idq >> 11;
        idq += didx;
        o4 = sine_i((ph4 + smc_digital_mod((fb1 + fb2) >> 1, fb)));
        fb2 = fb1;
        fb1 = o4;
        switch (alg) {
        case 0:
            o3 = sine_i((ph2 + smc_digital_mod(o4, idx)));
            o2 = sine_i((ph1 + smc_digital_mod(o3, idx)));
            s = sine_i((ph0 + smc_digital_mod(o2, idx)));
            break;
        case 1:
            o3 = sine_i(ph2);
            o2 = sine_i((ph1 + smc_digital_mod((o3 + o4) >> 1, idx)));
            s = sine_i((ph0 + smc_digital_mod(o2, idx)));
            break;
        case 2:
            o3 = sine_i(ph2);
            o2 = sine_i((ph1 + smc_digital_mod(o3, idx)));
            s = sine_i((ph0 + smc_digital_mod((o2 + o4) >> 1, idx)));
            break;
        case 3:
            o3 = sine_i((ph2 + smc_digital_mod(o4, idx)));
            o2 = sine_i(ph1);
            s = sine_i((ph0 + smc_digital_mod((o2 + o3) >> 1, idx)));
            break;
        case 4:
            o3 = sine_i((ph2 + smc_digital_mod(o4, idx)));
            o2 = sine_i(ph1);
            o1 = sine_i((ph0 + smc_digital_mod(o2, idx)));
            s = (o1 + o3) >> 1;
            break;
        case 5:
            o3 = sine_i((ph2 + smc_digital_mod(o4, idx)));
            o2 = sine_i((ph1 + smc_digital_mod(o4, idx)));
            o1 = sine_i((ph0 + smc_digital_mod(o4, idx)));
            s = mulq15(o1 + o2 + o3, 10923);           /* / 3 without a divide */
            break;
        case 6:
            o3 = sine_i((ph2 + smc_digital_mod(o4, idx)));
            o2 = sine_i(ph1);
            o1 = sine_i(ph0);
            s = mulq15(o1 + o2 + o3, 10923);           /* / 3 without a divide */
            break;
        default:
            o3 = sine_i(ph2);
            o2 = sine_i(ph1);
            o1 = sine_i(ph0);
            s = (o1 + o2 + o3 + o4) >> 2;
            break;
        }
        ph0 += i1;
        ph1 += i2;
        ph2 += i3;
        ph4 += i4;
        out[i] += mulq15(mulq15(s, amp_at(m, i)), VOICE_FS);   /* full-scale sines: 6 dB below the others */
    }
    v->ph[0] = ph0;
    v->ph[1] = ph1;
    v->ph[2] = ph2;
    v->s[7] = (int32_t)ph4;
    v->s[5] = fb1;
    v->s[6] = fb2;
}


static const preset_t SMC_DIGITAL_PRESETS[] PRESET_TABLE ={
{"RHODES",{4,1,1,14,26,35,0,0},{0,85,35,55},0,0},
{"DX RHODES",{4,1,1,14,85,45,0,0},{0,82,40,55},0,0},
{"WURLI",{4,1,2,3,62,35,8,0},{0,75,30,45},0,0},
{"CLAV",{3,1,3,5,84,25,7,0},{0,58,34,18},0,0},
{"M1 PIANO",{4,1,2,13,74,30,12,0},{0,86,30,48},0,0},
{"AFRO KEYS",{4,1,1,7,40,50,0,0},{0,82,40,60},0,0},
{"TRAP BELL",{4,8,1,4,80,70,0,0},{0,92,0,70},0,0},
{"MUSIC BOX",{4,4,3,9,45,75,0,0},{0,90,0,75},0,0},
{"MARIMBA",{4,4,1,1,60,30,0,0},{0,80,0,60},0,0},
{"KALIMBA",{4,4,1,6,52,24,0,0},{0,78,0,58},0,0},
{"PLUGG BELL",{4,3,1,4,56,60,0,0},{0,90,0,82},0,0},
{"GLASS PAD",{4,2,1,5,34,100,0,0},{62,92,112,92},0,0},
{"FM BASS",{0,1,1,1,34,40,8,0},{0,60,70,25},0,1},
{"STEEL DRUM",{4,2,2,2,45,45,0,0},{0,88,0,66},0,0},
{"GLOCKEN",{4,5,1,8,40,60,0,0},{0,95,0,85},0,0},
{"CELESTA",{4,2,1,4,30,50,0,0},{0,84,0,70},0,0},
{"XYLOPHONE",{4,4,3,3,70,18,0,0},{0,77,0,58},0,0},
{"HARP",{4,1,1,2,45,30,10,0},{0,92,0,80},0,0},
{"BASS GTR",{4,1,1,2,40,30,6,0},{0,76,40,22},0,1},
{"TUBE BELL",{4,6,1,13,50,90,0,0},{0,100,0,90},0,0},
{"FM PLUCK",{4,2,1,3,70,15,0,0},{0,78,0,54},0,0},
{"FM GRAND",{4,1,2,12,60,40,10,0},{0,90,20,60},0,0},
{"DX BASS",{0,1,1,1,46,26,10,0},{0,60,80,20},0,1},
{"SUITCASE",{4,1,1,14,30,38,0,0},{0,85,35,60},0,0},
{"CHURCH BEL",{4,6,1,9,44,95,0,0},{0,104,0,96},0,0},
{"FM BRASS",{0,1,1,1,48,70,6,0},{20,80,110,40},0,0},
{"FM LEAD",{0,1,2,1,40,60,10,0},{2,70,110,40},0,1},
{"TOY PIANO",{4,7,1,11,60,25,0,0},{0,80,0,50},0,0},
{"SYN VIBES",{4,4,1,4,40,40,0,0},{0,92,0,74},0,0},
{"PERC ORGAN",{4,2,1,3,30,20,0,0},{0,64,120,30},0,0},
};
static const engine_t ENG_SMC_DIGITAL={.pool=1,.name="DIGITAL",.page_title={"OPS", "MOD"},.edit={
        {"ALG", F_ENUM, 0, 7, 0, smc_N_FMALG, 0},
        {"R2", F_ENUM, 0, 14, 1, smc_N_RATIO, 0},
        {"R3", F_ENUM, 0, 14, 1, smc_N_RATIO, 0},
        {"R4", F_ENUM, 0, 14, 1, smc_N_RATIO, 0},
        {"IDX", F_PCT, 0, 127, 60, 0, 0},
        {"MDEC", F_TIME, 0, 127, 60, 0, 0},
        {"FB", F_PCT, 0, 127, 0, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
    },.presets=SMC_DIGITAL_PRESETS,.npresets=30,.knob={P_E4,P_E5,P_E6,P_REL},.note_on=smc_digital_note_on,.render=smc_digital_render};
/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* VOICE: formant oscillator, sung vowels from the keyboard. 4 voices.
 * After D. H. Klatt, "Software for a cascade/parallel formant synthesizer",
 * JASA 67 (1980): a glottal source (derivative of a Rosenberg (1971) pulse,
 * no DC) into four two-pole resonators in cascade, F1 -> F2 -> F3 -> F4.
 * Design follows klattsch by Tony Gies (MIT, design reference only, no code
 * copied). Formant data from Klatt (1980) and Hillenbrand et al. (1995).
 *
 *   source  rising half-sine while opening (Tp), falling quarter-sine while
 *           closing (Tn), 0 while closed; the closure step goes through smc_blep()
 *   BUZZ    open quotient 0.9 .. 0.35 and a one-pole spectral tilt
 *   BREATH  noise, louder while the glottis is open, mixed before the
 *           resonators; at the top the voicing fades out (whisper)
 *   F1..F4  y = A x + B y1 + C y2, C = -r^2, B = 2 r cos(2 pi F / FS),
 *           A = 1 - B - C (unity gain at DC), Q30 coefficients per block;
 *           F4 is fixed at 3300 Hz and moves only with SHIFT / key tracking
 *   PRES    out = y + k (y - y[-1]), a first-difference shelf; k follows BUZZ
 *
 * Voice state: ph[0] glottal phase, ph[1..2] F4 y1 / y2, s[0..5] F1..F3
 * y1 / y2, s[6] tilt, s[7] TALK progress (Q24, bits 0..24) and the note's
 * random vowel (bits 25..31). Formant key tracking is fixed at 30 / 127. */
#define FMT_FOLOW 30
#define FMT_TALK_MASK 0x01FFFFFF

/* formants (Hz) of A E I O U: adult male means, Hillenbrand et al., JASA 97
 * (1995) (/a/ hod, /E/ head, /i/ heed, /o/ hoed, /u/ who'd); bandwidths (Hz)
 * in the range of Klatt (1980) and of measured vowel bandwidths */
static const uint16_t smc_VOWEL_F[5][3] = {
    {768, 1333, 2522}, {580, 1799, 2605}, {342, 2322, 3000}, {497, 910, 2459}, {378, 997, 2343},
};
static const uint8_t smc_VOWEL_B[5][3] = {
    {90, 100, 160}, {70, 100, 170}, {50, 120, 200}, {70, 80, 150}, {60, 80, 150},
};
/* 2^(k/12), Q16, k = 0..12 */
static const uint32_t smc_SEMI_Q16[13] = {
    65536, 69433, 73562, 77936, 82570, 87480, 92682, 98193, 104032, 110218, 116772, 123715, 131072,
};

/* 2^(st16 / 192) in Q16 (st16 in 1/16 semitone), clamped to 1/16 .. 16.
 * Divisions by constants are written as multiplies: -Os keeps them as divides */
static uint32_t smc_fmt_ratio(int32_t st16)
{
    int32_t o, k;
    uint32_t r;
    st16 = clamp(st16, -4 * 192, 4 * 192 - 1) + 4 * 192;   /* 0 .. 8 octaves */
    o = (st16 * 2731) >> 19;                            /* / 192, exact for 0 .. 1535 */
    k = st16 - o * 192;
    r = smc_SEMI_Q16[k >> 4] + (((smc_SEMI_Q16[(k >> 4) + 1] - smc_SEMI_Q16[k >> 4]) * (uint32_t)(k & 15)) >> 4);
    return o >= 4 ? r << (o - 4) : r >> (4 - o);
}

/* the three formants of vowel position pos (0..127 << 8, A E I O U), Hz * 16 */
static void smc_vowel_at(int32_t pos, int32_t *f, int32_t *b)
{
    int32_t x = (clamp(pos, 0, 127 << 8) * 33027) >> 20, i = x >> 8, fr = x & 255, j;   /* * 4 / 127 */
    if (i >= 4) {
        i = 3;
        fr = 256;
    }
    for (j = 0; j < 3; j++) {
        f[j] = (smc_VOWEL_F[i][j] << 4) + (((smc_VOWEL_F[i + 1][j] - smc_VOWEL_F[i][j]) * fr) >> 4);
        b[j] = (smc_VOWEL_B[i][j] << 4) + (((smc_VOWEL_B[i + 1][j] - smc_VOWEL_B[i][j]) * fr) >> 4);
    }
}

typedef struct { int32_t a, b, c; } smc_fres_t;
static int32_t smc_formant_nz = 0x2545F491;                 /* breath noise, one xorshift for all voices */

/* Klatt resonator coefficients (Q30) for F, BW in Hz * 16 */
static void smc_fres_coef(smc_fres_t *r, uint32_t f16, uint32_t bw16)
{
    int32_t s = sine_i(f16 * 3044u);                    /* sin(pi F / FS), Q15 (2^32 / (2 FS) / 16) */
    int32_t x = (int32_t)(bw16 * 4781u);                /* pi BW / FS, Q30 (<= 0.2) */
    int32_t x2 = (int32_t)(((int64_t)x * x) >> 30), x3 = (int32_t)(((int64_t)x2 * x) >> 30);
    int32_t x4 = (int32_t)(((int64_t)x3 * x) >> 30);
    int32_t e = (1 << 30) - x + (x2 >> 1) - (int32_t)(((int64_t)x3 * 178956971) >> 30)   /* exp(-x): x^3 / 6, */
                + (int32_t)(((int64_t)x4 * 44739243) >> 30);                            /* x^4 / 24 */
    int32_t cs = (1 << 30) - 2 * s * s;                 /* cos(2 pi F / FS) = 1 - 2 sin^2, Q30 */
    r->b = (int32_t)(((int64_t)e * cs) >> 29);          /* 2 r cos */
    r->c = -(int32_t)(((int64_t)e * e) >> 30);          /* -r^2 */
    r->a = (1 << 30) - r->b - r->c;
}

static void smc_formant_note_on(track_t *t, voice_t *v)
{
    (void)t;
    smc_formant_nz ^= smc_formant_nz << 13;                     /* a random vowel for this note (RAND) */
    smc_formant_nz ^= (int32_t)((uint32_t)smc_formant_nz >> 17);
    smc_formant_nz ^= smc_formant_nz << 5;
    v->ph[3]=(uint32_t)smc_formant_nz;
    v->s[7] = (int32_t)(((uint32_t)smc_formant_nz >> 25) << 25);   /* TALK starts over, random vowel kept */
    if (!v->env && !v->env_out) {                       /* a fresh voice (not a retrigger): from rest */
        uint32_t i;
        v->ph[0] = 0;                                   /* the opening starts at zero: no click */
        v->ph[1] = v->ph[2] = 0;
        for (i = 0; i < 7u; i++)
            v->s[i] = 0;
    }
}

static void smc_formant_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    smc_fres_t r1, r2, r3, r4;
    int32_t fa[3], ba[3], fb[3], bb[3], k, j, buzz, oq, tilt, gv, gn, og, ogk;
    uint32_t inc = m->inc, i, te, tp, rp, rn, ratio, bws, f0;
    uint32_t ph = v->ph[0];
    int32_t y1 = v->s[0], y2 = v->s[1], y3 = v->s[2], y4 = v->s[3], y5 = v->s[4], y6 = v->s[5];
    int32_t y7 = (int32_t)v->ph[1], y8 = (int32_t)v->ph[2], lp = v->s[6], nst = (int32_t)v->ph[3];

    /* vowel: VOWEL -> VOWL2 over TALK after note-on; ENV / LFO -> FLT move both */
    k = 0;
    if (p[P_E2]) {
        uint32_t g = ((uint32_t)v->s[7] & FMT_TALK_MASK) + ENV_LIN[p[P_E2] & 127];
        g = g > (1u << 24) ? 1u << 24 : g;
        v->s[7] = (int32_t)(((uint32_t)v->s[7] & ~(uint32_t)FMT_TALK_MASK) | g);
        k = (int32_t)(g >> 9);                          /* Q15 */
        k = (k * (65536 - k)) >> 15;                    /* ease out: the mouth opens fast, then settles */
    }
    {   /* RAND: both vowels move towards this note's random vowel (0 = off, 127 = fully random) */
        int32_t rv = (int32_t)(((uint32_t)v->s[7] >> 25) << 8), v0 = p[P_E0] << 8, v1 = p[P_E1] << 8;
        v0 += ((rv - v0) * p[P_E7] * 258) >> 15;
        v1 += ((rv - v1) * p[P_E7] * 258) >> 15;
        smc_vowel_at(v0 + m->cutoff, fa, ba);
        smc_vowel_at(v1 + m->cutoff, fb, bb);
    }
    /* SHIFT (semitones) and FOLOW (key tracking around C4, fixed), one ratio for all four */
    ratio = smc_fmt_ratio(p[P_E3] * 16 + ((FMT_FOLOW * (v->pitch_cur - 60 * 16) * 516) >> 16));   /* FOLOW / 127 */
    bws = smc_fmt_ratio((64 - p[P_E6]) * 4);                /* Q: bandwidth x2.5 .. x0.4 */
    f0 = (uint32_t)(((uint64_t)inc * 705600u) >> 32);   /* F0 in Hz * 16 */
    for (j = 0; j < 3; j++) {
        int32_t f = fa[j] + (((fb[j] - fa[j]) * k) >> 15), b = ba[j] + (((bb[j] - ba[j]) * k) >> 15);
        f = (int32_t)(((int64_t)f * ratio) >> 16);
        b = (int32_t)(((((int64_t)b * ratio) >> 16) * bws) >> 16);
        if (j == 0 && (uint32_t)f < f0 + (f0 >> 4) && f > 0) {   /* sung high: F1 follows F0 (sopranos do), */
            b = (int32_t)(((int64_t)b * (int32_t)(f0 + (f0 >> 4))) / f);   /* same Q: no boom on the fundamental */
            f = (int32_t)(f0 + (f0 >> 4));
        }
        fa[j] = clamp(f, 60 * 16, 7000 * 16);
        ba[j] = clamp(b, 20 * 16, 1500 * 16);
    }
    smc_fres_coef(&r1, (uint32_t)fa[0], (uint32_t)ba[0]);
    smc_fres_coef(&r2, (uint32_t)fa[1], (uint32_t)ba[1]);
    smc_fres_coef(&r3, (uint32_t)fa[2], (uint32_t)ba[2]);
    smc_fres_coef(&r4, (uint32_t)clamp((int32_t)((3300u * 16u * (uint64_t)ratio) >> 16), 60 * 16, 7000 * 16),
              (uint32_t)clamp((int32_t)((((250u * 16u * (uint64_t)ratio) >> 16) * bws) >> 16), 20 * 16, 1500 * 16));

    /* source: BUZZ (+ SHP modulation, + velocity) -> open quotient and tilt */
    buzz = clamp((p[P_E4] << 8) + m->shape - (64 << 8) + (v->vel - 96) * 40, 0, 127 << 8);
    oq = 58982 - ((buzz * 284) >> 8);                   /* Q16: 0.90 .. 0.35 */
    tilt = 2500 + ((buzz * 109) >> 8);                  /* one-pole coefficient Q14: dark .. open */
    te = (uint32_t)oq << 16;                            /* closure (end of the open phase) */
    tp = (te >> 8) * 166u;                              /* opening: 65 % of it, closing 35 % */
    rp = (1u << 30) / ((tp >> 16) | 1u);
    rn = (1u << 30) / (((te - tp) >> 16) | 1u);
    gv = 32767 - p[P_E5] * p[P_E5] * 2;                 /* BREATH: voicing fades at the top */
    gn = p[P_E5] * 258;                                 /* noise: Q15, up to 1.0 */
    og = 13000 - ((p[P_E6] - 64) * 50);                 /* output: narrower bands ring louder */
    og = (int32_t)(((int64_t)og * smc_fmt_ratio((60 * 16 - v->pitch_cur) / 4)) >> 16);   /* 1.5 dB / oct: low notes are sparse pulses */
    ogk = (og >> 2) * (4 + (buzz >> 10));               /* presence k = 1 .. 4.9 */

    for (i = 0; i < n; i++) {
        int32_t e, x, a, s;
        if (ph < tp)                                    /* opening: + sin(pi u) * Tn / Tp */
            e = (sine_i(((ph >> 16) * rp) << 1) * 17644) >> 15;
        else if (ph < te)                               /* closing: - sin(pi / 2 u) */
            e = -sine_i(((ph - tp) >> 16) * rn);
        else
            e = 0;
        e += smc_blep(ph - te, inc) >> 1;                   /* the closure step, band-limited */
        lp += ((e - lp) * tilt) >> 14;
        x = (lp * gv) >> 15;
        if (gn) {
            int32_t nz = (((int32_t)(noise32(&nst) >> 16) - 32768) * gn) >> 15;
            x += ph < te ? nz : nz >> 1;                /* aspiration follows the glottal opening */
        }
        /* F1 -> F2 -> F3 -> F4, Q30, rounded; the signal x 64 inside: the rounding
         * dead band of F1 (+-0.5 / A, ~340 at 270 Hz) stays below 6 LSB */
        a = (int32_t)(((int64_t)r1.a * (x << 6) + (int64_t)r1.b * y1 + (int64_t)r1.c * y2 + (1 << 29)) >> 30);
        y2 = y1;
        y1 = a;
        a = (int32_t)(((int64_t)r2.a * a + (int64_t)r2.b * y3 + (int64_t)r2.c * y4 + (1 << 29)) >> 30);
        y4 = y3;
        y3 = a;
        a = (int32_t)(((int64_t)r3.a * a + (int64_t)r3.b * y5 + (int64_t)r3.c * y6 + (1 << 29)) >> 30);
        y6 = y5;
        y5 = a;
        a = (int32_t)(((int64_t)r4.a * a + (int64_t)r4.b * y7 + (int64_t)r4.c * y8 + (1 << 29)) >> 30);
        y8 = y7;
        y7 = clamp(a, -(1 << 28), 1 << 28);             /* int32 state stays far from overflow */
        s = clamp((int32_t)(((int64_t)y7 * og + (int64_t)(y7 - y8) * ogk) >> 21), -200000, 200000);
        a = s < 0 ? -s : s;
        if (a > 24000) {                                /* soft knee: only peaks saturate */
            a = 24000 + (softclip((a - 24000) * 2) >> 1);
            s = s < 0 ? -a : a;
        }
        ph += inc;
        out[i] += mulq15(mulq15(s, amp_at(m, i)), VOICE_FS) << 1;
    }
    v->ph[0] = ph;
    v->ph[1] = (uint32_t)y7;
    v->ph[2] = (uint32_t)y8;
    v->ph[3] = (uint32_t)nst;
    v->s[0] = y1;
    v->s[1] = y2;
    v->s[2] = y3;
    v->s[3] = y4;
    v->s[4] = y5;
    v->s[5] = y6;
    v->s[6] = lp;
}


static const preset_t SMC_FORMANT_PRESETS[] PRESET_TABLE ={
{"CHOIR AAH",{0,0,0,0,40,22,60,0},{85,90,115,95},0,0},
{"SOUL OOH",{110,0,0,0,30,30,55,0},{70,90,115,90},0,0},
{"TALKBOX",{32,95,60,0,100,4,75,0},{4,70,110,40},0,1},
{"WOW BASS",{95,0,68,0,100,0,80,0},{0,70,70,30},0,1},
{"HARMONICA",{64,0,0,0,127,20,70,0},{4,70,115,25},0,1},
{"ROBOT",{40,40,0,0,127,0,110,0},{0,60,110,20},0,1},
{"VOX PAD",{20,100,110,0,50,30,60,10},{80,90,118,90},0,0},
{"MALE CHOIR",{20,0,0,-5,50,20,60,10},{80,90,115,95},0,0},
{"WHISPER",{60,100,90,0,0,127,50,0},{20,80,110,60},0,0},
{"ROBO VOX",{64,10,40,0,127,0,100,0},{2,70,110,30},0,0},
{"AAH LEAD",{0,0,0,0,90,10,70,0},{4,70,115,40},0,1},
{"UH BASS",{110,0,0,0,110,0,80,0},{0,70,90,25},0,1},
};
static const engine_t ENG_SMC_FORMANT={.pool=1,.name="VOICE",.page_title={"VOWL", "TONE"},.edit={
        {"VOWL", F_INT, 0, 127, 0, 0, 0},
        {"VOWL2", F_INT, 0, 127, 64, 0, 0},
        {"TALK", F_TIME, 0, 127, 0, 0, 0},
        {"SHIFT", F_SEMI, -12, 12, 0, 0, 0},
        {"BUZZ", F_PCT, 0, 127, 64, 0, 0},
        {"BRTH", F_PCT, 0, 127, 10, 0, 0},
        {"Q", F_PCT, 0, 127, 64, 0, 0},
        {"RAND", F_PCT, 0, 127, 0, 0, 0},
    },.presets=SMC_FORMANT_PRESETS,.npresets=12,.knob={P_E0,P_E4,P_E5,P_E2},.note_on=smc_formant_note_on,.render=smc_formant_render,.poly=4};
/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* STRINGS: a string machine in the manner of the 1970s ensembles (Solina, ARP, Elka) (SynthMaven 0.2).
 * Divide-down registers: per tap one phase at the 16' pitch, the 8' and the 4' its double and quadruple, so the
 * registers stay locked as on the originals (polyBLEP saws, or towards a square with SHP). The ensemble is three taps,
 * each a copy of the registers whose pitch two LFOs move, 120 degrees apart from tap to tap (the slow ~0.6 Hz sweep
 * and the fast ~6 Hz shimmer of the three BBD lines), the depth ENS, the speed RATE. Then a 12 dB low-pass.
 * EDIT 1  16'  8'  4'  ENS        EDIT 2  CUT  RATE  SHP  RES
 * Voice state: ph[0..2] the three taps (16' phase), s[0..1] the SVF. Part state: the ensemble LFOs (smc_strings_block). */
static uint32_t smc_str_lfo[NPART][2];               /* the ensemble's slow and fast LFO, per part (smc_strings_block) */

static void smc_strings_block(track_t *t)
{
    uint32_t part = (uint32_t)(t - trk) % NPART;
    int32_t rate = t->p[P_E5];                       /* x 0.25 .. x 2 */
    uint32_t k = 8192u + (uint32_t)rate * 448u;      /* Q15 */
    smc_str_lfo[part][0] += (uint32_t)(((uint64_t)(58000u * CTL) * k) >> 15);      /* ~0.6 Hz at RATE 50 */
    smc_str_lfo[part][1] += (uint32_t)(((uint64_t)(584000u * CTL) * k) >> 15);     /* ~6 Hz */
}

static void smc_strings_note_on(track_t *t, voice_t *v)
{
    (void)t;
    if (!v->env && !v->env_out) {
        v->ph[0] = v->ph[1] = v->ph[2] = 0;          /* the taps start together (as the BBD lines share one input:
                                                      * set apart a third of a turn they would cancel all but every
                                                      * third harmonic); the LFOs pull them apart */
        v->s[0] = v->s[1] = 0;
    }
}

/* a register: a saw, towards a square with SHP (x Q15: 0 saw .. 32767 square) */
static inline int32_t smc_str_reg(uint32_t ph, uint32_t inc, int32_t x)
{
    int32_t a = hy_saw(ph, inc);
    return x ? a + mulq15(((a - hy_saw(ph + 0x80000000u, inc)) >> 1) - a, x) : a;
}

static void smc_strings_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    int32_t acc[CTL];                                /* (0.7: on the stack: two cores) */
    uint32_t part = (uint32_t)(t - trk) % NPART, i, k;
    int32_t g16 = p[P_E0] * 34, g8 = p[P_E1] * 34, g4 = p[P_E2] * 34;  /* Q15 / 7.5 (three taps, three
                                                                         * registers: headroom for chords) */
    int32_t ens = p[P_E3], shp = p[P_E6] * 258;
    int32_t ic1 = v->s[0], ic2 = v->s[1];
    uint32_t inc16 = m->inc >> 1;
    tsvf_t flt;
    for (i = 0; i < n; i++)
        acc[i] = 0;
    for (k = 0; k < 3u; k++) {
        /* the tap's pitch: the slow and the fast LFO, 120 degrees on; up to ~+-14 cents and ~+-4 cents at ENS 127 */
        int32_t s1 = osc_sine(smc_str_lfo[part][0] + k * 0x55555555u), s2 = osc_sine(smc_str_lfo[part][1] + k * 0x55555555u);
        int32_t dq = ((s1 * 14 + s2 * 4) >> 4) * ens;                   /* cents x 127 x 2048 */
        int32_t fq = dq * 38 / 260096;                                  /* the pitch factor - 1, Q16 (~38 a cent) */
        uint32_t inc = inc16 + (uint32_t)(((int64_t)(int32_t)inc16 * fq) >> 16), ph = v->ph[k];
        for (i = 0; i < n; i++, ph += inc) {
            int32_t s = 0;
            if (g16)
                s += mulq15(smc_str_reg(ph, inc, shp), g16);
            if (g8)
                s += mulq15(smc_str_reg(ph << 1, inc << 1, shp), g8);
            if (g4)
                s += mulq15(smc_str_reg(ph << 2, inc << 2, shp), g4);
            acc[i] += s;
        }
        v->ph[k] = ph;
    }
    smc_tsvf_coef(&flt, (p[P_E4] << 8) + m->cutoff + ((v->pitch16 - 60 * 16) << 3), p[P_E7]);   /* (keyboard ~40 %) */
    for (i = 0; i < n; i++) {
        int32_t y = tsvf_lp(&flt, acc[i] >> 1, &ic1, &ic2);
        if (y > 16000 || y < -16000)
            y = soft_knee(y, 16000);
        out[i] += mulq15(mulq15(y << 1, amp_at(m, i)), VOICE_FS) << 1;
    }
    v->s[0] = ic1;
    v->s[1] = ic2;
}


static const preset_t SMC_STRINGS_PRESETS[] PRESET_TABLE ={
{"SOLINA",{40,127,70,100,92,50,0,10},{50,100,120,80},0,0},
{"VIOLINS",{0,127,100,90,100,60,20,15},{40,100,120,70},0,0},
{"CELLOS",{127,80,0,80,76,45,30,20},{50,100,120,80},0,0},
{"ENSEMBLE",{90,110,90,127,96,55,10,10},{60,100,125,90},0,0},
{"STR MACHINE",{60,127,40,110,84,40,0,25},{30,100,120,60},0,0},
{"GLASS STR",{0,90,127,120,110,70,50,30},{70,100,120,95},0,0},
{"DARK ENS",{127,100,20,100,64,35,0,15},{80,100,125,100},0,0},
{"PULSE STR",{50,127,60,90,90,50,110,10},{40,100,120,70},0,0},
{"OMNI STR",{60,127,90,110,96,55,10,15},{50,100,120,80},0,0},
{"SLOW STR",{40,127,60,100,84,40,0,10},{100,100,125,105},0,0},
{"STR STAB",{50,127,80,90,100,60,0,10},{0,80,60,50},20,0},
{"CHOIR STR",{20,127,90,127,80,30,60,30},{70,100,120,90},0,0},
{"BRIGHT STR",{0,127,127,100,112,60,0,20},{40,100,120,70},0,0},
{"LOW STR",{127,60,0,90,70,40,10,10},{60,100,120,80},0,0},
{"80S STR",{40,127,60,60,96,80,40,10},{40,100,120,70},0,0},
{"TAPE STR",{60,127,50,127,80,20,0,10},{60,100,120,80},0,0},
};
/* Three ensemble taps per register cost more than an ordinary two-oscillator
 * voice. Reserve four shared-budget units; four-part chords plus independent
 * drums otherwise overrun the FM-1 audio deadline. Sound parameters are unchanged. */
static uint32_t smc_strings_units(const track_t *t){(void)t;return 4u;}
static const engine_t ENG_SMC_STRINGS={.pool=1,.name="STRINGS",.page_title={"REG", "TONE"},.edit={
        {"16'", F_PCT, 0, 127, 40, 0, 0},
        {"8'", F_PCT, 0, 127, 127, 0, 0},
        {"4'", F_PCT, 0, 127, 70, 0, 0},
        {"ENS", F_PCT, 0, 127, 100, 0, 0},
        {"CUT", F_CUTOFF, 0, 127, 92, 0, 0},
        {"RATE", F_PCT, 0, 127, 50, 0, 0},
        {"SHP", F_PCT, 0, 127, 0, 0, 0},
        {"RES", F_PCT, 0, 127, 10, 0, 0},
    },.presets=SMC_STRINGS_PRESETS,.npresets=16,.knob={P_E3,P_E4,P_E1,P_ATK},.note_on=smc_strings_note_on,.render=smc_strings_render,.block=smc_strings_block,.units=smc_strings_units};
/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* JUNO (SynthMaven 0.8): the poly synth of the Roland Juno-6 / 60 / 106, one of the most loved sounds there is (its
 * idea: one DCO with a saw, a pulse whose width the LFO moves, a sub oscillator an octave down and noise; a high-pass
 * in four positions; a 24 dB resonant low-pass; and the BBD chorus, I, II or both). No code or sample of Roland's.
 * EDIT 1  WAVE  PW  SUB  NOIS        EDIT 2  HPF  CUT  RES  CHOR
 *   WAVE  SAW, PLS (the pulse), S+P (both, as the 106's two buttons)
 *   PW    the pulse's width, 50 % (square) .. 5 %; SHAPE (LFO DEST SHP: the PWM, MW / AT SHP, the matrix) moves it
 *   SUB   the square an octave down; NOIS white noise
 *   HPF   0 a little bass boost, 1 flat, 2 and 3 a high-pass at ~240 and ~720 Hz (the 106's four positions)
 *   CUT RES  the 24 dB ladder (dsp.c, Zavalishin's), keyboard tracking half; ENV DEST FLT its envelope
 *   CHOR  OFF, I (0.5 Hz, wide), II (0.8 Hz, deeper), I+II (8 Hz, shallow: the 60's both buttons): stereo, after the
 *         sound's INSERT and FILTER (fx.c juno_chorus: one per part; the INSERT's JUNO types give it to any sound)
 * The oscillators are band-limited (polyBLEP: dsp.c smc_osc_saw / smc_osc_pulse).
 * Voice state: ph[0] the DCO, ph[1] the sub, ph[2] the noise; s[0..3] the ladder, s[4] the HPF's low-pass, s[5] the
 * bass boost's */
enum { JW_SAW, JW_PLS, JW_SP };
static const char *const smc_N_JWAVE[] = {"SAW", "PLS", "S+P"};
static const char *const smc_N_JHPF[] = {"0", "1", "2", "3"};
static const char *const smc_N_JCHOR[] = {"OFF", "I", "II", "I+II"};

static void smc_juno_note_on(track_t *t, voice_t *v)
{
    (void)t;
    if (!v->env && !v->env_out) {                    /* from silence: the filter clear (a DCO: its phase runs on) */
        v->s[0] = v->s[1] = v->s[2] = v->s[3] = v->s[4] = v->s[5] = 0;
        if (!v->ph[2])
            v->ph[2] = 0x2545F491u + v->note * 2654435761u;
    }
}

static void smc_juno_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    uint32_t i, ph = v->ph[0], phs = v->ph[1], nz = v->ph[2], inc = m->inc, incs = inc >> 1;
    uint32_t wave = (uint32_t)clamp(p[P_E0], 0, 2), hpf = (uint32_t)clamp(p[P_E4], 0, 3);
    int32_t w = clamp(p[P_E1] * 256 + (m->shape - (64 << 8)) * 2, 0, 127 * 256);   /* 0 square .. 127 * 256 narrow */
    uint32_t pw = 0x80000000u - (uint32_t)w * 59000u;                               /* 50 % .. ~5 % */
    int32_t sub = p[P_E2] * 200, nois = p[P_E3] * 160, hp = v->s[4], bb = v->s[5];
    int32_t cut = (p[P_E5] << 8) + m->cutoff + ((v->pitch16 - 60 * 16) << 3);
    int32_t ls[4] = {v->s[0], v->s[1], v->s[2], v->s[3]};
    ladder_t lad;
    ladder_coef(&lad, cut, p[P_E6]);
    for (i = 0; i < n; i++) {
        int32_t s, y, a;
        if (wave == JW_SAW)
            s = smc_osc_saw(ph, inc);
        else if (wave == JW_PLS)
            s = smc_osc_pulse(ph, inc, pw);
        else
            s = (smc_osc_saw(ph, inc) + smc_osc_pulse(ph, inc, pw)) * 5 >> 3;
        if (sub)
            s += mulq15(smc_osc_pulse(phs, incs, 0x80000000u), sub);
        if (nois) {
            nz = nz * 1664525u + 1013904223u;
            s += mulq15((int32_t)(nz >> 16) - 32768, nois);
        }
        ph += inc;
        phs += incs;
        s = s * 5 >> 3;                              /* (four sources: headroom) */
        if (hpf != 1u) {                             /* 0: bass boost (a low shelf ~120 Hz); 2, 3: a one-pole high-pass */
            if (!hpf) {
                bb += ((s - bb) * 560) >> 15;
                s += bb >> 1;
            } else {
                hp += ((s - hp) * (hpf == 2u ? 1120 : 3360)) >> 15;
                s -= hp;
            }
        }
        y = ladder_tick(&lad, s, ls) >> 1;
        a = y < 0 ? -y : y;
        if (a > 16000) {
            a = 16000 + (softclip((a - 16000) * 2) >> 1);
            y = y < 0 ? -a : a;
        }
        out[i] += mulq15(mulq15(y << 1, amp_at(m, i)), VOICE_FS) << 1;
    }
    v->ph[0] = ph;
    v->ph[1] = phs;
    v->ph[2] = nz;
    v->s[0] = ls[0], v->s[1] = ls[1], v->s[2] = ls[2], v->s[3] = ls[3];
    v->s[4] = hp;
    v->s[5] = bb;
}


static const preset_t SMC_JUNO_PRESETS[] PRESET_TABLE ={
{"JUNO 106 PAD",{2,40,40,0,1,88,20,1},{70,100,115,95},10,0},
{"JUNO BRASS",{0,0,30,0,1,62,30,0},{14,82,100,50},40,0},
{"JUNO BASS",{1,50,110,0,0,58,40,0},{0,70,70,26},36,1},
{"JUNO STRINGS",{0,0,0,0,2,96,10,2},{80,100,120,96},0,0},
{"POLY KEYS",{2,30,20,0,1,70,24,1},{0,88,60,60},30,0},
{"SYNTHWAVE LD",{0,0,60,0,1,90,30,1},{2,80,110,50},10,1},
{"80S HITS",{2,20,50,0,1,56,50,1},{0,64,0,40},50,0},
{"JUNO PIANO",{1,60,30,0,2,50,20,1},{0,90,0,70},50,0},
{"DREAM PAD",{2,50,30,10,2,92,30,3},{90,100,120,110},0,0},
{"SUB BASS 106",{1,0,127,0,0,40,10,0},{0,80,110,20},10,1},
{"SYNTH BRASS 2",{2,10,40,0,1,54,36,2},{20,86,96,56},46,0},
{"ARP PLUCK",{0,0,40,0,2,44,50,1},{0,60,0,40},56,0},
{"NOISE SWEEP",{0,0,0,127,3,40,90,3},{60,110,100,100},50,0},
{"JUNO ORGAN",{1,100,90,0,1,100,0,2},{0,30,127,20},0,0},
{"SAW LEAD 106",{0,0,30,0,1,82,44,0},{0,80,100,40},24,1},
{"HOOVER JUNO",{1,80,60,0,1,76,20,3},{0,90,110,50},0,1},
{"TURING JUNO",{0,0,20,0,2,64,36,1},{0,62,24,40},44,0},
};
static const engine_t ENG_SMC_JUNO={.pool=1,.name="JUNO",.page_title={"DCO", "VCF"},.edit={
        {"WAVE", F_ENUM, 0, 2, JW_SP, smc_N_JWAVE, 0},
        {"PW", F_PCT, 0, 127, 30, 0, 0},
        {"SUB", F_PCT, 0, 127, 40, 0, 0},
        {"NOIS", F_PCT, 0, 127, 0, 0, 0},
        {"HPF", F_ENUM, 0, 3, 1, smc_N_JHPF, 0},
        {"CUT", F_CUTOFF, 0, 127, 90, 0, 0},
        {"RES", F_PCT, 0, 127, 20, 0, 0},
        {"CHOR", F_ENUM, 0, 3, 1, smc_N_JCHOR, 0},
    },.presets=SMC_JUNO_PRESETS,.npresets=17,.knob={P_E5,P_E6,P_E1,P_E7},.note_on=smc_juno_note_on,.render=smc_juno_render};

/* Named-parameter conversion: legacy FX/matrix IDs are never reinterpreted. */
static const uint8_t SMC_PARAM[]={P_ATK,P_DEC,P_SUS,P_REL,P_ED_FLT,P_ED_PIT,P_ED_SHP,P_LRATE,P_LWAVE,P_LPHASE,P_LFADE,P_LD_PIT,P_LD_FLT,P_LD_SHP,P_LD_AMP,P_VOICE,P_GLIDE,P_TRANS,P_PAN,P_GLMODE,P_PRIO,P_ALLOC,P_DETUNE,P_E0,P_E1,P_E2,P_E3,P_E4,P_E5,P_E6,P_E7,SM_CLASSIC_FILTER};
static const int8_t SMC_ANALOG_PARAM[][32]={
{0,112,0,52,0,18,0,60,0,0,0,0,0,0,0,2,59,-24,0,1,0,0,40,3,0,0,0,127,0,48,0,1},
{0,108,0,46,0,24,0,60,0,0,0,0,0,0,0,2,56,-24,0,1,0,0,40,3,0,0,0,96,12,118,30,1},
{0,60,118,24,0,0,0,60,0,0,0,0,0,0,0,2,40,-24,0,0,0,0,40,3,0,0,0,127,0,10,0,1},
{0,118,0,60,0,12,0,60,0,0,0,0,0,0,0,2,82,-24,0,1,0,0,40,3,0,0,0,127,0,72,0,1},
{0,56,24,20,52,0,0,60,0,0,0,0,0,0,0,2,34,-12,0,1,0,0,40,0,0,0,0,36,112,44,64,1},
{0,96,92,34,0,0,0,60,0,0,0,0,0,0,0,2,52,-24,0,1,0,0,40,2,0,0,0,92,0,30,0,1},
{0,70,118,28,0,0,0,22,0,0,0,0,10,0,0,2,0,-24,0,0,0,0,40,0,22,64,0,56,22,36,40,1},
{0,64,127,22,0,0,0,74,0,0,0,0,40,0,0,2,0,-24,0,0,0,0,40,0,9,64,0,38,72,62,30,1},
{0,55,62,20,30,0,0,60,0,0,0,0,0,0,0,2,50,-24,0,1,0,0,40,1,6,50,0,50,30,22,64,1},
{6,70,112,46,0,0,0,89,0,0,50,2,0,0,0,2,74,0,0,0,0,0,40,2,0,0,0,92,0,10,64,1},
{0,80,24,45,48,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,8,64,0,52,35,20,64,1},
{12,70,96,35,34,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,10,64,0,45,20,16,64,1},
{4,70,112,46,10,0,0,60,0,0,0,0,0,0,0,3,30,0,0,0,0,0,64,0,14,64,0,96,12,10,64,1},
{75,90,115,90,0,0,0,30,0,0,0,0,0,20,0,0,0,0,0,0,0,0,40,4,12,64,6,52,8,0,32,1},
{60,90,118,85,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,18,64,0,48,6,0,32,1},
{90,100,120,100,0,0,0,18,0,0,0,1,0,0,0,0,0,0,0,0,0,0,40,4,25,64,20,60,15,0,32,1},
{14,70,110,40,0,0,0,80,0,0,70,1,0,0,0,2,0,0,0,0,0,0,40,2,0,0,30,70,10,0,64,1},
{2,70,110,30,30,0,0,60,0,0,0,0,0,0,0,2,40,0,0,0,0,0,40,0,8,64,0,70,30,30,64,1},
{0,60,70,20,44,0,0,60,0,0,0,0,0,0,0,2,0,-24,0,0,0,0,40,0,4,64,0,48,36,30,64,1},
{2,70,110,30,0,0,0,80,0,0,60,1,0,0,0,2,30,0,0,0,0,0,40,4,0,0,0,80,20,10,64,1},
{0,60,60,20,30,0,0,60,0,0,0,0,0,0,0,2,0,-24,0,0,0,0,40,0,6,64,0,52,30,40,64,1},
{80,90,118,90,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,3,10,64,4,90,0,0,32,1},
{0,64,70,24,46,0,0,60,0,0,0,0,0,0,0,2,26,-24,0,0,0,0,40,0,6,64,0,42,60,40,64,2},
{2,70,110,40,26,0,0,82,0,0,0,1,0,0,0,2,36,0,0,0,0,0,40,0,9,64,0,62,80,30,64,2},
{60,100,118,88,8,0,0,30,0,0,0,0,0,30,0,0,0,0,0,0,0,0,40,4,14,64,4,62,10,0,40,1},
{14,74,100,40,34,0,0,60,0,0,0,0,0,0,0,3,0,0,0,0,0,0,50,0,16,64,0,50,20,10,64,1},
{0,80,100,30,30,0,0,60,0,0,0,0,0,0,0,2,20,-24,0,0,0,0,40,0,6,64,0,40,40,50,64,2},
{2,70,110,40,20,0,0,60,0,0,0,0,0,0,0,2,30,0,0,0,0,0,40,1,10,64,0,74,40,20,64,2},
{0,62,20,30,40,0,0,60,0,0,0,0,0,0,0,3,0,0,0,0,0,0,60,0,20,64,0,70,30,10,64,1},
{0,70,0,50,50,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,1,4,64,0,48,40,0,80,1},
{0,64,80,24,30,0,0,60,0,0,0,0,0,0,0,2,0,-12,0,0,0,0,40,0,6,64,0,40,50,30,64,2},
{2,70,110,40,0,0,0,70,0,0,0,0,20,0,0,2,30,0,0,0,0,0,40,0,8,64,0,60,70,20,64,1},
};
static const int8_t SMC_DIGITAL_PARAM[][32]={
{0,85,35,55,0,0,0,84,0,0,0,0,0,0,26,0,0,0,0,0,0,0,40,4,1,1,14,26,35,0,0,1},
{0,82,40,55,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,1,1,14,85,45,0,0,1},
{0,75,30,45,0,0,0,89,0,0,0,0,0,0,16,0,0,0,0,0,0,0,40,4,1,2,3,62,35,8,0,1},
{0,58,34,18,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,3,1,3,5,84,25,7,0,1},
{0,86,30,48,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,1,2,13,74,30,12,0,1},
{0,82,40,60,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,1,1,7,40,50,0,0,1},
{0,92,0,70,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,8,1,4,80,70,0,0,1},
{0,90,0,75,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,4,3,9,45,75,0,0,1},
{0,80,0,60,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,4,1,1,60,30,0,0,1},
{0,78,0,58,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,4,1,6,52,24,0,0,1},
{0,90,0,82,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,3,1,4,56,60,0,0,1},
{62,92,112,92,0,0,0,40,0,0,0,1,0,0,0,0,0,0,0,0,0,0,40,4,2,1,5,34,100,0,0,1},
{0,60,70,25,0,0,0,60,0,0,0,0,0,0,0,2,36,-24,0,0,0,0,40,0,1,1,1,34,40,8,0,1},
{0,88,0,66,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,2,2,2,45,45,0,0,1},
{0,95,0,85,0,0,0,60,0,0,0,0,0,0,0,0,0,12,0,0,0,0,40,4,5,1,8,40,60,0,0,1},
{0,84,0,70,0,0,0,60,0,0,0,0,0,0,0,0,0,12,0,0,0,0,40,4,2,1,4,30,50,0,0,1},
{0,77,0,58,0,0,0,60,0,0,0,0,0,0,0,0,0,12,0,0,0,0,40,4,4,3,3,70,18,0,0,1},
{0,92,0,80,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,1,1,2,45,30,10,0,1},
{0,76,40,22,0,0,0,60,0,0,0,0,0,0,0,2,0,-24,0,0,0,0,40,4,1,1,2,40,30,6,0,1},
{0,100,0,90,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,6,1,13,50,90,0,0,1},
{0,78,0,54,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,2,1,3,70,15,0,0,1},
{0,90,20,60,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,1,2,12,60,40,10,0,1},
{0,60,80,20,0,0,0,60,0,0,0,0,0,0,0,2,0,-12,0,0,0,0,40,0,1,1,1,46,26,10,0,1},
{0,85,35,60,0,0,0,66,0,0,0,0,0,0,14,0,0,0,0,0,0,0,40,4,1,1,14,30,38,0,0,1},
{0,104,0,96,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,6,1,9,44,95,0,0,1},
{20,80,110,40,0,0,20,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,1,1,1,48,70,6,0,1},
{2,70,110,40,0,0,0,60,0,0,0,0,0,0,0,2,30,0,0,0,0,0,40,0,1,2,1,40,60,10,0,1},
{0,80,0,50,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,7,1,11,60,25,0,0,1},
{0,92,0,74,0,0,0,70,0,0,0,0,0,0,20,0,0,0,0,0,0,0,40,4,4,1,4,40,40,0,0,1},
{0,64,120,30,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,4,2,1,3,30,20,0,0,1},
};
static const int8_t SMC_FORMANT_PARAM[][32]={
{85,90,115,95,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,0,0,0,40,22,60,0,1},
{70,90,115,90,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,110,0,0,0,30,30,55,0,1},
{4,70,110,40,0,0,0,89,0,0,0,2,0,0,0,2,60,0,0,0,0,0,40,32,95,60,0,100,4,75,0,1},
{0,70,70,30,0,0,0,60,0,0,0,0,0,0,0,2,0,-12,0,0,0,0,40,95,0,68,0,100,0,80,0,1},
{4,70,115,25,0,0,0,80,0,0,60,0,0,0,18,2,0,0,0,0,0,0,40,64,0,0,0,127,20,70,0,1},
{0,60,110,20,0,0,0,60,0,0,0,0,0,0,0,2,20,0,0,0,0,0,40,40,40,0,0,127,0,110,0,1},
{80,90,118,90,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,20,100,110,0,50,30,60,10,1},
{80,90,115,95,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,20,0,0,-5,50,20,60,10,1},
{20,80,110,60,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,60,100,90,0,0,127,50,0,1},
{2,70,110,30,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,64,10,40,0,127,0,100,0,1},
{4,70,115,40,0,0,0,60,0,0,0,0,0,0,0,2,30,0,0,0,0,0,40,0,0,0,0,90,10,70,0,1},
{0,70,90,25,0,0,0,60,0,0,0,0,0,0,0,2,0,-12,0,0,0,0,40,110,0,0,0,110,0,80,0,1},
};
static const int8_t SMC_STRINGS_PARAM[][32]={
{50,100,120,80,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,40,127,70,100,92,50,0,10,1},
{40,100,120,70,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,127,100,90,100,60,20,15,1},
{50,100,120,80,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,127,80,0,80,76,45,30,20,1},
{60,100,125,90,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,90,110,90,127,96,55,10,10,1},
{30,100,120,60,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,60,127,40,110,84,40,0,25,1},
{70,100,120,95,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,90,127,120,110,70,50,30,1},
{80,100,125,100,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,127,100,20,100,64,35,0,15,1},
{40,100,120,70,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,50,127,60,90,90,50,110,10,1},
{50,100,120,80,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,60,127,90,110,96,55,10,15,1},
{100,100,125,105,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,40,127,60,100,84,40,0,10,1},
{0,80,60,50,20,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,50,127,80,90,100,60,0,10,1},
{70,100,120,90,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,20,127,90,127,80,30,60,30,1},
{40,100,120,70,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,127,127,100,112,60,0,20,1},
{60,100,120,80,0,0,0,60,0,0,0,0,0,0,0,0,0,-12,0,0,0,0,40,127,60,0,90,70,40,10,10,1},
{40,100,120,70,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,40,127,60,60,96,80,40,10,1},
{60,100,120,80,0,0,0,30,0,0,0,2,0,0,0,0,0,0,0,0,0,0,40,60,127,50,127,80,20,0,10,1},
};
static const int8_t SMC_JUNO_PARAM[][32]={
{70,100,115,95,10,0,0,28,0,0,0,0,0,40,0,0,0,0,0,0,0,0,40,2,40,40,0,1,88,20,1,1},
{14,82,100,50,40,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,0,30,0,1,62,30,0,1},
{0,70,70,26,36,0,0,60,0,0,0,0,0,0,0,2,10,0,0,0,0,0,40,1,50,110,0,0,58,40,0,1},
{80,100,120,96,0,0,0,34,0,0,0,0,0,30,0,0,0,0,0,0,0,0,40,0,0,0,0,2,96,10,2,1},
{0,88,60,60,30,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,2,30,20,0,1,70,24,1,1},
{2,80,110,50,10,0,0,70,0,0,0,3,0,0,0,2,24,0,0,0,0,0,40,0,0,60,0,1,90,30,1,1},
{0,64,0,40,50,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,2,20,50,0,1,56,50,1,1},
{0,90,0,70,50,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,1,60,30,0,2,50,20,1,1},
{90,100,120,110,0,0,0,20,0,0,0,0,0,50,0,0,0,0,0,0,0,0,40,2,50,30,10,2,92,30,3,1},
{0,80,110,20,10,0,0,60,0,0,0,0,0,0,0,2,0,0,0,0,0,0,40,1,0,127,0,0,40,10,0,1},
{20,86,96,56,46,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,2,10,40,0,1,54,36,2,1},
{0,60,0,40,56,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,0,40,0,2,44,50,1,1},
{60,110,100,100,50,0,0,10,0,0,0,0,50,0,0,0,0,0,0,0,0,0,40,0,0,0,127,3,40,90,3,1},
{0,30,127,20,0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,1,100,90,0,1,100,0,2,1},
{0,80,100,40,24,0,0,60,0,0,0,0,0,0,0,2,20,0,0,0,0,0,40,0,0,30,0,1,82,44,0,1},
{0,90,110,50,0,0,0,60,0,0,0,0,0,63,0,2,40,0,0,0,0,0,40,1,80,60,0,1,76,20,3,1},
{0,62,24,40,44,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,0,0,20,0,2,64,36,1,1},
};
static void sm_classic_preset_loaded(track_t *t,uint32_t preset)
{
 const int8_t *p=0;
 switch(t->eng_req){
 case ENGI_SM_ANALOG:p=SMC_ANALOG_PARAM[preset%NELEM(SMC_ANALOG_PARAM)];break;
 case ENGI_SM_DIGITAL:p=SMC_DIGITAL_PARAM[preset%NELEM(SMC_DIGITAL_PARAM)];break;
 case ENGI_SM_VOICE:p=SMC_FORMANT_PARAM[preset%NELEM(SMC_FORMANT_PARAM)];break;
 case ENGI_SM_STRINGS:p=SMC_STRINGS_PARAM[preset%NELEM(SMC_STRINGS_PARAM)];break;
 case ENGI_SM_JUNO:p=SMC_JUNO_PARAM[preset%NELEM(SMC_JUNO_PARAM)];break;
 }
 if(p)for(uint32_t i=0;i<NELEM(SMC_PARAM);i++)t->p[SMC_PARAM[i]]=p[i];
}
