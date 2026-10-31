/* SPDX-License-Identifier: GPL-3.0-only */
#define MIDI_CONTROL_NO_MAIN 1
#include "midi_control_test.c"
#include <assert.h>
/* queued() sends on channel 1 only; a raw status carries the channel. */
static void setup(void){
 midi_test_reset();settings_drumch=0;sm_layers=(sm_layers_t){.high={127,127,127,127}};
 sm_routes=(sm_routes_t){.enabled=1,.input={2,2,0,0,1},.channel={1,1,0,0,SM_ROUTE_ALL}};
 for(uint32_t tr=0;tr<4;tr++){set_engine_of(&trk[tr],HOST_SYNTH);trk[tr].p[P_VOICE]=V_POLY;}
 set_engine_of(&trk[3],ENGI_DRUM);song.rec=0;
}
int main(void){
 /* two layers split on TRS; USB on any channel plays only the drummer */
 setup();sm_layers.high[0]=59;sm_layers.low[1]=60;
 queued(0x90,48,100,2);assert(gate_note(&trk[0],48)&&!gate_note(&trk[1],48));
 queued(0x90,72,100,2);assert(gate_note(&trk[1],72)&&!gate_note(&trk[0],72));
 queued(0x80,48,0,2);queued(0x80,72,0,2);assert(!gate_note(&trk[0],48)&&!gate_note(&trk[1],72));
 queued(0x95,60,100,1);assert(!gate_note(&trk[0],60)&&!gate_note(&trk[1],60));
 /* overlapping ranges stack, and both release */
 setup();queued(0x90,60,100,2);assert(gate_note(&trk[0],60)&&gate_note(&trk[1],60));
 queued(0x80,60,0,2);assert(!gate_note(&trk[0],60)&&!gate_note(&trk[1],60));
 /* an unassigned channel or input plays nothing */
 setup();queued(0x91,60,100,2);queued(0x90,60,100,1);for(uint32_t tr=0;tr<3;tr++)assert(!gate_note(&trk[tr],60));
 /* the same note on the same channel from each input stays on its own target */
 setup();sm_routes.channel[4]=1;sm_routes.input[1]=1;sm_routes.channel[1]=2;
 queued(0x90,60,100,2);assert(gate_note(&trk[0],60));queued(0x80,60,0,1);assert(gate_note(&trk[0],60));queued(0x80,60,0,2);assert(!gate_note(&trk[0],60));
 /* sustain on the keyboard's channel holds both split layers */
 setup();queued(0xB0,64,127,2);queued(0x90,60,100,2);queued(0x80,60,0,2);assert(gate_note(&trk[0],60)&&gate_note(&trk[1],60));
 queued(0xB0,64,0,2);assert(!gate_note(&trk[0],60)&&!gate_note(&trk[1],60));
 /* drummer records through the normal drum path */
 setup();rec_set(&trk[3],1);seq_start();events_block(CTL);queued(0x99,36,100,1);events_block(CTL);
 {uint32_t hits=0;for(uint32_t i=0;i<NSTEP;i++)hits|=trk[3].step[i].hit;assert(hits);}
 seq_stop();rec_set(&trk[3],0);
 /* no DRUM part: the assignment plays nothing and layers stay available */
 setup();set_engine_of(&trk[3],HOST_SYNTH);sm_routes.input[2]=1;sm_routes.channel[2]=2;
 queued(0x91,60,100,1);assert(gate_note(&trk[2],60));queued(0x81,60,0,1);queued(0x95,60,100,1);assert(!gate_note(&trk[3],60));
 /* drummer's part is not also a layer */
 setup();sm_routes.channel[3]=1;sm_routes.input[3]=2;queued(0x90,60,100,2);assert(!mn_get(0,60)||gate_note(&trk[0],60));
 /* disabled: ROUT applies as before */
 setup();sm_routes.enabled=0;song.g[G_ROUTE]=0;queued(0x91,60,100,1);assert(gate_note(&trk[1],60));queued(0x81,60,0,1);
 /* settings round trip; older settings keep the defaults */
 setup();sm_routes.input[2]=2;sm_routes.channel[2]=7;persist_t saved={0};settings_export(&saved);sm_routes.enabled=0;
 assert(settings_import(&saved,sizeof saved)&&sm_routes.enabled==1&&sm_routes.channel[2]==7&&sm_routes.input[2]==2);
 memset(saved.favorites.factory[9]+16,0,SM_ROUTES_WIRE);assert(settings_import(&saved,sizeof saved)&&!sm_routes.enabled&&sm_routes.channel[4]==10);
 {sm_routes_t bad;uint8_t b[SM_ROUTES_WIRE];sm_routes_encode(b,&sm_routes);b[10]=18;assert(!sm_routes_decode(&bad,b));}
 sm_routes=SM_ROUTES_DEFAULT;
 puts("MIDI assignments: split and stacked layers, input and channel filters, sustain, drummer recording and persistence passed");return 0;
}
