/* SPDX-License-Identifier: GPL-3.0-only
 * Patch layout retained from SynthMaven eng_hybrid.c:
 * Copyright (C) 2026 3dSam, after Leo Kuroshita (@kurogedelic), Hügelton Instruments (Felucca) */
#pragma once
#include <stdint.h>
#define HY_FACTORY_PRESETS 105u
#define HY_NOSC 3u
#define HY_NSLOT 16u
#define HY_SUPER 7u

enum { HO_OFF, HO_SAW, HO_PULSE, HO_TRI, HO_SINE, HO_MORPH, HO_SUPER, HO_FM, HO_SYNC, HO_FOLD, HO_NOISE, HO_N };
enum { HF_OFF, HF_LP12, HF_LP24, HF_BP, HF_HP, HF_NOTCH, HF_PEAK, HF_MOOG, HF_N };   /* (MOOG: SynthMaven 0.2) */
enum { HR_SER, HR_PAR, HR_SPLIT, HR_N };
enum { HW_SINE, HW_TRI, HW_SAW, HW_SQR, HW_SH, HW_SMOOTH, HW_N };   /* the voices' LFO waves */
/* matrix sources: Q15, bipolar (LFOs, KEY, BEND, RAND) or unipolar (the rest) */
enum {
    HYS_NONE, HYS_ENV2, HYS_ENV3, HYS_AENV, HYS_LFO1, HYS_LFO2, HYS_LFO3, HYS_VEL, HYS_KEY, HYS_MW, HYS_AT, HYS_BEND,
    HYS_EXPR, HYS_RAND, HYS_M1, HYS_M2, HYS_M3, HYS_M4, HYS_ONE, HYS_N
};
/* matrix destinations */
enum {
    HD_NONE, HD_PITCH, HD_P1, HD_P2, HD_P3, HD_SH1, HD_SH2, HD_SH3, HD_LV1, HD_LV2, HD_LV3, HD_RING,
    HD_CUT, HD_CUT1, HD_CUT2, HD_RES1, HD_RES2, HD_DRIVE, HD_AMP, HD_LR2, HD_LR3, HD_FBAL, HD_N
};
#define HL_KEY 1u                /* LFO flags: restart at each note */
#define HL_SYNC 2u               /*            RATE in note values at the song tempo (HY_SYNC_DIV) */
#define HOF_RESET 1u             /* oscillator flags: phase 0 at each note (else free: phases kept / random) */
#define HOF_FIXED 2u             /*                   no keyboard tracking: the pitch of C4 on every key */

typedef struct {
    uint8_t type;
    int8_t coarse;               /* semitones -48..48 */
    int8_t fine;                 /* cents -64..63 */
    uint8_t level;               /* 0..127 */
    uint8_t shape;               /* 0..127 */
    uint8_t flags;               /* HOF_* */
} hy_osc_t;
typedef struct {
    uint8_t type;                /* HF_* */
    uint8_t cut;                 /* 0..127 (30 Hz .. 16 kHz) */
    uint8_t res;                 /* 0..127 */
    int8_t env;                  /* ENV2 -> cutoff, -64..63 */
    int8_t key;                  /* keyboard tracking, 64 = 100 % (an octave up opens it an octave) */
    uint8_t vel;                 /* velocity -> cutoff, 0..127 */
} hy_flt_t;
typedef struct {
    uint8_t wave, rate, flags, fade;   /* HW_*, 0..127 (or a HY_SYNC_DIV index), HL_*, fade-in 0..127 */
} hy_lfo_t;
typedef struct {
    uint8_t src, via, dst;       /* HYS_*, HYS_* (0 = none: the amount alone), HD_* */
    int8_t amt;                  /* -64..63 */
} hy_slot_t;
typedef struct {
    hy_osc_t osc[HY_NOSC];
    uint8_t ring;                /* osc 1 x osc 2, 0..127 */
    uint8_t drift;               /* analog drift of the pitch, 0..127 */
    uint8_t drive;               /* before the filters, 0..127 */
    uint8_t route;               /* HR_* */
    uint8_t fbal;                /* PAR: F1 0 .. 127 F2 */
    hy_flt_t flt[2];
    uint8_t env2[4], env3[4];    /* A D S R, the ADSR's scales (voice.c env_tick) */
    hy_lfo_t lfo[2];             /* LFO 2, LFO 3 (LFO 1 is the part's) */
    hy_slot_t mod[HY_NSLOT];
    uint8_t bend;                /* pitch bend range, semitones 0..24 */
    uint8_t level;               /* 0..127, 100 = unity */
    uint8_t rsv;                 /* 0 (room for later) */
    char name[10];
} hy_patch_t;
#define HY_PATCH_SIZE 128u
_Static_assert(sizeof(hy_patch_t) == HY_PATCH_SIZE, "hy_patch_t is the editor's byte layout");

static int hy_patch_valid(const hy_patch_t *p)
{
    for (uint32_t i=0;i<HY_NOSC;i++) {
        const hy_osc_t *o=&p->osc[i];
        if(o->type>=HO_N || o->coarse < -48 || o->coarse > 48 || o->fine < -64 || o->fine > 63 ||
           o->level>127 || o->shape>127 || o->flags>3) return 0;
    }
    if(p->ring>127 || p->drift>127 || p->drive>127 || p->route>=HR_N || p->fbal>127 || p->bend>24 || p->level>127) return 0;
    for(uint32_t i=0;i<2;i++) {
        const hy_flt_t *f=&p->flt[i]; const hy_lfo_t *l=&p->lfo[i];
        if(f->type>=HF_N || f->cut>127 || f->res>127 || f->env < -64 || f->env>63 ||
           f->key < -64 || f->key>63 || f->vel>127 || l->wave>=HW_N || l->rate>127 || l->flags>3 || l->fade>127) return 0;
    }
    for(uint32_t i=0;i<4;i++)if(p->env2[i]>127 || p->env3[i]>127)return 0;
    for(uint32_t i=0;i<HY_NSLOT;i++) {
        const hy_slot_t *m=&p->mod[i];
        if(m->src>=HYS_N || m->via>=HYS_N || m->dst>=HD_N || m->amt < -64 || m->amt>63)return 0;
    }
    for(uint32_t i=0;i<sizeof p->name;i++)if(p->name[i] && (p->name[i]<32 || p->name[i]>126))return 0;
    return 1;
}
