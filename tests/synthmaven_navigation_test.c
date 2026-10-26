/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint32_t active_part(uint32_t p) {
 uint32_t i,n=0;for(i=0;i<NVOICE;i++)n+=trk[p].v[i].active && trk[p].v[i].stage!=4u;return n;
}
int main(void) {
 uint32_t i,e;program_t saved;
 outdir="build/host";bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();palette_set(4);go_home();frames(3);
 for(e=0;e<NENGINES;e++) {
  set_engine(e);apply_preset(0);frames(4);
  encs[panel.enc[EN_PRESET]]=-1;frames(2);
  check(TSEL->eng_req==e && TSEL->preset==sm_factory_count(e)-1,"PRESET backwards wraps inside engine");
  encs[panel.enc[EN_PRESET]]=1;frames(2);check(TSEL->preset==0,"PRESET forward wraps inside engine");
  encs[panel.enc[EN_ALGO]]=1;frames(2);check(TSEL->eng_req==(e+1)%NENGINES,"ALGORITHM cycles engines");
 }
 for(i=0;i<4;i++) {
  song.sel=i;set_engine(i);apply_preset(i%sm_factory_count(i));frames(3);
 }
 song.sel=0;encs[panel.enc[EN_SELECT]]=-1;frames(2);check(song.sel==3,"SELECT wraps across four loaded sounds");
 encs[panel.enc[EN_SELECT]]=2;frames(2);check(song.sel==1 && TSEL->eng_req==1,"SELECT focuses loaded slot without replacing it");
 program_layer_set(0);frames(3);trk_note_on(TSEL,60,100);frames(2);
 check(active_part(1)>0 && !active_part(0) && !active_part(2) && !active_part(3),"SOLO plays selected sound only");
 trk_note_off(TSEL,60);trk_all_off(TSEL);frames(8);
 program_layer_set(1);frames(3);trk_note_on(TSEL,64,100);frames(2);
 check(active_part(0)>0 && active_part(1)>0 && active_part(2)>0 && active_part(3)>0,"LAYER triggers all four sounds");
 song.sel=3;trk_note_off(TSEL,64);check(!ly_route[64],"note-off follows ownership after focus changes");
 trk_all_off(TSEL);frames(30);
 program_layer_set(0);song.sel=3;frames(4);TSEL->p[P_AMODE]=1;
 input_on(TSEL,67,100);frames(20);check(active_part(3)>0,"arpeggiator runs on the fourth selected sound");
 input_off(TSEL,67);TSEL->p[P_AMODE]=0;program_layer_set(1);frames(30);
 program_capture(&saved);check(program_valid(&saved),"full four-sound program validates");
 song.sel=0;set_engine(ENGI_HYBRID);apply_preset(0);program_layer_set(0);frames(4);
 program_apply(&saved);frames(8);
 check(song.sel==3 && program_layer && trk[0].eng_req==0 && trk[3].eng_req==3,"program restores all four sounds, focus and layer mode");
 saved.sum^=1;check(!program_valid(&saved),"program corruption rejected");
 go_home();tap(B_SCL);check(tune_cfg.sys==1,"SEL tap cycles tuning scales");
 press(B_SCL);frames(190);release(B_SCL);check(scale_edit && ly_lock==LY_SCALE,"SEL holds three seconds then custom editor stays open");
 tune_cfg.root=0;tune_cfg.sys=TU_CUSTOM1;custom_scale[0]=0;
 for(i=1;i<=4;i++){key(0);check(custom_scale[0]==((i%4u)<<(2u*(53u%12u))),"custom key cycles -25/-50/-75/0 cents");}
 key(0);key(1);key(1);key(2);key(2);key(2);
 ppm("synthmaven-custom-scale");
 tap(B_SAVE);check(scale_edit && custom_scale_dirty,"SAVE stores custom scale without leaving editor");
 tap(B_SCL);check(!scale_edit && ly_lock==LY_PLAY,"SEL exits latched scale editor");
 go_home();frames(110);ppm("synthmaven-four-program-sounds");
 open_family(FAM_SAVE);check(!strcmp(cur_page()->title,"PROGRAM"),"SAVE opens program save/load page");
 return fails?1:0;
}
