/* SPDX-License-Identifier: GPL-3.0-only */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
#include <assert.h>
int main(void)
{
 reset();sm_tuning=(sm_tuning_t){.enabled=1,.scale=1,.root=0,.apply=15,.mask=4095};sm_layers=(sm_layers_t){.enabled=1,.high={127,127,127,127}};
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
 puts("SynthMaven programs: eight independent slots, names, scenes, optional global tuning override, sequence preservation, torn-save recovery and legacy protection passed");return 0;
}
