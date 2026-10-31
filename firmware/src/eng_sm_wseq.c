/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* WSEQ: wave sequencing (SynthMaven 0.6, after the Korg Wavestation and wavestate: the idea, not any code). Each note
 * plays a sequence of steps, each its own wave, pitch and level, one after the other at the song's tempo, the next
 * one fading in over the end of the one before (XFADE): rhythmic, stepping timbres from a single key, or (long steps,
 * full crossfade) a sound that keeps changing.
 *   SEQ    one of 16 sequences (WSEQ_SEQ below: up to 16 steps of {wave, semitones, level})
 *   RATE   a step's length: 1/32, 1/16T, 1/16, 1/8T, 1/8, 1/4, 1/2, a bar (the song's tempo, MIDI clock too)
 *   XFADE  the share of a step the next one fades in over (0: a cut, 127: the whole step)
 *   LOOP   FWD, BACK, PING (back and forth), ONCE (stops on its last step)
 *   START  the first step (each note starts there)
 *   PITCH  how far the steps' pitches go (64: as the sequence has them, 0: every step at the note, 127: twice)
 *   CUT RES  a 12 dB low-pass after them (keyboard ~40 %)
 * Waves use compact classic oscillators: SINE TRI SAW SQR HALF ABS QUART PULSE SIN8 NOISE.
 * Saw and pulse use BLEP correction; the shaped sine waves retain their characteristic harmonics.
 * Voice state: ph[0] the step playing, ph[1] the next one (from its fade in), ph[2] samples into the step; s[0..1] the
 * SVF, s[2] the step, s[3] the direction (PING) and 0x100 = stopped (ONCE), s[4] NOISE's state, s[5] the level glided
 * (a cut between two levels: no click). */
static const uint32_t sm_wseq_units[8]={FS*60/8,FS*60/6,FS*60/4,FS*60/3,FS*60/2,FS*60,FS*120,FS*240};
/* Compact 3.5 oscillator backend: reuse the classic BLEP primitives instead
 * of retaining FMX's 26 KiB tables after removing that engine. Sequence,
 * timing and crossfade semantics are unchanged; wave spectra differ. */
enum { FXW_SINE,FXW_TRI,FXW_SAW,FXW_SQR,FXW_HALF,FXW_ABS,FXW_QUART,FXW_PULSE,FXW_SIN8,FXW_NOISE };
static int32_t wseq_wave(uint32_t wave,uint32_t ph,uint32_t inc,uint32_t *rnd)
{
    int32_t y;
    switch(wave){
    case FXW_SAW:return smc_osc_saw(ph,inc);
    case FXW_SQR:return smc_osc_pulse(ph,inc,0x80000000u);
    case FXW_PULSE:return smc_osc_pulse(ph,inc,0x40000000u);
    case FXW_TRI:return osc_tri(ph);
    case FXW_HALF:y=sine_i(ph);return ((y>0?y:0)-10430)*3/2;
    case FXW_ABS:return (sine_i(ph>>1)-20860)*3/2;
    case FXW_QUART:return ((ph<0x40000000u?sine_i(ph<<1):0)-5215)*6/5;
    case FXW_SIN8:return (sine_i(ph)>>12)*4096+2048;
    case FXW_NOISE:*rnd=*rnd*1664525u+1013904223u;return (int32_t)(*rnd>>16)-32768;
    default:return sine_i(ph);
    }
}
#define WSEQ_NSEQ 16u
#define WSEQ_MAXST 16u
typedef struct {
    uint8_t wave;                /* FXW_* (fmx_tables.h) */
    int8_t semi;                 /* -24..24 */
    uint8_t lvl;                 /* 0..15 */
} wseq_step_t;
typedef struct {
    uint8_t n;                   /* steps, 1..16 */
    wseq_step_t st[WSEQ_MAXST];
} wseq_seq_t;
#define S(w, p, l) {FXW_##w, p, l}
static const wseq_seq_t WSEQ_SEQ[WSEQ_NSEQ] = {
    {8, {S(SAW, 0, 15), S(SQR, 12, 12), S(PULSE, 7, 13), S(TRI, 0, 11), S(SAW, 12, 14), S(SQR, 0, 12), S(PULSE, 19, 11), S(TRI, 7, 12)}},
    {8, {S(SAW, 0, 15), S(SAW, 0, 6), S(SQR, 0, 12), S(SAW, 0, 6), S(PULSE, 0, 14), S(SAW, 0, 6), S(SQR, 0, 12), S(HALF, 0, 9)}},
    {6, {S(SINE, 0, 14), S(TRI, 0, 14), S(ABS, 0, 13), S(PULSE, 0, 12), S(SQR, 0, 12), S(SAW, 0, 12)}},
    {4, {S(TRI, 0, 15), S(TRI, 7, 13), S(SINE, 12, 13), S(TRI, 19, 11)}},
    {8, {S(SAW, 0, 15), S(SAW, 3, 13), S(SAW, 7, 13), S(SAW, 10, 12), S(SAW, 12, 14), S(SAW, 10, 12), S(SAW, 7, 13), S(SAW, 3, 13)}},
    {8, {S(SQR, 0, 15), S(SQR, 4, 13), S(SQR, 7, 13), S(SQR, 11, 12), S(SQR, 12, 14), S(SQR, 11, 12), S(SQR, 7, 13), S(SQR, 4, 13)}},
    {6, {S(PULSE, 0, 15), S(PULSE, 12, 12), S(SQR, 0, 14), S(PULSE, 12, 12), S(PULSE, 24, 10), S(SQR, 12, 12)}},
    {8, {S(SAW, 0, 15), S(SAW, 0, 0), S(SAW, 0, 15), S(SAW, 0, 0), S(SAW, 0, 15), S(SAW, 0, 15), S(SAW, 0, 0), S(SAW, 0, 12)}},
    {4, {S(SINE, 0, 15), S(HALF, 12, 12), S(ABS, 19, 10), S(SINE, 24, 9)}},
    {8, {S(SIN8, 0, 15), S(SQR, 12, 11), S(SIN8, 7, 13), S(SQR, 5, 11), S(SIN8, 0, 14), S(SQR, 12, 11), S(SIN8, 3, 13), S(SQR, 7, 11)}},
    {4, {S(SINE, 0, 15), S(ABS, 0, 14), S(HALF, 0, 14), S(TRI, 0, 14)}},
    {6, {S(QUART, 0, 15), S(HALF, 0, 13), S(ABS, 12, 12), S(QUART, 7, 13), S(HALF, 5, 12), S(ABS, 0, 13)}},
    {8, {S(SAW, 0, 15), S(NOISE, 0, 5), S(SQR, 0, 13), S(NOISE, 0, 7), S(SAW, 0, 15), S(NOISE, 0, 4), S(SQR, 0, 13), S(NOISE, 0, 8)}},
    {4, {S(SQR, 0, 15), S(SQR, -12, 14), S(SQR, 0, 13), S(SQR, -5, 13)}},
    {8, {S(SAW, 0, 14), S(SAW, 2, 13), S(SAW, 3, 13), S(SAW, 5, 13), S(SAW, 7, 14), S(SAW, 9, 13), S(SAW, 10, 13), S(SAW, 12, 14)}},
    {4, {S(TRI, 0, 15), S(PULSE, 7, 12), S(SAW, 12, 13), S(PULSE, 7, 12)}},
};
#undef S
static const char *const N_WSEQ[WSEQ_NSEQ] = {"STEPS", "RHYTHM", "SWEEP", "FIFTHS", "MINOR", "MAJOR", "OCTAVE", "GATE",
                                              "BELLS", "8BIT", "MORPH", "VOX", "NOISY", "DUB", "DORIAN", "PING"};
static const char *const N_WSEQ_RATE[] = {"1/32", "16T", "1/16", "8T", "1/8", "1/4", "1/2", "BAR", 0};
static const uint8_t WSEQ_DIV[8] = {3, 5, 2, 4, 1, 0, 6, 7};   /* N_WSEQ_RATE -> div_units's order */
static const char *const N_WSEQ_LOOP[] = {"FWD", "BACK", "PING", "ONCE"};
enum { WL_FWD, WL_BACK, WL_PING, WL_ONCE };
#define WSEQ_STOP 0x100

/* the step after step k of a sequence of n in loop mode lp (dir: PING's, +1 / -1; 0x100 on it: ONCE ran out) */
static int32_t wseq_next(int32_t k, int32_t n, uint32_t lp, int32_t *dir)
{
    switch (lp) {
    case WL_BACK:
        return k ? k - 1 : n - 1;
    case WL_PING:
        if (n < 2)
            return 0;
        if (k + *dir < 0 || k + *dir >= n)
            *dir = -*dir;
        return k + *dir;
    case WL_ONCE:
        return k + 1 < n ? k + 1 : k;
    default:
        return k + 1 < n ? k + 1 : 0;
    }
}

static void wseq_note_on(track_t *t, voice_t *v)
{
    const wseq_seq_t *q = &WSEQ_SEQ[(uint32_t)clamp(t->p[P_E0], 0, (int32_t)WSEQ_NSEQ - 1)];
    v->ph[0] = v->ph[1] = 0;
    v->ph[2] = 0;
    v->s[2] = clamp(t->p[P_E4], 0, 127) * q->n / 128;  /* START: each note from there */
    v->s[3] = 1;
    v->s[5] = q->st[v->s[2]].lvl * 2184;            /* (the level smoothed: from the first step's) */
    if (!v->env && !v->env_out)
        v->s[0] = v->s[1] = 0;
    if (!v->s[4])
        v->s[4] = (int32_t)(0x2545F491u ^ ((uint32_t)v->note * 2654435761u));
}

static void wseq_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    const wseq_seq_t *q = &WSEQ_SEQ[(uint32_t)clamp(p[P_E0], 0, (int32_t)WSEQ_NSEQ - 1)];
    uint32_t lp = (uint32_t)clamp(p[P_E3], 0, 3), i, rate = (uint32_t)clamp(p[P_E1], 0, 127) * 8u / 128u;
    uint32_t len = sm_wseq_units[rate] / (uint32_t)(song.g[G_BPM] > 0 ? song.g[G_BPM] : 120);   /* samples a step */
    uint32_t xf, xinv = 0, pos = v->ph[2], into;
    int32_t k = v->s[2], dir = v->s[3] & 0x80 ? -1 : 1, stopped = (v->s[3] & WSEQ_STOP) != 0;
    int32_t pscale = clamp(p[P_E5], 0, 127), ic1 = v->s[0], ic2 = v->s[1], nk, gs = v->s[5];
    uint32_t pa = v->ph[0], pb = v->ph[1], ia, ib, rnd = (uint32_t)v->s[4];
    tsvf_t flt;
    if (len < 64u)
        len = 64u;
    xf = len * (uint32_t)clamp(p[P_E2], 0, 127) / 128u;
    if (xf)
        xinv = (256u << 16) / xf;                    /* (the fade's position: one multiply a sample) */
    into = len - xf;
    if (k >= q->n || k < 0)
        k = 0;
    {
        int32_t d2 = dir;
        nk = stopped ? k : wseq_next(k, q->n, lp, &d2);
    }
    /* the two steps' increments (the step's pitch scaled by PITCH: 64 = as written) */
    ia = hy_inc(m->pitch16 + q->st[k].semi * 16 * pscale / 64, 0, m->fine);
    ib = hy_inc(m->pitch16 + q->st[nk].semi * 16 * pscale / 64, 0, m->fine);
    smc_tsvf_coef(&flt, (p[P_E6] << 8) + m->cutoff + ((v->pitch16 - 60 * 16) << 3), p[P_E7]);
    for (i = 0; i < n; i++) {
        int32_t ya = wseq_wave(q->st[k].wave, pa, ia, &rnd), y, x;
        int32_t ga = q->st[k].lvl * 2184;             /* (15 x 2184 = 32760) */
        if (xf && pos >= into && !stopped) {         /* the next step fading in */
            int32_t f = (int32_t)(((pos - into) * xinv) >> 16), gb = q->st[nk].lvl * 2184;   /* 0..255 */
            int32_t yb = wseq_wave(q->st[nk].wave, pb, ib, &rnd);
            f = f > 256 ? 256 : f;
            y = ((ya * ((gs * (256 - f)) >> 8)) >> 15) + ((yb * ((gb * f) >> 8)) >> 15);
            pb += ib;
        } else {
            gs += (ga - gs) >> 5;                    /* (a cut: the level glides ~0.7 ms, no click) */
            y = (ya * gs) >> 15;
        }
        pa += ia;
        if (++pos >= len) {                          /* the next step: its phase from its fade in */
            pos = 0;
            if (!stopped) {
                int32_t d2 = dir, k2 = wseq_next(k, q->n, lp, &d2);
                if (lp == WL_ONCE && k2 == k) {
                    stopped = 1;
                } else {
                    dir = d2;
                    k = k2;
                    if (xf) {
                        pa = pb;
                        gs = q->st[k].lvl * 2184;    /* (faded in: at its level) */
                    }
                    pb = 0;
                    ia = ib;
                    {
                        int32_t d3 = dir;
                        nk = wseq_next(k, q->n, lp, &d3);
                    }
                    ib = hy_inc(m->pitch16 + q->st[nk].semi * 16 * pscale / 64, 0, m->fine);
                }
            }
        }
        x = tsvf_lp(&flt, y * 3 >> 2, &ic1, &ic2);
        if (x > 16000 || x < -16000)
            x = soft_knee(x, 16000);
        out[i] += mulq15(mulq15(x << 1, amp_at(m, i)), VOICE_FS) << 1;
    }
    v->ph[0] = pa;
    v->ph[1] = pb;
    v->ph[2] = pos;
    v->s[0] = ic1;
    v->s[1] = ic2;
    v->s[2] = k;
    v->s[3] = (dir < 0 ? 0x80 : 0x01) | (stopped ? WSEQ_STOP : 0);
    v->s[4] = (int32_t)rnd;
    v->s[5] = gs;
}


static const preset_t WSEQ_PRESETS[] PRESET_TABLE ={{"WAVE STEPS",{0, 40, 20, WL_FWD, 0, 64, 110, 10},{0, 90, 110, 50},0,0},
{"RHYTHM SEQ",{1, 40, 10, WL_FWD, 0, 64, 104, 20},{0, 90, 120, 40},0,0},
{"SWEEP PAD",{2, 110, 127, WL_PING, 0, 64, 100, 10},{80, 100, 120, 100},0,0},
{"FIFTH STAB",{3, 40, 0, WL_FWD, 0, 64, 112, 10},{0, 80, 100, 40},0,0},
{"MINOR SEQ",{4, 40, 0, WL_FWD, 0, 64, 100, 30},{0, 90, 110, 40},10,1},
{"MAJOR SEQ",{5, 40, 0, WL_FWD, 0, 64, 104, 20},{0, 90, 110, 40},0,0},
{"OCTAVE BASS",{6, 40, 0, WL_FWD, 0, 64, 84, 30},{0, 80, 110, 20},10,1},
{"TRANCE GATE",{7, 40, 8, WL_FWD, 0, 64, 110, 10},{20, 90, 120, 60},0,0},
{"BELL STEPS",{8, 40, 20, WL_FWD, 0, 64, 120, 0},{0, 90, 90, 70},0,0},
{"8BIT SEQ",{9, 24, 0, WL_FWD, 0, 64, 127, 0},{0, 80, 110, 30},0,1},
{"MORPH PAD",{10, 127, 127, WL_PING, 0, 64, 96, 10},{90, 100, 120, 110},0,0},
{"VOX STEPS",{11, 56, 60, WL_FWD, 0, 64, 104, 20},{10, 90, 110, 60},0,0},
{"NOISY SEQ",{12, 40, 0, WL_FWD, 0, 64, 110, 10},{0, 80, 110, 30},0,0},
{"DUB STEPS",{13, 56, 30, WL_FWD, 0, 64, 90, 40},{0, 80, 110, 30},10,1},
{"DORIAN RUN",{14, 24, 0, WL_PING, 0, 64, 108, 10},{0, 80, 110, 30},0,1},
{"PING PONG",{15, 40, 30, WL_PING, 0, 64, 110, 10},{0, 90, 110, 50},0,0}};
static const engine_t ENG_WSEQ={.pool=1,.name="WSEQ",.page_title={"SEQUENCE", "TONE"},.edit={
        {"SEQ", F_ENUM, 0, WSEQ_NSEQ - 1, 0, N_WSEQ, 0},
        {"RATE", F_INT, 0, 127, 40, N_WSEQ_RATE, 0},
        {"XFADE", F_PCT, 0, 127, 20, 0, 0},
        {"LOOP", F_ENUM, 0, 3, 0, N_WSEQ_LOOP, 0},
        {"START", F_INT, 0, 127, 0, 0, 0},
        {"PITCH", F_PCT, 0, 127, 64, 0, 0},
        {"CUT", F_CUTOFF, 0, 127, 110, 0, 0},
        {"RES", F_PCT, 0, 127, 10, 0, 0},
    },.presets=WSEQ_PRESETS,.npresets=16,.knob={P_E0,P_E1,P_E2,P_E3},.poly=4,.note_on=wseq_note_on,.render=wseq_render,};
