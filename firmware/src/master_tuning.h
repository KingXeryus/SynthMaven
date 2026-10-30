/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include "maqam_scales.h"
/* Chromatic key identity is preserved. Offsets are relative to the chosen
 * root, never a degree-address remapping. C0 is MIDI 12; C8 is MIDI 108. */
#define SM_TUNE_CUSTOM 47u
#define SM_TUNE_WIRE 28u
typedef struct {
    uint8_t enabled, scale, root, octave, quantize, apply;
    int8_t cents[12];
    uint16_t mask;
} sm_tuning_t;
static sm_tuning_t sm_tuning = {.apply=15,.mask=0xfff};
#define SM_MAQAM_PROFILE(id,name,profile) profile,
static const uint8_t SM_MAQAM_PROFILES[]={MAQAM_EXISTING(SM_MAQAM_PROFILE) MAQAM_ADDED(SM_MAQAM_PROFILE)};
#undef SM_MAQAM_PROFILE
#define SM_MAQAM_NAME(id,name,profile) name,
static const char *const SM_TUNE_NAMES[]={"CHROMATIC",MAQAM_EXISTING(SM_MAQAM_NAME) MAQAM_ADDED(SM_MAQAM_NAME) "CUSTOM"};
#undef SM_MAQAM_NAME
static int sm_tuning_valid(const sm_tuning_t *t)
{
    if(t->enabled>1 || t->scale>SM_TUNE_CUSTOM || t->root>11 || t->octave>8 || t->quantize>1 || t->apply>15 || !t->mask || t->mask>0xfff)return 0;
    for(uint32_t i=0;i<12;i++)if(t->cents[i]<-100 || t->cents[i]>100)return 0;
    return 1;
}
static uint32_t sm_tuning_mask(const sm_tuning_t *t)
{
    return !t->scale?0xfff:t->scale==SM_TUNE_CUSTOM?t->mask:MAQAM_NOTE_MASKS[t->scale-1u];
}
static int sm_tuning_applies(const sm_tuning_t *t,uint32_t part,uint32_t note)
{
    return t->enabled && part<4u && (t->apply&(1u<<part)) && note>=12u*(t->octave+1u);
}
static int32_t sm_tuning_cents(const sm_tuning_t *t,uint32_t part,uint32_t note)
{
    if(!sm_tuning_applies(t,part,note) || !t->scale)return 0;
    uint32_t key=(note+12u-t->root)%12u;
    if(t->scale==SM_TUNE_CUSTOM)return t->cents[key];
    const uint16_t *m=MAQAM_MASKS[SM_MAQAM_PROFILES[t->scale-1u]];
    return ((m[0]>>key)&1u)*-50+((m[1]>>key)&1u)*50+((m[2]>>key)&1u)*-25;
}
static uint32_t sm_tuning_snap(const sm_tuning_t *t,uint32_t part,uint32_t note)
{
    if(!sm_tuning_applies(t,part,note) || !t->quantize)return note;
    uint32_t mask=sm_tuning_mask(t),low=12u*(t->octave+1u);
    /* Match the existing SNAP convention: downward to an enabled chromatic
     * key, with the octave threshold remaining a hard lower boundary. */
    for(uint32_t n=0;n<12u;n++){
        if((mask>>((note+12u-t->root)%12u))&1u)return note;
        if(note==low)break;
        note--;
    }
    return note;
}
static void sm_tuning_encode(uint8_t *b,const sm_tuning_t *t)
{
    for(uint32_t i=0;i<SM_TUNE_WIRE;i++)b[i]=0;
    b[0]='S';b[1]='M';b[2]='T';b[3]=1;
    b[4]=t->enabled;b[5]=t->scale;b[6]=t->root;b[7]=t->octave;b[8]=t->quantize;b[9]=t->apply;
    for(uint32_t i=0;i<12;i++)b[10+i]=(uint8_t)t->cents[i];
    b[22]=(uint8_t)t->mask;b[23]=(uint8_t)(t->mask>>8);
}
static int sm_tuning_decode(sm_tuning_t *t,const uint8_t *b)
{
    if(b[0]!='S'||b[1]!='M'||b[2]!='T'||b[3]!=1||b[24]||b[25]||b[26]||b[27])return 0;
    sm_tuning_t v={.enabled=b[4],.scale=b[5],.root=b[6],.octave=b[7],.quantize=b[8],.apply=b[9],.mask=(uint16_t)(b[22]|b[23]<<8)};
    for(uint32_t i=0;i<12;i++)v.cents[i]=(int8_t)b[10+i];
    if(!sm_tuning_valid(&v))return 0;
    *t=v;return 1;
}
