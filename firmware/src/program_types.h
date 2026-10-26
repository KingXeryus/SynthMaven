/* SPDX-License-Identifier: GPL-3.0-only */
#define PROGRAM_MAGIC 0x31475250u /* PRG1: distinct from legacy FUN projects */
typedef struct {
    int16_t p[P_COUNT];
    int8_t ms[MSEQ_LANES][MSEQ_STEPS];
    uint8_t engine,preset,user,reserved;
    uint8_t patch[156]; /* HYBRID/FMX 128 bytes, FM6 156 bytes */
} program_sound_t;
typedef struct {
    uint32_t magic,size;
    uint8_t active,layer,reserved[2];
    tune_t tuning;
    uint32_t custom[4];
    program_sound_t sound[4];
    uint32_t sum;
} program_t;
_Static_assert(sizeof(program_t)<=3840u,"four-sound program fits atomic A/B storage");
static uint32_t program_hash(const void *data,uint32_t n) {
    const uint8_t *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;
}
static int program_valid(const program_t *p) {
    uint32_t i,k;
    if(p->magic!=PROGRAM_MAGIC || p->size!=sizeof *p || p->active>=4u || p->layer>1u ||
       p->tuning.sys>=TU_N || p->tuning.root>11 || p->tuning.a4< -40 || p->tuning.a4>40 ||
       p->tuning.n<1 || p->tuning.n>TU_MAXDEG || p->sum!=program_hash(p,sizeof *p-4u))return 0;
    for(i=0;i<4;i++)if(p->custom[i]&0xFF000000u)return 0;
    for(i=0;i<4;i++) {
        const program_sound_t *v=&p->sound[i];
        if(v->engine>=NENGINES || v->user>UP_SLOTS || v->preset>=sm_factory_count(v->engine))return 0;
        for(k=0;k<8u;k++){const param_desc_t *d=&ENGINES[v->engine]->edit[k];if(v->p[P_E0+k]<d->min || v->p[P_E0+k]>d->max)return 0;}
        for(k=0;k<P_E0;k++)if(v->p[k]<TP[k].min || v->p[k]>TP[k].max)return 0;
        for(k=0;k<64;k++)if(((const int8_t *)v->ms)[k]<-8 || ((const int8_t *)v->ms)[k]>7)return 0;
    }
    return 1;
}
