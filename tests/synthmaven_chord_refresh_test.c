/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void){
 bank_resolve();palette_set(0);panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir="build/host";song.sel=0;program_mode=1;program_layer_set(0);go_home();
 uint32_t env_start;press(B_ENV);env_start=fm1_ms-16u;
 fm1_ms=env_start+2999u;frame();check(ui.home,"ENV does not open chord screen before three seconds");
 fm1_ms=env_start+3000u;frame();check(!ui.home && cur_fam()==FAM_CHORD,"ENV opens dedicated CHORDS at three seconds");
 uint32_t chord_page=ui.page;release(B_ENV);frames(3);check(ui.page==chord_page && cur_fam()==FAM_CHORD,"CHORDS persists after ENV release without envelope tap");
 check(cur_page()->id[0]==P_CHORD && cur_page()->id[1]==P_VLEAD && cur_page()->id[2]==P_STRUM,"CHORDS groups mode voicing and strum");
 tap(B_ENV);check(cur_fam()==FAM_ENV,"short ENV press still opens envelopes");go_home();
 track_t *t=TSEL;t->p[P_VOICE]=V_POLY;t->p[P_TRANS]=0;tune_cfg.sys=TU_EQUAL;t->p[P_SCALE]=1;t->p[P_ROOT]=0;
 check(TP[P_CHORD].max==17 && !strcmp(N_CHORD[1],"TRIAD") && !strcmp(N_CHORD[5],"POWER"),"chord modes append without changing six saved legacy IDs");
 check(TP[P_VLEAD].max==5 && !strcmp(N_VOICING[1],"SMOOTH"),"legacy voice-leading ON remains smooth voicing");
 uint32_t readable=1;
 for(uint32_t palid=0;palid<NPALETTES;palid++) {palette_set(palid);if(C_DIM==C_BLACK || C_GRAY==C_BLACK)readable=0;}palette_set(0);
 check(readable,"all five palettes retain visible labels and units");
 uint8_t n[4];t->p[P_CHORD]=6;uint32_t count=chord_play_notes(t,60,0,n);check(count==3 && n[0]==60 && n[1]==64 && n[2]==67,"fixed major chord");
 t->p[P_CHORD]=7;count=chord_play_notes(t,60,0,n);check(count==3&&n[1]==64,"named chord mode follows selected major scale");
 t->p[P_CHORD]=6;t->p[P_VOICE]=V_MONO;t->p[P_VLEAD]=VC_SMOOTH;vl_n[0]=1;vl_prev[0][0]=84;count=chord_play_notes(t,60,0,n);check(count==1 && n[0]==60,"new fixed chords preserve the mono root instead of voice-leading it to another octave");t->p[P_VOICE]=V_POLY;
 uint32_t every_chord_in_scale=1,every_key=1;
 for(uint32_t sc=0;sc<NSCALES;sc++)for(uint32_t root=0;root<12;root++){
  t->p[P_SCALE]=(int16_t)sc;t->p[P_ROOT]=(int16_t)root;
  uint32_t allowed=chord_auto_rotate(scale_mask(t),(int32_t)root);
  for(uint32_t mode=1;mode<18;mode++){
   t->p[P_CHORD]=(int16_t)mode;
   for(uint32_t input=0;input<128;input++){
    uint32_t q=chord_notes(t,input,n);if(!q || q>4)every_chord_in_scale=0;
    for(uint32_t j=0;j<q;j++)if(n[j]>127 || !(allowed&(1u<<(n[j]%12))))every_chord_in_scale=0;
   }
  }
 }
 t->p[P_SCALE]=1;t->p[P_ROOT]=2;t->p[P_CHORD]=1;
 for(uint32_t quant=0;quant<4;quant++){t->p[P_QUANT]=(int16_t)quant;for(uint32_t key=0;key<27;key++)if(kb_pitch(t,key,0)==KB_SILENT)every_key=0;}
 check(every_key,"chord mode enables all physical keys, including black keys, in every quantize setting");
 check(every_chord_in_scale,"all chord modes follow all 16 scales at all roots across the full MIDI range");
 t->p[P_QUANT]=0;t->p[P_ROOT]=0;t->p[P_SCALE]=1;
 uint32_t valid=1,top=1,preview=1;
 for(uint32_t scale=0;scale<NSCALES;scale++)for(uint32_t root=0;root<12;root++)for(uint32_t voic=0;voic<6;voic++){
  t->p[P_SCALE]=(int16_t)scale;t->p[P_ROOT]=(int16_t)root;t->p[P_VLEAD]=(int16_t)voic;
  for(uint32_t key=0;key<128;key++){
   t->p[P_CHORD]=CH_AUTO;count=chord_notes(t,key,n);uint32_t allowed=chord_auto_rotate(scale_mask(t),(int32_t)root);
   if(!count||count>4)valid=0;
   for(uint32_t i=0;i<count;i++)if(n[i]>127 || !(allowed&(1u<<(n[i]%12))))valid=0;
   chord_auto_t before=chord_auto[0];chord_notes(t,key,n);if(memcmp(&before,&chord_auto[0],sizeof before))preview=0;
   t->p[P_CHORD]=CH_MELODY;count=chord_notes(t,key,n);int32_t mapped=chord_melody_mapped(t,key);
   if(!count||count>4||n[count-1]!=(uint32_t)mapped)top=0;
   for(uint32_t i=0;i<count;i++)if(n[i]>127 || !(allowed&(1u<<(n[i]%12))))valid=0;
  }
 }
 check(valid,"AUTO/AUTO2: 16 scales, 12 roots, six voicings and all MIDI notes remain in scale/range");check(top,"AUTO2 preserves mapped melody as highest note even at MIDI endpoints");check(preview,"chord preview never commits automatic harmony history");
 uint32_t maqam_ok=1,options_ok=1;
 for(uint32_t sys=TU_RAST;sys<TU_CUSTOM1;sys++)if(tune_is_maqam(sys))for(uint32_t root=0;root<12;root++){
  tune_cfg.sys=(uint8_t)sys;tune_cfg.root=(uint8_t)root;tune_build();
  for(uint32_t mode=16;mode<=17;mode++){
   t->p[P_CHORD]=(int16_t)mode;uint32_t allowed=chord_allowed(t);
   for(uint32_t input=0;input<128;input++){uint32_t q=chord_play_notes(t,input,0,n);if(!q || q>4)maqam_ok=0;for(uint32_t j=0;j<q;j++)if(!(allowed&(1u<<(n[j]%12u))))maqam_ok=0;}
  }
  if(sm_chord_micro(t) && (chord_mode_allowed(t,6) || !chord_mode_allowed(t,16) || !chord_mode_allowed(t,17)))options_ok=0;
 }
 check(maqam_ok,"both neutral modes stay inside all 46 maqam degree maps at all roots and MIDI notes");
 check(options_ok,"microtonal tunings expose both neutral choices and hide conflicting templates");
 tune_cfg.sys=TU_RAST;tune_cfg.root=0;tune_build();t->p[P_CHORD]=CH_NEUTRAL;t->p[P_VLEAD]=VC_CLOSE;
 uint32_t rn=chord_play_notes(t,60,0,n);check(rn==3 && n[0]==60 && n[1]==64 && n[2]==67 && 4*4096+tun_tab[64]-tun_tab[60]==14336,"Rast neutral chord sounds the stored 350-cent E and 700-cent G");
 t->p[P_CHORD]=0;check(chord_mode_step(t,1)==16,"microtonal knob skips conflicting chord templates");t->p[P_CHORD]=17;check(chord_mode_step(t,-1)==16,"both neutral modes selectable in reverse");
 tune_cfg.sys=TU_19;tune_build();t->p[P_SCALE]=0;t->p[P_CHORD]=CH_OPEN5;rn=chord_notes(t,60,n);check(rn==3 && n[1]==71 && n[2]==79,"19 EDO open chord selects a tuned fifth and 19-key octave");

 tune_cfg.sys=TU_EQUAL;tune_cfg.root=0;tune_build();t->p[P_CHORD]=16;
 check(chord_mode_allowed(t,6) && chord_mode_allowed(t,17),"equal temperament restores all chord choices");
 tune_cfg.sys=TU_RAST;tune_build();master_tuning_bypass|=1u<<trk_index(t);
 check(!sm_chord_micro(t) && chord_mode_allowed(t,6),"per-part master-tuning bypass restores all choices");master_tuning_bypass&=~(1u<<trk_index(t));
 t->p[P_SCALE]=1;t->p[P_ROOT]=0;t->p[P_CHORD]=CH_AUTO;tune_cfg.sys=TU_RAST;tune_cfg.root=0;count=chord_notes(t,60,n);check(count>=2&&n[0]==60,"incompatible saved AUTO safely becomes a neutral open chord under microtuning");tune_cfg.sys=TU_EQUAL;
 t->p[P_SCALE]=1;t->p[P_ROOT]=0;t->p[P_VLEAD]=0;t->p[P_CHORD]=6;t->p[P_STRUM]=0;
 check(sm_mch_event(t,0,0,60,100),"MIDI generates owned chord");check(sm_mch_refs[0][64]==1,"MIDI owns third");sm_mch_event(t,0,0,64,100);check(sm_mch_refs[0][64]==2,"overlapping MIDI chords share tone ownership");
 t->p[P_CHORD]=0;t->p[P_ROOT]=8;sm_mch_event(t,0,0,60,0);check(sm_mch_refs[0][64]==1 && !sm_mch_refs[0][60],"release uses captured notes after chord/root changes");sm_mch_event(t,0,0,64,0);uint32_t clear=1;for(uint32_t i=0;i<128;i++)if(sm_mch_refs[0][i])clear=0;check(clear,"final release leaves no owned MIDI tones");
 t->p[P_CHORD]=CH_MELODY;t->p[P_STRUM]=64;uint8_t chord[3]={48,52,60};trk_note_chord(t,chord,3,2,100);uint32_t queued_top=0;for(uint32_t i=0;i<STQ;i++)if(stq[i].on && stq[i].note==60)queued_top=1;check(!queued_top,"AUTO2 melody starts immediately under a beat strum");
 sm_mch_forget(0);for(uint32_t i=0;i<4;i++){set_engine_of(&trk[i],TRK_DEF[i][0]);apply_preset_to(&trk[i],TRK_DEF[i][1]);}t->p[P_CHORD]=6;t->p[P_STRUM]=0;t->p[P_ROOT]=0;chord_play_notes(t,60,0,n);go_home();ui.force=1;frames(3);ppm("refresh-home");
 open_family(FAM_SCL);scale_browser=0;ui.force=1;frames(3);ppm("refresh-chord");
 scale_browser=1;tune_cfg.sys=TU_RAST;ui.force=1;frames(3);ppm("refresh-scale");
 open_family(FAM_ENV);ui.force=1;frames(3);ppm("refresh-env");
 printf("SynthMaven 2.1 chord and visual refresh: %s\n",fails?"FAIL":"PASS");return fails?1:0;
}
