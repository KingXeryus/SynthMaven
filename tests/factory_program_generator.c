/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
#include "../firmware/src/factory_programs.h"
#include "factory_program_recipes.h"
static void factory_prepare(uint32_t slot){
 memset(trk,0,sizeof trk);host_tracks_init();for(uint32_t i=0;i<NTRK;i++)track_defaults(&trk[i]);
 program_mode=program_layer=1;song.sel=song.solo=0;program_patch_pending=0;sm_filter_comp_mask=master_tuning_bypass=tune_floor_octave=0;
 memset(sm_scenes,0,sizeof sm_scenes);memset(custom_scale,0,sizeof custom_scale);tune_cfg.sys=TU_EQUAL;tune_cfg.root=0;tune_cfg.a4=0;tune_cfg.n=12;tune_cfg.period=19200;
 for(uint32_t i=0;i<TU_MAXDEG;i++)tune_cfg.deg[i]=(uint16_t)(i*1600u);tune_build();
 track_t *t;for(uint32_t j=0;j<4;j++)for(uint32_t k=0;k<MX_SLOTS;k++){trk[j].p[mx_pid(k)]=MS_OFF;trk[j].p[mx_pid(k)+1]=MT_OFF;trk[j].p[mx_pid(k)+2]=0;}
 factory_recipe(slot);frames(4);
 /* Scene timbres differ; stored level offsets match their reference loudness to MAIN. */
 sm_scene_capture(0);
 for(uint32_t s=1;s<4;s++){sm_scenes[s]=sm_scenes[0];for(uint32_t j=0;j<4;j++){
  sm_scene_part_t *p=&sm_scenes[s].part[j];if(s==1){if(j>=2)p->level=p->level>8?p->level-8:0;p->rev/=2;p->chor/=2;}
  if(s==2&&(j==0||j==1))p->level=p->level>10?p->level-10:0;
  if(s==3)p->dist=p->chor=p->dly=p->rev=0;
  p->level=(uint8_t)clamp((int32_t)p->level+FACTORY_SCENE_LEVEL[slot][s],1,127);
 }}sm_scene_index=255;
}
int main(void){
 bank_resolve();panel=PANEL_DEFAULT;layers_init();outdir="build/host";
 FILE *raw=fopen("build/host/factory-programs.bin","wb"),*json=fopen("build/host/factory-programs.json","w");if(!raw||!json)return 1;
 fprintf(json,"[");
 for(uint32_t i=0;i<8;i++){
  factory_prepare(i);program_t p;program_packed_t packed;program_capture(&p);if(!program_valid(&p)||!program_pack(&packed,&p,"FACTORY"))return 2;
  const char *names[]=FACTORY_RECIPE_NAMES;memset(packed.name,0,13);memcpy(packed.name,names[i],strlen(names[i]));fwrite(&packed,1,sizeof packed,raw);
  fprintf(json,"%s{\"layer\":1,\"selected\":0,\"tuning\":{\"sys\":0,\"root\":0,\"floor\":0,\"a4\":440},\"parts\":[",i?",":"");
  for(uint32_t j=0;j<4;j++){program_sound_t *s=&p.sound[j];program_route_t *r=&p.route[j];fprintf(json,"%s{\"engine\":%u,\"preset\":%u,\"solo_gain_db\":%u,\"p\":[",j?",":"",s->engine,s->preset,(s->reserved>>2)?(s->reserved>>2)-1u:0u);for(uint32_t k=0;k<P_COUNT;k++)fprintf(json,"%s%d",k?",":"",s->p[k]);fprintf(json,"],\"patch\":[");for(uint32_t k=0;k<156;k++)fprintf(json,"%s%u",k?",":"",s->patch[k]);fprintf(json,"],\"route\":[%u,%u,%u,%u,%d]}",r->enabled,r->split,r->upper,r->point,r->transpose);}
  fprintf(json,"],\"scenes\":[");for(uint32_t k=0;k<4;k++){fprintf(json,"%s[",k?",":"");for(uint32_t z=0;z<sizeof(sm_scene_t);z++)fprintf(json,"%s%u",z?",":"",((uint8_t*)&p.scenes[k])[z]);fprintf(json,"]");}fprintf(json,"]}");
 }fprintf(json,"]\n");fclose(json);fclose(raw);return 0;
}
