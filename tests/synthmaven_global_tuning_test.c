/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void){
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir="build/host";palette_set(0);
 program_t global,saved,legacy;program_packed_t packed;
 tune_cfg.sys=TU_RAST;tune_cfg.root=5;tune_cfg.a4=-8;tune_floor_octave=3;custom_scale[0]=0x123456;
 program_save_tuning=0;program_capture(&global);
 check(program_valid(&global)&&!program_has_tuning(&global),"new programs follow global tuning by default, even when captured in Rast");
 tune_cfg.root=7;tune_floor_octave=4;custom_scale[0]=0x654321;program_apply(&global);
 check(tune_cfg.sys==TU_RAST&&tune_cfg.root==7&&tune_cfg.a4==-8&&tune_floor_octave==4&&custom_scale[0]==0x654321,"normal program change preserves scale, root, A4, octave and custom scales");
 program_save_tuning=1;program_capture(&saved);check(program_pack(&packed,&saved,"SAVED TUNE"),"explicit saved tuning packs without changing storage size");
 tune_cfg.sys=TU_EQUAL;tune_cfg.root=0;tune_cfg.a4=0;tune_floor_octave=0;custom_scale[0]=0;
 check(program_unpack(&saved,&packed),"explicit saved tuning unpacks");program_apply(&saved);
 check(program_save_tuning&&tune_cfg.sys==TU_RAST&&tune_cfg.root==7&&tune_cfg.a4==-8&&tune_floor_octave==4&&custom_scale[0]==0x654321,"saved program override restores complete tuning");
 legacy=saved;legacy.reserved[0]&=15;legacy.sum=program_hash(&legacy,sizeof legacy-4);
 check(program_valid(&legacy)&&program_has_tuning(&legacy),"legacy non-default tuning remains a saved override");
 legacy.tuning.sys=TU_EQUAL;legacy.tuning.a4=0;legacy.reserved[1]&=15;legacy.sum=program_hash(&legacy,sizeof legacy-4);program_apply(&legacy);
 check(!program_save_tuning&&tune_cfg.sys==TU_RAST&&tune_cfg.root==7,"legacy equal-tempered factory programs preserve current global tuning");
 program_save_tuning=1;tune_cfg.sys=TU_EQUAL;tune_cfg.a4=0;tune_floor_octave=0;program_capture(&saved);tune_cfg.sys=TU_RAST;program_apply(&saved);
 check(tune_cfg.sys==TU_EQUAL&&program_save_tuning,"explicit saved equal temperament also overrides a maqam");
 saved.reserved[0]|=0x30;saved.sum=program_hash(&saved,sizeof saved-4);check(!program_valid(&saved),"contradictory scope bits rejected");
 int16_t *v;const param_desc_t *d=syn_desc(X_TUSAVE,&v);*v=0;syn_changed(X_TUSAVE,v);check(!program_save_tuning&&d->max==1,"TUNING 2 SAVE knob controls opt-in");
 for(uint32_t e=0;e<NENGINES;e++){
  set_engine(e);frames(3);d=syn_desc(X_LCUT,&v);check(d&&v,"LFO cutoff descriptor exists for every engine");
  d=syn_desc(X_LDEPTH,&v);check(d->max==(SM_FILTER_CAP[e].cut<0?0:63),"filterless engine disables filter depth");
 }
 set_engine(0);frames(3);d=syn_desc(X_LCUT,&v);*v=50;syn_changed(X_LCUT,v);check(TSEL->p[P_E4]==50,"LFO filter base edits actual engine cutoff");
 d=syn_desc(X_LDEPTH,&v);*v=-20;syn_changed(X_LDEPTH,v);check(TSEL->p[P_LD_FLT]==-20,"LFO depth remains bipolar and preserves existing DSP");
 open_family(FAM_LFO);for(uint32_t i=0;i<NPAGES;i++)if(PAGES[i].scope==SC_SYN&&PAGES[i].id[0]==X_LCUT){ui.page=i;page_entered();break;}frames(3);ppm("lfo-filter-sweep");
 puts(fails?"Global tuning / LFO filter FAIL":"Global tuning / LFO filter PASS");return fails?1:0;
}
