/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
static void down(uint32_t b){fm1_in.buttons|=1u<<panel.btn[b];host_pressed|=1u<<panel.btn[b];frame();}
static void up(uint32_t b){fm1_in.buttons&=~(1u<<panel.btn[b]);frame();}
int main(void){
 ui_power_on();go_home();sm_layers.enabled=0;
 down(B_GLO);frames(1800);assert(ui.home&&!ui.layer&&!sm_layers.enabled);frames(300);assert(sm_layers.enabled&&sm_layer_notice_until);up(B_GLO);assert(!ui.lock&&ui.home);frames(1600);
 press(B_SCL);assert(cur_page()->scope==SC_MASTER_TUNE&&cur_page()->id[0]==0);turn(EN_SELECT,1);assert(sm_tuning.scale==1);
 down(B_SCL);frames(2800);assert(cur_page()->id[0]==0);frames(300);up(B_SCL);assert(cur_page()->id[0]==4);
 go_home();down(B_EDIT);frames(1800);assert(ui.home);frames(300);assert(cur_page()->scope==SC_SM_MIX);up(B_EDIT);assert(cur_page()->scope==SC_SM_MIX);
 turn(EN_SELECT,1);assert(song.sel==1);turn(EN_K2,-7);assert(trk[1].p[P_LEVEL]>=0);turn(EN_ALGO,1);assert(sm_layers.low[1]==33);turn(EN_ALGO,100);assert(sm_layers.low[1]==93);turn(EN_PRESET,-13);assert(trk[1].p[P_PAN]==-13);
 go_home();down(B_ENV);frames(2800);assert(ui.home);frames(300);up(B_ENV);assert(cur_page()->graph==GR_CHORD);
 puts("Shortcuts: delayed GLO toggle, scale selection, custom tuning, latched EDIT mixer, split/pan/part controls and three-second ENV chords passed");return 0;
}
