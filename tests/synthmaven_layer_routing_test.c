/* SPDX-License-Identifier: GPL-3.0-only */
#define MIDI_CONTROL_NO_MAIN 1
#include "midi_control_test.c"
#include <assert.h>
static void setup(void){midi_test_reset();sm_layers=(sm_layers_t){.enabled=1,.high={127,127,127,127}};for(uint32_t tr=0;tr<4;tr++){set_engine_of(&trk[tr],HOST_SYNTH);trk[tr].p[P_VOICE]=V_POLY;trk[tr].p[P_CHRD]=0;trk[tr].p[P_QUANT]=0;}events_block(CTL);}
int main(void){
 setup();queued(0x90,60,100,1);for(uint32_t tr=0;tr<4;tr++)assert(gate_note(&trk[tr],60));
 sm_layers.enabled=0;song.sel=3;queued(0x80,60,0,1);for(uint32_t tr=0;tr<4;tr++)assert(!gate_note(&trk[tr],60));
 setup();sm_layers.high[0]=59;sm_layers.low[1]=60;sm_layers.low[2]=72;sm_layers.high[3]=47;
 queued(0x90,60,100,1);assert(!gate_note(&trk[0],60)&&gate_note(&trk[1],60)&&!gate_note(&trk[2],60)&&!gate_note(&trk[3],60));
 queued(0x80,60,0,1);assert(!gate_note(&trk[1],60));
 setup();queued(0xB0,64,127,1);queued(0x90,60,100,1);queued(0x80,60,0,1);for(uint32_t tr=0;tr<4;tr++)assert(gate_note(&trk[tr],60));queued(0xB0,64,0,1);for(uint32_t tr=0;tr<4;tr++)assert(!gate_note(&trk[tr],60));
 setup();queued(0x90,60,100,1);midi_silence_track(0);queued(0x80,60,0,1);for(uint32_t tr=0;tr<4;tr++)assert(!gate_note(&trk[tr],60));
 setup();key_down(7);frame();for(uint32_t tr=0;tr<4;tr++)assert(gate_note(&trk[tr],60));sm_layers.enabled=0;key_up(7);frame();for(uint32_t tr=0;tr<4;tr++)assert(!gate_note(&trk[tr],60));
 setup();key_down(7);frame();queued(0x90,60,100,1);key_up(7);frame();for(uint32_t tr=0;tr<4;tr++)assert(gate_note(&trk[tr],60));queued(0x80,60,0,1);for(uint32_t tr=0;tr<4;tr++)assert(!gate_note(&trk[tr],60));
 setup();for(uint32_t n=60;n<69;n++)queued(0x90,n,100,1);assert(mn_get(0,67)&&!mn_get(0,68));for(uint32_t n=60;n<69;n++)queued(0x80,n,0,1);assert(sm_chord_free()==MCHORD_N);for(uint32_t tr=0;tr<4;tr++)assert(!midi_owners[tr]);
 setup();trk[1].p[P_TRANS]=-12;trk[2].p[P_TRANS]=12;queued(0x90,60,100,1);assert(gate_note(&trk[1],48)&&gate_note(&trk[2],72));trk[1].p[P_TRANS]=7;queued(0x80,60,0,1);assert(!gate_note(&trk[1],48)&&!gate_note(&trk[2],72));
 setup();sm_layers.low[1]=60;sm_layers.high[0]=59;persist_t saved={0};settings_export(&saved);sm_layers.enabled=0;assert(settings_import(&saved,sizeof saved)&&sm_layers.enabled&&sm_layers.low[1]==60&&sm_layers.high[0]==59);
 /* four stacked layers hold eight notes each with the budget high enough; two layers at 24 points hold six before stealing */
 {int16_t kept=settings_budget;
  setup();settings_budget=64;voice_kills=0;for(uint32_t n=60;n<68;n++)queued(0x90,n,100,1);for(uint32_t tr=0;tr<4;tr++)for(uint32_t n=60;n<68;n++)assert(gate_note(&trk[tr],n));assert(!voice_kills);for(uint32_t n=60;n<68;n++)queued(0x80,n,0,1);
  setup();settings_budget=24;sm_layers.low[2]=sm_layers.low[3]=127;sm_layers.high[2]=sm_layers.high[3]=127;for(uint32_t n=60;n<66;n++)queued(0x90,n,100,1);assert(voices_busy()==24u&&gate_note(&trk[0],60));queued(0x90,66,100,1);{uint32_t held=0;for(uint32_t n=60;n<67;n++)held+=gate_note(&trk[0],n)!=0;assert(voices_busy()==24u&&held==6u&&gate_note(&trk[0],60)&&gate_note(&trk[0],66));}for(uint32_t n=60;n<67;n++)queued(0x80,n,0,1);
  /* a budget above 32 survives a save, and the tagged field older firmware reads stays valid */
  setup();settings_budget=64;persist_t saved={0};settings_export(&saved);assert(((saved.ext.usb_off>>8)&255u)==32u);settings_budget=16;assert(settings_import(&saved,sizeof saved)&&settings_budget==64);
  settings_budget=kept;}
 puts("Four-part routing: USB layers, split boundaries, mode changes while held, sustain, part panic and physical keys passed");return 0;
}
