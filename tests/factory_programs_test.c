/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void){
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir="build/host";
 program_t p,q;program_packed_t saved={0};const char *names[]=SM_FACTORY_PROGRAM_NAMES;
 for(uint32_t slot=0;slot<8;slot++){
  check(sm_factory_program(&p,slot)&&program_valid(&p),"factory four-part program validates");
  check(!strcmp(names[slot],SM_FACTORY_PROGRAMS[slot].packed.name),"factory slot has its exact curated name");
  check(p.layer==1 && !p.active && !p.reserved[0] && !p.reserved[1],"four layers enabled, main instrument focused, standard tuning defaults");
  check(sm_program_slot(&q,&saved,slot)&&q.sum==p.sum,"unused slot resolves to its factory program without a flash write");
  for(uint32_t j=0;j<4;j++)check(p.route[j].enabled&&!p.sound[j].p[P_MUTE]&&p.sound[j].p[P_LEVEL]>0,"all four parts are present and audible");
  for(uint32_t j=0;j<4;j++)check(p.sound[j].p[P_VOICE]<V_UNISON,"no hidden unison multiplication in curated programs");
  for(uint32_t j=0;j<4;j++)check(sm_scene_valid(&p.scenes[j])&&p.scenes[j].valid,"four safe scene variants are included");
  check(p.sound[0].p[P_LEVEL]>p.sound[1].p[P_LEVEL]&&p.sound[0].p[P_LEVEL]>p.sound[2].p[P_LEVEL]&&p.sound[0].p[P_LEVEL]>p.sound[3].p[P_LEVEL],"main instrument stays ahead of support levels");
  check(program_pack(&saved,&p,"MY PROGRAM"),"curated program can be saved and renamed");saved.sound[0].p[P_PAN]=17;
  check(sm_program_slot(&q,&saved,slot)&&q.sound[0].p[P_PAN]==17&&!strcmp(sm_program_slot_name(&saved,slot),"MY PROGRAM"),"existing user sound and name override the factory slot");
  saved.sound[0].engine=255;check(!sm_program_slot(&q,&saved,slot),"damaged user data is rejected rather than silently replaced");memset(&saved,0,sizeof saved);
 }
 custom_scale[2]=0x654321u;check(sm_factory_program(&p,0)&&p.custom[2]==0x654321u,"factory loading preserves existing custom-scale definitions");
 check(!sm_factory_program(&p,8),"out-of-range factory slot rejected");
 check(sm_factory_program(&p,0)&&p.sound[0].engine==12&&p.sound[0].preset==51,"P1 preserves the captured HYBRID main instrument");
 check(sm_factory_program(&p,1)&&p.sound[0].engine==1&&p.sound[0].preset==0,"P2 main instrument is electric piano");
 check(sm_factory_program(&p,7)&&p.sound[0].engine==0&&p.sound[0].p[P_VOICE]==V_MONO,"P8 main instrument is mono bass");
 printf("Eight researched instrument-led four-part programs, scenes and non-destructive factory fallback %s\n",fails?"FAIL":"PASS");return fails?1:0;
}
