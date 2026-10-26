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
 go_home();program_layer_set(0);press(B_GLO);
 check(!glo_screen_shown,"GLO short press never flashes the hold screen");
 frames(123);check(!program_layer,"GLO does not toggle before two seconds");frames(2);
 check(program_layer && ly_lock!=LY_MIX && !strcmp(glo_layer_label(),"Layered ON"),"GLO toggles at two seconds and shows Layered ON without latching");ppm("synthmaven-layered-on");
 frames(250);check(program_layer && glo_screen_shown,"GLO toggles only once and keeps its screen while held");release(B_GLO);check(ly_lock==LY_PLAY && !glo_screen_shown,"GLO release restores normal screen without latching");
 uint8_t prior_page=ui.page;press(B_GLO);frames(125);check(!program_layer && !strcmp(glo_layer_label(),"Layered OFF"),"second two-second GLO hold shows Layered OFF");ppm("synthmaven-layered-off");release(B_GLO);check(ly_lock==LY_PLAY && !glo_screen_shown && ui.page==prior_page,"second GLO release returns to the previous page");
 tap(B_GLO);check(!program_layer && !glo_screen_shown,"short GLO tap keeps layering unchanged");
 open_family(FAM_ENV);prior_page=ui.page;press(B_GLO);frames(30);release(B_GLO);check(!ui.home && cur_page()->fam==FAM_GLO && !glo_screen_shown,"short GLO press opens its menu without flashing the hold screen");
 ui.menu=1;ui.menu_sel=2;press(B_GLO);frames(125);check(program_layer && glo_screen_shown && ui.menu==1,"GLO hold replaces a menu and still toggles at two seconds");release(B_GLO);check(!glo_screen_shown && ui.menu==1 && ui.menu_sel==2,"GLO release restores the same menu and selection");ui.menu=0;

 go_home();uint8_t scale_before=tune_cfg.sys;tap(B_SCL);check(scale_browser && tune_cfg.sys==scale_before,"SEL tap opens scale browser without changing tuning");
 encs[panel.enc[EN_SELECT]]=1;frames(2);check(tune_cfg.sys==(scale_before+1u)%TU_N,"SELECT cycles tuning while scale browser stays visible");ppm("synthmaven-scale-browser");
 encs[panel.enc[EN_SELECT]]=-1;frames(2);check(tune_cfg.sys==scale_before,"SELECT browses scales backwards");
 tap(B_SCL);check(scale_browser && tune_cfg.sys==scale_before,"repeated SEL taps never cycle scales");
 press(B_SCL);frames(190);release(B_SCL);check(scale_edit && ly_lock==LY_SCALE,"SEL holds three seconds then custom editor stays open");
 tune_cfg.root=0;tune_cfg.sys=TU_CUSTOM1;custom_scale[0]=0;
 for(i=1;i<=4;i++){key(0);check(custom_scale[0]==((i%4u)<<(2u*(53u%12u))),"custom key cycles -25/-50/-75/0 cents");}
 key(0);key(1);key(1);key(2);key(2);key(2);
 ppm("synthmaven-custom-scale");
 tap(B_SAVE);check(scale_edit && custom_scale_dirty,"SAVE stores custom scale without leaving editor");
 tap(B_SCL);check(!scale_edit && ly_lock==LY_PLAY,"SEL exits latched scale editor");

 go_home();press(B_EDIT);frames(80);check(ui.layer==LY_PLAY && ly_lock==LY_PLAY,"EDIT shows no hold screen before two seconds");frames(45);check(ui.layer==LY_ERASE && ly_lock==LY_PLAY,"EDIT mixer previews at two seconds without latching");
 check(layer_now()==LY_PLAY,"mixer leaves keyboard playing and never erases sequence notes");int16_t lv[4],pan[4];for(i=0;i<4;i++){lv[i]=trk[i].p[P_LEVEL];pan[i]=trk[i].p[P_PAN];encs[panel.enc[EN_K1+i]]=-1;}frames(2);
 for(i=0;i<4;i++)check(trk[i].p[P_LEVEL]==lv[i]-1 && trk[i].p[P_PAN]==pan[i],"four mixer knobs adjust their own sound volume only");ppm("synthmaven-layer-volume");
 encs[panel.enc[EN_SELECT]]=1;frames(2);check(layer_mixer_pan,"mixer SELECT switches volume to pan");for(i=0;i<4;i++)encs[panel.enc[EN_K1+i]]=1;frames(2);
 for(i=0;i<4;i++)check(trk[i].p[P_PAN]==pan[i]+1 && trk[i].p[P_LEVEL]==lv[i]-1,"four mixer knobs adjust their own pan without changing volume");ppm("synthmaven-layer-pan");
 frames(65);release(B_EDIT);check(ly_lock==LY_ERASE && ui.layer==LY_ERASE,"EDIT mixer latches at three seconds and remains after release");
 uint8_t bpm_before=(uint8_t)song.g[G_BPM];encs[panel.enc[EN_SELECT]]=1;frames(2);check(!layer_mixer_pan && song.g[G_BPM]==bpm_before,"latched mixer SELECT toggles back without changing tempo");press(B_SCL);frames(8);check(ly_lock==LY_PLAY && ui.layer==LY_PLAY,"switching from a latched mixer never flashes the next hold screen");release(B_SCL);frames(2);
 layer_mixer_pan=1;press(B_EDIT);frames(190);release(B_EDIT);check(!layer_mixer_pan,"a new EDIT hold always opens the volume mixer");tap(B_EDIT);check(ly_lock==LY_PLAY,"EDIT tap exits latched mixer");
 set_engine(0);open_family(FAM_EDIT);uint32_t edit_pages=0;for(i=0;i<NPAGES;i++)if(PAGES[i].fam==FAM_EDIT&&!page_hidden(i))edit_pages++;check(edit_pages==4,"normal EDIT has four focused sound pages");
 go_home();frames(110);ppm("synthmaven-four-program-sounds");
 open_family(FAM_SAVE);check(!strcmp(cur_page()->title,"PROGRAM"),"SAVE opens program save/load page");
 int16_t old_bpm=song.g[G_BPM];uint8_t save_page=ui.page,old_focus=song.sel;song.g[G_SLOT]=GP[G_SLOT].min;ui.arm=G_SAVE;ui.arm_t=40;
 encs[panel.enc[EN_SELECT]]=-1;frames(2);check(song.g[G_SLOT]==GP[G_SLOT].max && !ui.arm && !ui.arm_t,"SAVE SELECT wraps backwards through slots and cancels pending confirmation");
 encs[panel.enc[EN_SELECT]]=1;frames(2);check(song.g[G_SLOT]==GP[G_SLOT].min && song.g[G_BPM]==old_bpm && ui.page==save_page && song.sel==old_focus && !saves && !loads,"SAVE SELECT wraps forwards without loading, saving, changing focus, page or tempo");

 check(GP[G_SLOT].max==8,"SAVE offers eight program slots");
 for(uint32_t i=0;i<NELEM(PAGES);i++)if(PAGES[i].fam==FAM_SAVE)check(strcmp(PAGES[i].title,"PROGRAM MODE")&&strcmp(PAGES[i].title,"PRESETS")&&strcmp(PAGES[i].title,"USER"),"redundant layer, preset and user pages are absent from SAVE");
 int glo_layer=0;for(uint32_t i=0;i<NELEM(PAGES);i++)if(PAGES[i].fam==FAM_GLO)for(uint32_t k=0;k<4;k++)if(PAGES[i].scope==SC_SYN&&PAGES[i].id[k]==X_PGLAYER)glo_layer=1;check(glo_layer,"GLO menu still provides layering ON/OFF");
 ui.page=(uint8_t)page_first(FAM_SAVE);ui.home=0;page_entered();song.g[G_SLOT]=8;frames(2);uint8_t eng=TSEL->eng_req,preset=TSEL->preset;
 encs[panel.enc[EN_ALGO]]=1;frames(2);check(program_name_cursor==1,"SAVE ALGORITHM selects the name character position");
 encs[panel.enc[EN_PRESET]]=1;frames(2);check(TSEL->eng_req==eng&&TSEL->preset==preset&&strcmp(program_name_draft,program_names[7]),"SAVE PRESET edits the name without replacing the sound");
 encs[panel.enc[EN_K2]]=1;frames(2);check(program_name_cursor==1,"SAVE knob 2 edits the selected character");ppm("synthmaven-eight-program-names");
 uint8_t msg[13]={7,'M','Y',' ','P','R','O','G','R','A','M',0,0},reply[15];check(program_name_request(msg,13,reply)==15&&!reply[0]&&!strcmp(program_name_draft,"MY PROGRAM"),"editor stages a twelve-character slot name for saving");msg[1]=1;check(program_name_request(msg,13,reply)==2&&reply[0]&&!strcmp(program_name_draft,"MY PROGRAM"),"invalid editor name leaves the pending save name unchanged");
 return fails?1:0;
}
