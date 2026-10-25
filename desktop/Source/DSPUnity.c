/* SPDX-License-Identifier: GPL-3.0-only
 * Desktop adapter for the original SynthMaven DSP. No device HAL is executed.
 * tools/contextualize.py turns mutable firmware storage into instance-owned
 * arenas, including function-local statics. Immutable ROM stays shared. */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "SynthMavenDSP.h"
#define __attribute__(x)
#undef memset
#undef memcpy
#undef memcmp
#define memset felucca_memset
#define memcpy felucca_memcpy
#define memcmp felucca_memcmp
#include "felucca_tables.h"
#include "../../firmware/src/libc.c"
#undef memset
#undef memcpy
#undef memcmp
static struct { volatile uint32_t notes,buttons; } fm1_in;
static void fm1_irq_off(void) {}
static void fm1_irq_on(void) {}
#include "../../firmware/src/core.h"
#include "../../firmware/src/cpu1.h"
static int cpu1_run(cpu1_job_t fn,uint32_t arg){(void)fn;(void)arg;return 0;}
static int cpu1_wait(void){return 0;}
static int cpu1_on(void){return 0;}
static uint32_t cpu1_ticks(void){return 0;}
#include "../../firmware/src/engines.c"
#include "../../firmware/src/drums.c"
#include "../../firmware/src/params.c"
#include "../../firmware/src/voice.c"
#include "../../firmware/src/slicer.c"
#include "../../firmware/src/fx.c"
static void fm1_delay_ms(uint32_t ms){(void)ms;}
#define MQ 256u
static uint32_t midi_in_q[MQ];
static uint32_t mi_w,mi_r;
static void midi_out_event(uint32_t p){(void)p;}
#include "../../firmware/src/seq.c"
static int32_t sm_audio[CTL*2];
static uint32_t sm_cursor=CTL;
static int sm_program;
static void sm_context_initialize(sm_context *ctx); /* generated */
static int sm_find_program(int flat,int *engine,int *preset) {
    int e;for(e=0;e<NENGINES;e++){int n=ENGINES[e]->npresets;if(flat<n){*engine=e;*preset=flat;return 1;}flat-=n;}return 0;
}
int sm_engine_count(void){return NENGINES;}
const char *sm_engine_name(int e){return e>=0&&e<NENGINES?ENGINES[e]->name:"";}
int sm_program_count(void){int n=0,e;for(e=0;e<NENGINES;e++)n+=ENGINES[e]->npresets;return n;}
const char *sm_program_name(int p){int e,i;return sm_find_program(p,&e,&i)?ENGINES[e]->presets[i].name:"";}
int sm_program_engine(int p){int e,i;return sm_find_program(p,&e,&i)?e:0;}
int sm_parameter_count(void){return P_COUNT+G_COUNT;}
sm_parameter sm_parameter_info(int e,int p){
    sm_parameter r={"",0,127,0,0};const param_desc_t *d;
    if(e<0||e>=NENGINES||p<0||p>=P_COUNT+G_COUNT)return r;
    d=p>=P_COUNT?&GP[p-P_COUNT]:p>=P_E0?&ENGINES[e]->edit[p-P_E0]:&TP[p];
    r.name=d->label;r.minimum=d->min;r.maximum=d->max;r.initial=d->def;r.choices=d->fmt==F_ENUM&&d->names!=0;return r;
}
const char *sm_parameter_choice(int e,int p,int v){
    const param_desc_t *d;if(e<0||e>=NENGINES||p<0||p>=P_COUNT+G_COUNT)return "";
    d=p>=P_COUNT?&GP[p-P_COUNT]:p>=P_E0?&ENGINES[e]->edit[p-P_E0]:&TP[p];
    if(e==20&&p==P_E1&&v>=0&&v<=127)return N_WSEQ_RATE[v>>4];
    return d->fmt==F_ENUM&&d->names&&v>=d->min&&v<=d->max?d->names[v-d->min]:"";
}
void sm_panic(sm_context *ctx){SM_ENTER(ctx);panic_req=15;memset(trk[0].v,0,sizeof trk[0].v);trk[0].nheld=0;}
void sm_select_program(sm_context *ctx,int flat){
    SM_ENTER(ctx);int e,pi;if(!sm_find_program(flat,&e,&pi))return;
    const preset_t *p=&ENGINES[e]->presets[pi];uint32_t i;sm_panic(ctx);
    trk[0].engine=trk[0].eng_req=e;trk[0].preset=pi;sm_program=flat;
    for(i=0;i<P_E0;i++)trk[0].p[i]=TP[i].def;
    for(i=0;i<8;i++)trk[0].p[P_E0+i]=p->e[i];
    for(i=0;i<4;i++)trk[0].p[P_ATK+i]=p->env[i];
    trk[0].p[P_ED_FLT]=p->fenv;trk[0].p[P_ED_FX]=preset_trim(e,pi);trk[0].p[P_VOICE]=p->mono?V_LEGATO:V_POLY;
    for(i=0;i<4;i++){trk[0].p[P_DIST+i]=p->fx[i]?p->fx[i]-1:0;trk[0].p[P_AMODE+i]=p->arp[i]?p->arp[i]-1:TP[P_AMODE+i].def;}
    preset_extras(trk[0].p,p);mseq_init(&trk[0]);fm6_track_loaded(&trk[0]);
    hy_slot_seen[0]=-1;wt_have=-1;sm_cursor=CTL;
}
float sm_get_parameter(sm_context *ctx,int p){SM_ENTER(ctx);if(p<0||p>=P_COUNT+G_COUNT)return 0;sm_parameter d=sm_parameter_info(trk[0].eng_req,p);int v=p>=P_COUNT?song.g[p-P_COUNT]:trk[0].p[p];return d.maximum>d.minimum?(float)(v-d.minimum)/(d.maximum-d.minimum):0;}
void sm_set_parameter(sm_context *ctx,int p,float v){SM_ENTER(ctx);if(p<0||p>=P_COUNT+G_COUNT||!isfinite(v))return;sm_parameter d=sm_parameter_info(trk[0].eng_req,p);int n=d.minimum+(int)lroundf(fmaxf(0,fminf(1,v))*(d.maximum-d.minimum));if(p>=P_COUNT){if(p-P_COUNT!=G_ENGSEL)song.g[p-P_COUNT]=n;}else trk[0].p[p]=n;}
void sm_midi(sm_context *ctx,const uint8_t *b,int n){SM_ENTER(ctx);if(!b||n<1||n>3)return;uint32_t status=b[0],cin=status>>4;if(status>=0xF8)cin=15;uint32_t pkt=cin|(status<<8)|(n>1?(uint32_t)b[1]<<16:0)|(n>2?(uint32_t)b[2]<<24:0);if(mi_w-mi_r<MQ){midi_in_q[mi_w%MQ]=pkt;mi_w++;}}
void sm_render(sm_context *ctx,float *out,int n){SM_ENTER(ctx);int i;for(i=0;i<n;i++){if(sm_cursor>=CTL){wt_service();mix_block(sm_audio,CTL);sm_cursor=0;midi_pc_req=0;}out[i*2]=(float)sm_audio[sm_cursor*2]/65536.f;out[i*2+1]=(float)sm_audio[sm_cursor*2+1]/65536.f;sm_cursor++;}}
void sm_tempo(sm_context *ctx,double bpm){SM_ENTER(ctx);if(isfinite(bpm))song.g[G_BPM]=clamp((int)bpm,40,240);}
int sm_control_period(void){return CTL;}
int sm_custom_patch(sm_context *ctx,int kind,const uint8_t *b,int n){SM_ENTER(ctx);if(n!=128)return 0;if(kind==12){hy_patch_t p;memcpy(&p,b,128);hy_set_patch(0,&p);return 1;}if(kind==9){uint8_t v[FP_SIZE+1];fm6_unpack(b,v);fm6_set_patch(0,v);return 1;}return 0;}
static void sm_init_voice(void){uint32_t i,k;for(i=0;i<G_COUNT;i++)song.g[i]=GP[i].def;for(k=0;k<NTRK;k++){for(i=0;i<P_E0;i++)trk[k].p[i]=TP[i].def;steps_clear(&trk[k]);}song.master_q12=4096;song.g[G_BPM]=120;}

double sm_portamento_ms(float normalized){int n=clamp((int)lroundf(normalized*127),0,127);return (double)glide_ticks(n)*CTL*1000.0/FS;}
double sm_voice_pitch(sm_context *ctx){SM_ENTER(ctx);voice_t *v=0;for(int i=0;i<NVOICE;i++)if(trk[0].v[i].active&&(!v||trk[0].v[i].age>v->age))v=&trk[0].v[i];return v?((double)v->pitch_cur+(double)v->pitch_frac/256)/16:-1;}

void sm_set_tuning(sm_context *ctx,const double*cents){SM_ENTER(ctx);int on=0;for(int i=0;i<128;i++){double c=isfinite(cents[i])?fmax(-10000,fmin(10000,cents[i])):0;int32_t v=(int32_t)lround(c*4096/100);tun_tab[i]=v;on|=v!=0;}tun_tab[128]=tun_tab[127];tun_on=on;}
double sm_tuning_offset(sm_context *ctx,int note){SM_ENTER(ctx);return tun_on?tun_tab[clamp(note,0,127)]*100.0/4096:0;}
