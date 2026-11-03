/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
int main(void){
 ui_power_on();go_home();press(B_SAVE);frames(400);
 assert(cur_page()->scope==SC_SM_PROGRAM);
 turn(EN_SELECT,7);assert(sm_program_slot==8);
 turn(EN_SELECT,1);assert(sm_program_slot==1);
 trk[0].p[P_LEVEL]=73;
 turn(EN_K2,1);assert(sm_name_draft[0]);
 turn(EN_K4,1);assert(sm_io_arm==3);
 turn(EN_K4,1);assert(!sm_io_arm && msg_is("PROGRAM IO ERROR")); /* host UI has no flash */
 trk[0].p[P_LEVEL]=21;
 turn(EN_K3,1);assert(trk[0].p[P_LEVEL]==21);
 turn(EN_K3,1);assert(sm_program_current==0 && msg_is("PROGRAM LOADED"));
 go_home();btn_down(B_EDIT);frame();frames(2100);btn_up(B_EDIT);frame();assert(cur_page()->scope==SC_SM_MIX);
 for(int k=0;k<4;k++)trk[k].p[P_LEVEL]=50;
 for(int k=0;k<4;k++){turn(EN_K1+k,k+1);assert(trk[k].p[P_LEVEL]==51+k);}
 name_close();go_home();ui.home_pg=1;
 uint8_t selected=song.sel;uint8_t engines[4];for(int i=0;i<4;i++)engines[i]=trk[i].eng_req;
 turn(EN_SELECT,1);assert(sm_drum_lane==1&&song.sel==selected);
 turn(EN_K1,3);assert(sm_drum_cursor==3);
 turn(EN_K2,1);assert(sm_drums.steps[1]&(1u<<3));
 turn(EN_K2,-1);assert(!(sm_drums.steps[1]&(1u<<3)));
 press(B_PLAY);frames(100);assert(sm_drums.enabled);
 press(B_PLAY);frames(100);assert(!sm_drums.enabled);
 for(int i=0;i<4;i++)assert(trk[i].eng_req==engines[i]);
 go_home();ui.home_pg=0;sm_program_current=0;int8_t octave=*octave_control();
 for(unsigned direction=0;direction<2;direction++){
  uint32_t label=direction?B_OCTUP:B_OCTDN,bit=1u<<panel.btn[label];
  fm1_in.buttons|=bit;host_pressed|=bit;frame();
  for(unsigned n=0;n<180;n++)frame();assert(sm_program_current==0);assert(*octave_control()==octave);
  for(unsigned n=0;n<12;n++)frame();assert(sm_program_current==(direction?1:7));assert(*octave_control()==octave);
  for(unsigned n=0;n<220;n++)frame();assert(sm_program_current==(direction?1:7));
  fm1_in.buttons&=~bit;frame();assert(*octave_control()==octave);sm_program_current=0;
 }
 /* (layered, a tap moves the selected layer's own octave: its TRN) */
 {int32_t o=*octave_control(),trn=TSEL->p[P_TRANS],layered=layer_octave_on(TSEL);press(B_OCTUP);
  assert(layered?TSEL->p[P_TRANS]==trn+12&&*octave_control()==o:*octave_control()==o+1);press(B_OCTDN);assert(TSEL->p[P_TRANS]==trn&&*octave_control()==o);}
 /* KNOB 4: the drummer's volume while it runs, on both HOME screens; stopped, as before */
 ui.menu=0;ui.home=1;sm_drums.enabled=1;
 for(uint32_t pg=0;pg<2;pg++){int16_t bpm=song.g[G_BPM];ui.home_pg=(uint8_t)pg;sm_drums.volume=90;turn(EN_K4,1);assert(sm_drums.volume==91&&song.g[G_BPM]==bpm);frame();}
 sm_drums.volume=126;turn(EN_K4,1);turn(EN_K4,1);assert(sm_drums.volume==127);
 sm_drums.enabled=0;ui.home_pg=1;sm_drums.volume=90;{int16_t bpm=song.g[G_BPM];turn(EN_K4,1);assert(sm_drums.volume==90&&song.g[G_BPM]==bpm+1);}
 puts("Program UI: SAVE opens programs, SELECT cycles eight slots, legacy named save/load and four independent mixer knobs passed");return 0;
}
