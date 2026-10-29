/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint16_t pixel(unsigned x,unsigned y){return swap16(cv_px[y*cv_w+x]);}
int main(void){
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();palette_set(4);
 tune_cfg.sys=TU_RAST;tune_cfg.root=0;tune_floor_octave=3;tune_cfg.a4=0;tune_build();
 uint32_t detuned=graph_detuned(tune_floor_note());
 check((detuned&(1u<<4))&&(detuned&(1u<<11)),"Rast highlights E and B physical keys");
 cv_begin(240,124,C_BLACK);graph_keyboard(29,MAQAM_NOTE_MASKS[0],detuned,0);
 check(pixel(15,65)==C_WHITE,"included natural key stays white");
 check(pixel(80,65)==RGB(255,44,52),"detuned natural key is red");
 check(pixel(40,40)==RGB(128,132,140),"excluded sharp is grey");
 cv_begin(240,124,C_BLACK);graph_keyboard(29,0xFFF,0,0);
 check(pixel(40,40)==C_BLACK,"included equal tempered sharp is black");
 tune_cfg.root=2;tune_build();detuned=graph_detuned(tune_floor_note());
 check((detuned&(1u<<6))&&(detuned&(1u<<1)),"root rotation changes F sharp and C sharp, not piano geometry");
 check(graph_detuned(36)==0,"octaves below tuning boundary remain equal tempered");
 cv_begin(240,124,C_BLACK);cv_card(5,5,100,50,C_WHITE);
 check(pixel(5,5)==C_BLACK&&pixel(9,5)==C_WHITE,"rounded card corners remain clear");
 puts("Keyboard colours, root rotation, tuning boundary and rounded geometry: PASS");return 0;
}
