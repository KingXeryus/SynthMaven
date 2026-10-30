/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
int main(void)
{
    ui_power_on();go_home();
    sm_tuning=(sm_tuning_t){.apply=15,.mask=0xfff};
    press(B_SCL);frames(400);
    assert(str_eq(cur_page()->title,"MASTER TUNE"));
    turn(EN_K1,1);assert(sm_tuning.scale==1 && sm_tuning.enabled);
    turn(EN_K2,3);assert(sm_tuning.octave==3);
    turn(EN_K2,20);assert(sm_tuning.octave==8);
    turn(EN_K2,-20);assert(sm_tuning.octave==0);
    turn(EN_K3,2);assert(sm_tuning.root==2);
    btn_down(B_SCL);frames(3100);btn_up(B_SCL);frame();assert(str_eq(cur_page()->title,"CUSTOM TUNE"));
    turn(EN_K1,6);assert(sm_tune_key==6);
    turn(EN_K2,1);assert(sm_tuning.scale==47 && sm_tuning.cents[4]==-40);
    turn(EN_K2,-1);assert(sm_tuning.cents[4]==-50);
    turn(EN_K2,-20);assert(sm_tuning.cents[4]==-100);
    turn(EN_K2,30);assert(sm_tuning.cents[4]==100);
    turn(EN_K3,1);assert(sm_tuning.quantize==1);
    turn(EN_K4,-1);assert(!(sm_tuning.apply&(1u<<song.sel)));
    turn(EN_K4,1);assert(sm_tuning.apply&(1u<<song.sel));
    puts("SynthMaven tuning controls: octave-only threshold, root-relative custom conversion, 10-cent steps and part bypass passed");
    return 0;
}
