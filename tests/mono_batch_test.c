/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static void setup(void){host_tracks_init();program_mode=program_layer=1;song.solo=0;sm_mono_pending=sm_mono_batch=0;for(uint32_t p=0;p<4;p++){set_engine_of(&trk[p],0);apply_preset_to(&trk[p],13);trk[p].engine=0;trk[p].eng_req=0;trk[p].xf_on=0;trk[p].p[P_VOICE]=p?V_MONO:V_POLY;trk[p].p[P_CHORD]=0;trk[p].p[P_PRIO]=0;trk[p].p[P_MUTE]=0;trk[p].p[P_LEVEL]=100;program_route[p].enabled=1;memset(trk[p].v,0,sizeof trk[p].v);trk[p].nmono=0;trk[p].mono_note=0;}}
int main(void){bank_resolve();panel=PANEL_DEFAULT;layers_init();setup();uint32_t before=vage;
 sm_mono_batch_begin();for(uint32_t n=60;n<68;n++)for(uint32_t p=0;p<4;p++)trk_note_on1(&trk[p],n,100);
 check(voices_busy()<=NVOICE,"pending mono starts reserve their share of the global voice budget");sm_mono_batch_end();
 check(vage-before==11&&sm_mono_coalesced==21,"eight-note burst starts each backing voice once, not eight times");
 for(uint32_t p=1;p<4;p++)check(trk[p].v[0].active&&trk[p].v[0].note==67&&trk[p].nmono==8,"mono note stack and final winning note are preserved");
 check(voices_busy()<=NVOICE&&!sm_mono_pending,"flush leaves no pending reservations and respects shared budget");
 setup();trk[1].p[P_PRIO]=1;sm_mono_batch_begin();trk_note_on1(&trk[1],60,50);trk_note_on1(&trk[1],72,120);sm_mono_batch_end();check(trk[1].v[0].note==60&&trk[1].v[0].vel==50,"LOW priority keeps the winning key velocity");
 setup();sm_mono_batch_begin();trk_note_on1(&trk[1],60,90);trk_note_off1(&trk[1],60);sm_mono_batch_end();check(!trk[1].v[0].active&&!sm_mono_pending,"note-off before flush cancels the unsounded note");
 setup();program_layer=0;song.sel=1;sm_mono_batch_begin();trk_note_on1(&trk[1],60,90);check(trk[1].v[0].active&&!sm_mono_pending,"Layer OFF keeps immediate note behavior");sm_mono_batch_end();
 setup();song.solo=2;sm_mono_batch_begin();trk_note_on1(&trk[1],60,90);check(trk[1].v[0].active&&!sm_mono_pending,"solo keeps immediate note behavior");sm_mono_batch_end();
 printf("Layered mono burst batching %s\n",fails?"FAIL":"PASS");return fails?1:0;
}
