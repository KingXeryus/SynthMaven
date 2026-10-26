/* SPDX-License-Identifier: GPL-3.0-only */
#define PROGRAM_MAGIC 0x32475250u /* PRG2: four-sound routing; PRG1 remains importable */
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
    program_route_t route[4];
    uint32_t sum;
} program_t;
typedef struct {
    uint32_t magic,size;
    uint8_t active,layer,reserved[2];
    tune_t tuning;
    uint32_t custom[4];
    program_sound_t sound[4];
    uint32_t sum;
} program_v1_t;
_Static_assert(sizeof(program_t)<=3840u,"four-sound program fits atomic A/B storage");
static uint32_t program_hash(const void *data,uint32_t n) {
    const uint8_t *p=data;uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;
}
static int program_valid(const program_t *p) {
    uint32_t i,k;
    if(p->magic!=PROGRAM_MAGIC || p->size!=sizeof *p || p->active>=4u || p->layer>1u || p->reserved[0]>15u || p->reserved[1]>15u ||
       p->tuning.sys>=TU_N || p->tuning.root>11 || p->tuning.a4< -40 || p->tuning.a4>40 ||
       p->tuning.n<1 || p->tuning.n>TU_MAXDEG || p->sum!=program_hash(p,sizeof *p-4u))return 0;
    for(i=0;i<4;i++)if(p->custom[i]&0xFF000000u)return 0;
    for(i=0;i<4;i++) {
        const program_route_t *r=&p->route[i];
        const program_sound_t *v=&p->sound[i];
        if(r->enabled>1u || r->split>1u || r->upper>1u || r->point>127u || r->transpose< -24 || r->transpose>24)return 0;
        if(v->engine>=NENGINES || v->user>UP_SLOTS || v->preset>=sm_factory_count(v->engine))return 0;
        for(k=0;k<8u;k++){const param_desc_t *d=&ENGINES[v->engine]->edit[k];if(v->p[P_E0+k]<d->min || v->p[P_E0+k]>d->max)return 0;}
        for(k=0;k<P_E0;k++)if(v->p[k]<TP[k].min || v->p[k]>TP[k].max)return 0;
        for(k=0;k<64;k++)if(((const int8_t *)v->ms)[k]<-8 || ((const int8_t *)v->ms)[k]>7)return 0;
    }
    return 1;
}

/* Upgrade PRG1 in the staging buffer; preserve all sounds, levels, pans and tuning. */
static int program_import(program_t *out,const void *raw,uint32_t n) {
    uint32_t i;
    if(n==sizeof(program_t)) {
        if(!program_valid((const program_t *)raw))return 0;
        if(out!=raw)memcpy(out,raw,n);
        return 1;
    }
    if(n==sizeof(program_v1_t)) {
        const program_v1_t *old=(const program_v1_t *)raw;
        if(old->magic!=0x31475250u || old->size!=sizeof *old || old->sum!=program_hash(old,sizeof *old-4u))return 0;
        if(out!=raw)memcpy(out,raw,sizeof *old-4u);
        out->magic=PROGRAM_MAGIC;out->size=sizeof *out;out->reserved[0]=out->reserved[1]=0;
        for(i=0;i<4;i++){memset(&out->route[i],0,sizeof out->route[i]);out->route[i].enabled=1;out->route[i].point=60;}
        out->sum=program_hash(out,sizeof *out-4u);return program_valid(out);
    }
    return 0;
}

/* Two losslessly packed named programs share each existing atomic A/B object.
 * Parameter ranges fit signed bytes; validate before narrowing. No sample space is reclaimed. */
#define PROGRAM_SLOTS 8u
#define PROGRAM_NAME_LEN 12u
#define PROGRAM_BANK_MAGIC 0x314B4250u /* PBK1 */
typedef struct {int8_t p[P_COUNT];int8_t ms[MSEQ_LANES][MSEQ_STEPS];uint8_t engine,preset,user,reserved;uint8_t patch[156];} program_packed_sound_t;
typedef struct {uint8_t used,active,layer,solo,bypass;char name[13];tune_t tuning;uint32_t custom[4];program_packed_sound_t sound[4];program_route_t route[4];} program_packed_t;
typedef struct {uint32_t magic;program_packed_t slot[2];} program_bank_t;
_Static_assert(sizeof(program_bank_t)<=3840u,"two named programs fit the existing A/B flash object");
static int program_name_valid(const char *name){uint32_t i;for(i=0;i<=12u;i++){uint8_t c=(uint8_t)name[i];if(!c)return 1;if(i==12u || c<32u || c>126u)return 0;}return 0;}
static int program_pack(program_packed_t *out,const program_t *p,const char *name){
 uint32_t i,k;if(!program_valid(p)||!program_name_valid(name))return 0;
 for(i=0;i<4;i++)for(k=0;k<P_COUNT;k++)if(p->sound[i].p[k]<-128 || p->sound[i].p[k]>127)return 0;
 memset(out,0,sizeof *out);out->used=1;out->active=p->active;out->layer=p->layer;out->solo=p->reserved[0];out->bypass=p->reserved[1];for(i=0;i<12u && name[i];i++)out->name[i]=name[i];out->tuning=p->tuning;memcpy(out->custom,p->custom,sizeof out->custom);memcpy(out->route,p->route,sizeof out->route);
 for(i=0;i<4;i++){const program_sound_t *a=&p->sound[i];program_packed_sound_t *b=&out->sound[i];for(k=0;k<P_COUNT;k++)b->p[k]=(int8_t)a->p[k];memcpy(b->ms,a->ms,sizeof b->ms);b->engine=a->engine;b->preset=a->preset;b->user=a->user;b->reserved=a->reserved;memcpy(b->patch,a->patch,sizeof b->patch);}return 1;
}
static int program_unpack(program_t *out,const program_packed_t *p){
 uint32_t i,k;if(p->used!=1u || !program_name_valid(p->name) || p->solo>15u || p->bypass>15u)return 0;
 memset(out,0,sizeof *out);out->magic=PROGRAM_MAGIC;out->size=sizeof *out;out->active=p->active;out->layer=p->layer;out->reserved[0]=p->solo;out->reserved[1]=p->bypass;out->tuning=p->tuning;memcpy(out->custom,p->custom,sizeof out->custom);memcpy(out->route,p->route,sizeof out->route);
 for(i=0;i<4;i++){program_sound_t *a=&out->sound[i];const program_packed_sound_t *b=&p->sound[i];for(k=0;k<P_COUNT;k++)a->p[k]=b->p[k];memcpy(a->ms,b->ms,sizeof a->ms);a->engine=b->engine;a->preset=b->preset;a->user=b->user;a->reserved=b->reserved;memcpy(a->patch,b->patch,sizeof a->patch);}
 out->sum=program_hash(out,sizeof *out-4u);return program_valid(out);
}
static int program_bank_valid(const program_bank_t *b,program_t *scratch){uint32_t i;if(b->magic!=PROGRAM_BANK_MAGIC)return 0;for(i=0;i<2;i++)if(b->slot[i].used>1u || (b->slot[i].used&&!program_unpack(scratch,&b->slot[i])))return 0;return 1;}
