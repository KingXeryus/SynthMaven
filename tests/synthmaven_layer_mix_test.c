/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint32_t gated(uint32_t part,uint32_t note){uint32_t n=0;for(uint32_t i=0;i<NVOICE;i++)n+=trk[part].v[i].active&&trk[part].v[i].gate&&trk[part].v[i].note==note;return n;}
static uint64_t stereo_energy(uint32_t channel){int32_t o[CTL*2];uint64_t e=0;for(uint32_t b=0;b<300;b++){mix_block(o,CTL);if(b>150)for(uint32_t i=0;i<CTL;i++)e+=(int64_t)o[2*i+channel]*o[2*i+channel];}return e;}
int main(void){
 outdir="build/host";bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();song.sel=0;program_layer_set(1);
 for(uint32_t i=0;i<4;i++){set_engine_of(&trk[i],0);apply_preset_to(&trk[i],2);trk[i].p[P_AMODE]=0;trk[i].p[P_VOICE]=0;trk[i].p[P_CHOR]=trk[i].p[P_DLY]=trk[i].p[P_REV]=0;}
 frames(8);
 program_route[0]=(program_route_t){1,1,0,60,-12};program_route[1]=(program_route_t){1,1,1,60,12};program_route[2]=(program_route_t){1,0,0,60,0};program_route[3].enabled=0;
 trk_note_on(TSEL,60,100);frames(2);
 check(gated(0,48)&&gated(2,60)&&!gated(1,72)&&!gated(3,60),"split boundary goes to Lower, full layer also plays, disabled slot stays silent");
 trk_note_on(TSEL,61,100);frames(2);check(gated(1,73)&&gated(2,61)&&!gated(0,49),"Upper starts one semitone above split point; octave shifts each part");
 program_route[0].transpose=12;program_route[1].point=90;song.sel=3;
 trk_note_off(TSEL,60);trk_note_off(TSEL,61);
 check(!gated(0,48)&&!gated(1,73)&&!gated(2,60)&&!gated(2,61),"note-offs release original pitches after transpose, split and focus change");
 trk_all_off(TSEL);frames(30);song.sel=0;program_layer_set(0);program_route[0]=(program_route_t){1,0,0,60,0};trk[0].p[P_PAN]=-64;trk[0].p[P_ATK]=0;trk[0].p[P_DEC]=0;trk[0].p[P_SUS]=127;frames(8);trk_note_on(TSEL,48,100);
 uint64_t left=stereo_energy(0),right=stereo_energy(1);check(left>0&&right<left/1000,"mixer hard left pan places the selected layer in left output");
 trk[0].p[P_PAN]=63;left=stereo_energy(0);right=stereo_energy(1);check(right>left*100,"mixer hard right pan places the selected layer in right output");
 trk[0].p[P_MUTE]=1;left=stereo_energy(0);right=stereo_energy(1);check(left+right<2u*149u*CTL,"mute fades a held layer below one PCM least-significant bit RMS");
 trk[0].p[P_MUTE]=0;song.solo=2;left=stereo_energy(0);right=stereo_energy(1);check(left+right<1000,"soloing another layer excludes the held layer");song.solo=1;check(stereo_energy(1)>1000,"soloing the held layer restores its output");
 open_family(FAM_GLO);for(uint32_t p=0;p<NELEM(PAGES);p++)if(!strcmp(PAGES[p].title,"LAYER ROUTE")){ui.page=p;ui.home=0;ui.force=1;frame();break;}encs[panel.enc[EN_K2]]=1;frame();check(program_route[0].split==1,"hardware GLO LAYER ROUTE changes selected sound routing");
 uint8_t payload[10]={2,1,1,1,64,12,90,100,0,1},reply[11];
 panic_req=0;uint8_t selected=song.sel;
 check(program_mix_request(payload,10,reply)==11&&!reply[0]&&reply[1]==2&&program_route[2].point==64&&program_route[2].transpose==-12&&trk[2].p[P_LEVEL]==90&&trk[2].p[P_PAN]==36&&song.sel==selected,"mixer wire command addresses a part without moving editing focus");
 check(panic_req==(1u<<2),"routing change releases only the edited layer");panic_req=0;payload[6]=70;payload[7]=0;
 check(program_mix_request(payload,10,reply)==11&&!panic_req&&trk[2].p[P_LEVEL]==70&&trk[2].p[P_PAN]==-64,"live level and pan changes preserve held notes");
 payload[5]=49;check(program_mix_request(payload,10,reply)==2&&reply[0]&&program_route[2].transpose==-12&&trk[2].p[P_LEVEL]==70,"invalid mixer write changes nothing");
 payload[0]=4;check(program_mix_request(payload,1,reply)==2&&reply[0],"hidden drum part cannot be configured as a synth layer");
 return fails?1:0;
}
