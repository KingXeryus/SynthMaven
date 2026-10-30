/* SPDX-License-Identifier: GPL-3.0-only */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
#include <assert.h>
int main(void)
{
 reset();
 for(unsigned i=0;i<HY_NFACTORY;i++){hy_patch_t patch;hy_factory_read(i,&patch);assert(!memcmp(&patch,&HY_FACTORY[i],sizeof patch));}
 for(unsigned slot=0;slot<8;slot++){
  assert(!sm_program_load(slot));assert(sm_program_current==slot&&sm_layers.enabled);
  for(unsigned k=0;k<4;k++)assert(!trk[k].p[P_MUTE]&&trk[k].p[P_LEVEL]>0);
 }
 assert(!sm_program_load(0));assert(trk[0].eng_req==ENGI_FM6&&trk[0].preset==35);
 assert(!strcmp(FM6_PRESETS[35].name,"E.PIANO 1"));
 FILE *capture=fopen("assets/factory/programs-3.5.bin","rb");assert(capture);
 for(unsigned slot=0;slot<8;slot++){
  unsigned char record[3556];assert(fread(record,1,sizeof record,capture)==sizeof record);
  assert(!sm_factory_program(slot));
  for(unsigned k=0;k<4;k++){
   const unsigned char *t=record+68+k*250;assert(trk[k].eng_req==t[0]&&trk[k].preset==t[1]);
   for(unsigned i=0;i<P_COUNT;i++)if(i<P_SLEN||i>P_SGATE)assert(trk[k].p[i]==(int16_t)(t[2+i*2]|t[3+i*2]<<8));
   if(t[0]==ENGI_FM6){uint8_t packed[128];fm6_pack(fm6_patch_of(k),packed);assert(!memcmp(packed,record+1068+k*128,128));assert(!memcmp(fm6_fn[k],record+1580+k*16,16));}
   if(t[0]==ENGI_HYBRID)assert(!memcmp(&hy_in[k],record+1068+k*128,128));
   if(t[0]==ENGI_CZ)assert(!memcmp(cz_patch_of(k),record+1644+k*144,144));
   if(t[0]==ENGI_PROPHET)assert(!memcmp(&p5_patch[k],record+2220+k*138,138));
  }
  assert(!memcmp(sm_program_meta,record+3056,500));
  for(unsigned i=0;i<12;i++)assert(song.g[G_MFXTYPE+i]==record[3028+i]);
  assert(song.g[G_FXRETURN]==record[3040]&&song.g[G_FXMODE]==record[3041]);
 }
 fclose(capture);
 sm_tuning=(sm_tuning_t){.enabled=1,.scale=1,.root=0,.apply=15,.mask=4095};sm_layers=(sm_layers_t){.enabled=1,.high={127,127,127,127}};
 for(uint32_t tr=0;tr<4;tr++){set_engine_of(&trk[tr],ENGI_HYBRID);apply_preset_to(&trk[tr],41+tr);}
 for(uint32_t s=0;s<8;s++){char name[13];snprintf(name,sizeof name,"PROGRAM %u",s+1);trk[0].p[P_LEVEL]=(int16_t)(70+s);sm_tuning.root=(uint8_t)s;assert(!sm_program_save(s,name,s==7));}
 for(uint32_t s=0;s<8;s++){sm_tuning.root=11;assert(!sm_program_load(s));assert(trk[0].p[P_LEVEL]==70+(int)s);assert(sm_tuning.root==(s==7?7:11));assert(sm_program_current==s);}
 trk[0].p[P_LEVEL]=39;trk[1].p[P_PAN]=-37;assert(!sm_scene_store(2,"QUIET"));assert(!sm_program_save(7,"SCENES",1));
 trk[0].p[P_LEVEL]=88;trk[1].p[P_PAN]=0;assert(!sm_program_load(7));assert(!sm_scene_load(2));assert(trk[0].p[P_LEVEL]==39 && trk[1].p[P_PAN]==-37);
 step_t step=trk[0].step[0];step.n=67;trk[0].step[0]=step;assert(!sm_program_load(0));assert(trk[0].step[0].n==67);
 for(int cut=0;cut<120;cut+=7){assert(!sm_program_load(0));int old=trk[0].p[P_LEVEL];trk[0].p[P_LEVEL]=99;fail_after=cut;int rc=sm_program_save(0,"REPLACEMENT",0);fail_after=-1;assert(!sm_program_load(0));assert(trk[0].p[P_LEVEL]==(rc?old:99));assert(!sm_program_load(1)&&trk[0].p[P_LEVEL]==71);}
 assert(!sm_program_load(1));sm_program_autosave();uint32_t unchanged_erases=erases;sm_program_autosave();assert(erases==unchanged_erases);
 trk[0].p[P_LEVEL]=68;sm_program_autosave();assert(!sm_program_load(1)&&trk[0].p[P_LEVEL]==68);
 sm_program_current=255;sm_program_name[0]=0;sm_config_load();sm_program_boot();assert(sm_program_current==1&&trk[0].p[P_LEVEL]==68);
 assert(project_save(0));assert(project_erase(0));assert(!sm_program_load(1));
 reset();assert(!project_save(0));assert(sm_program_save(0,"PROTECTED",0)==3);
 reset();sm_drums.enabled=0;sm_drums.steps[0]=0xa55a;sm_drums.level[0]=81;sm_drums.volume=94;
 assert(!sm_config_save());sm_drums.steps[0]=0;sm_drums.level[0]=0;sm_drums.enabled=1;
 sm_config_load();assert(sm_drums.steps[0]==0xa55a&&sm_drums.level[0]==81&&sm_drums.volume==94&&!sm_drums.enabled);
 uint8_t drum_config[SM_CONFIG_BYTES];sm_config_pack(drum_config);assert(sm_config_valid(drum_config));
 drum_config[347]=128;assert(!sm_config_valid(drum_config));
 puts("SynthMaven programs: eight independent slots, names, scenes, optional global tuning override, sequence preservation, torn-save recovery and legacy protection passed");return 0;
}
