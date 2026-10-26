/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void){
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir="build/host";go_home();
 check(SM_VITAL_COUNT==64u&&HY_NFACTORY==100u&&ENGINES[ENGI_HYBRID]->npresets==105u,"64 Vital patches append without moving 36 patch IDs or 41 sound IDs");
 for(uint32_t i=0;i<4;i++){check(TRK_DEF[i][0]==ENGI_HYBRID&&TRK_DEF[i][1]==SM_VITAL_PRESET_FIRST+i,"startup program contains four complementary Vital sounds");set_engine_of(&trk[i],TRK_DEF[i][0]);apply_preset_to(&trk[i],TRK_DEF[i][1]);}
 song.sel=0;program_mode=1;program_layer_set(0);check(!program_layer&&TSEL->preset==41u,"startup selects first Vital sound with layering OFF");
 for(uint32_t i=0;i<64;i++){check(TSEL->eng_req==ENGI_HYBRID&&TSEL->preset==41u+i,"PRESET traverses the contiguous Vital bank");check(TSEL->p[P_E3]==36u+i,"factory sound references correct appended HYBRID patch");uint32_t k,pos,total;pos=base_pos(&total);check(base_at(pos,&k)==ENGI_HYBRID&&k==TSEL->preset&&!strcmp(base_kind(pos),"VITAL"),"global browser resolves Vital preset without shifting regional entries");home_preset_step(1);}
 check(TSEL->preset==0,"after 64 Vital sounds PRESET continues into existing HYBRID sounds");home_preset_step(-1);check(TSEL->preset==104,"reverse browsing returns to last Vital sound");apply_preset(41);home_preset_step(-1);check(TSEL->preset==40,"first Vital sound wraps backward to last original HYBRID sound");
 set_engine(11);home_engine_step(1);check(TSEL->eng_req==ENGI_HYBRID&&TSEL->preset==41,"ALGORITHM entering HYBRID starts on the Vital group");
 for(uint32_t i=0;i<FAV_N;i++)fav[i]=FAV_NONE;apply_preset(104);fav_toggle(1);uint32_t list[FAV_N];check((fav[0]&FAV_VITAL)&&fav_list(list)==1&&list[0]==VITAL_BANK_FIRST+63,"Vital favourites use distinct keys, preserving old regional favourites");fav_toggle(0);check(!fav_list(list),"Vital favourite removes normally");
 program_t saved;for(uint32_t i=0;i<4;i++){song.sel=i;set_engine_of(TSEL,ENGI_HYBRID);apply_preset_to(TSEL,41+i);}song.sel=2;program_layer_set(1);frames(4);program_capture(&saved);check(program_valid(&saved),"Vital startup program captures normally");set_engine(0);program_layer_set(0);program_apply(&saved);frames(4);check(song.sel==2&&program_layer&&trk[0].preset==41&&trk[3].preset==44,"saved working program overrides startup defaults without replacing user sounds");
 return fails?1:0;
}
