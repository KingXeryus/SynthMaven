/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <assert.h>
int main(void)
{
    ui_power_on();
    go_home();
    set_engine(ENGI_HYBRID);
    unsigned part = song.sel;
    apply_preset(ENG_HYBRID.npresets - 1);
    turn(EN_PRESET, 1);
    assert(TSEL->eng_req == ENGI_HYBRID && TSEL->preset == 0 && song.sel == part);
    turn(EN_PRESET, -1);
    assert(TSEL->preset == ENG_HYBRID.npresets - 1 && TSEL->eng_req == ENGI_HYBRID);
    turn(EN_ALGO, 1);
    assert(TSEL->eng_req == ENGI_SM_ANALOG && song.sel == part);
    turn(EN_ALGO, -1);
    assert(TSEL->eng_req == ENGI_HYBRID && song.sel == part);
    unsigned engine = trk[0].eng_req, preset = trk[0].preset;
    turn(EN_SELECT, -1);
    assert(song.sel == 3 && trk[0].eng_req == engine && trk[0].preset == preset);
    turn(EN_SELECT, 1);
    assert(song.sel == 0 && TSEL->eng_req == engine && TSEL->preset == preset);
    puts("SynthMaven HOME: engine, within-engine preset wrap, and four-part selection passed");
    return 0;
}
