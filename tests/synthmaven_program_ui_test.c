/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
int main(void){
 ui_power_on();go_home();press(B_SAVE);frames(400);
 assert(cur_page()->scope==SC_SM_PROGRAM);
 turn(EN_SELECT,7);assert(sm_program_slot==8);
 turn(EN_SELECT,1);assert(sm_program_slot==1);
 turn(EN_K4,1);assert(sm_program_override==1);
 turn(EN_K3,1);assert(act_col()==3);act_do();
 assert(nm.kind==NK_SM_PROGRAM_SAVE && nm.slot==0);
 name_close();
 puts("Program UI: SAVE opens programs, SELECT cycles eight slots, optional tuning and named save passed");return 0;
}
