/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void){
 bank_resolve();palette_set(0);panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir="build/host";song.sel=0;program_mode=1;program_layer_set(0);go_home();
 track_t *t=TSEL;t->p[P_VOICE]=V_POLY;t->p[P_TRANS]=0;tune_cfg.sys=TU_EQUAL;t->p[P_SCALE]=1;t->p[P_ROOT]=0;
 check(TP[P_CHORD].max==15 && !strcmp(N_CHORD[1],"TRIAD") && !strcmp(N_CHORD[5],"POWER"),"chord modes append without changing six saved legacy IDs");
 check(TP[P_VLEAD].max==5 && !strcmp(N_VOICING[1],"SMOOTH"),"legacy voice-leading ON remains smooth voicing");
 uint32_t readable=1;
 for(uint32_t palid=0;palid<NPALETTES;palid++) {palette_set(palid);if(C_DIM==C_BLACK || C_GRAY==C_BLACK)readable=0;}palette_set(0);
 check(readable,"all five palettes retain visible labels and units");
 uint8_t n[4];t->p[P_CHORD]=6;uint32_t count=chord_play_notes(t,60,0,n);check(count==3 && n[0]==60 && n[1]==64 && n[2]==67,"fixed major chord");
 t->p[P_CHORD]=7;count=chord_play_notes(t,60,0,n);check(count==3&&n[1]==63,"fixed minor chord independent of scale");
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
 t->p[P_CHORD]=CH_AUTO;tune_cfg.sys=TU_RAST;count=chord_notes(t,60,n);check(count==1&&n[0]==60,"automatic harmony bypasses microtuning without changing scale implementation");tune_cfg.sys=TU_EQUAL;
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
