/* SPDX-License-Identifier: GPL-3.0-only */
/* Virtual factory presets append to each engine without duplicating full patch
 * records. Original engine preset indices and existing bank/favourite IDs stay fixed. */
#ifndef SM_ORIENTAL
#define SM_ORIENTAL SYNTHMAVEN_SYNTH
#endif
#if SM_ORIENTAL
#include "oriental_factory.h"
static int sm_or_find(uint32_t engine,uint32_t preset,uint32_t *world,uint32_t *style)
{
    uint32_t n=ENGINES[engine%NENGINES]->npresets,w;
    if(preset<n)return 0;
    preset-=n;
    for(w=0;w<SM_OR_WORLDS;w++)if(SM_OR_WORLD[w].engine==engine){
        if(preset<2u){*world=w;*style=preset;return 1;}preset-=2u;
    }return 0;
}
static uint32_t sm_factory_count(uint32_t engine)
{
    uint32_t n=ENGINES[engine%NENGINES]->npresets,w;
    for(w=0;w<SM_OR_WORLDS;w++)if(SM_OR_WORLD[w].engine==engine)n+=2u;
    return n;
}
static uint32_t sm_or_at(uint32_t index,uint32_t *preset)
{
    uint32_t w=index/2u,n=0,i,e=SM_OR_WORLD[w].engine;
    for(i=0;i<w;i++)n+=SM_OR_WORLD[i].engine==e;
    *preset=ENGINES[e]->npresets+n*2u+index%2u;return e;
}
static uint32_t sm_factory_base(uint32_t engine,uint32_t preset)
{
    uint32_t w,v;return sm_or_find(engine,preset,&w,&v)?SM_OR_WORLD[w].base:preset;
}
static const char *sm_factory_name(uint32_t engine,uint32_t preset)
{
    uint32_t w,v;static char name[13];
    if(!sm_or_find(engine,preset,&w,&v))return ENGINES[engine]->presets[preset%ENGINES[engine]->npresets].name;
    str_cpy(name,SM_OR_NAMES+SM_OR_WORLD[w].name,11);w=str_len(name);name[w++]=' ';name[w++]="DE"[v];name[w]=0;return name;
}
static void sm_or_apply(track_t *t,uint32_t preset)
{
    uint32_t w,v,i;if(!sm_or_find(t->eng_req,preset,&w,&v))return;
    for(i=0;i<sizeof(SM_OR_OP)/sizeof(SM_OR_OP[0]);i++){
        const int8_t *d=SM_OR_DATA+SM_OR_OP[i].offset;uint32_t mode=SM_OR_OP[i].mode;
        uint32_t at=mode==0u?v:mode==1u?w:mode==2u?((((uint8_t)d[4u+w/8u]>>(w%8u))&1u)*2u+v):w*2u+v;
        t->p[SM_OR_OP[i].id]=d[at];
    }
}
#else
#define SM_OR_COUNT 0u
static uint32_t sm_factory_count(uint32_t e){return ENGINES[e%NENGINES]->npresets;}
static uint32_t sm_factory_base(uint32_t e,uint32_t p){(void)e;return p;}
static const char *sm_factory_name(uint32_t e,uint32_t p){return ENGINES[e]->presets[p%ENGINES[e]->npresets].name;}
static void sm_or_apply(track_t *t,uint32_t p){(void)t;(void)p;}
static int sm_or_find(uint32_t e,uint32_t p,uint32_t*w,uint32_t*v){(void)e;(void)p;(void)w;(void)v;return 0;}
static uint32_t sm_or_at(uint32_t i,uint32_t*p){(void)i;*p=0;return 0;}
#endif
