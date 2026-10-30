/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
static int event(uint32_t st,uint32_t ch,uint32_t a,uint32_t b,uint32_t cable){return sm_controller_event(st,ch,&a,b,0,cable);}
int main(void){
 host_tracks_init();song.g[G_ROUTE]=0;
 sm_controller_attach(0x09e8,1,"MPK mini IV","MIDI");assert(ct_connected&&ct_mode==CT_MIDI);
 sm_controller_attach(0x1235,1,"Launchkey MK3","MIDI");assert(ct_mode==CT_MCU&&ct_daw==1);
 int level=trk[0].p[P_LEVEL];assert(!event(0xe0,0,127,127,0)&&trk[0].p[P_LEVEL]==level);
 assert(event(0xe0,0,127,127,1)&&trk[0].p[P_LEVEL]==127);
 assert(event(0xb0,0,16,1,1));assert(trk[0].p[P_PAN]==1);
 assert(event(0x90,0,16,127,1)&&trk[0].p[P_MUTE]);
 assert(event(0x90,0,94,127,1)&&ct_transport_pending==2);sm_controller_tick(fm1_ms);assert(transport_req==1);
 sm_controller_mode(CT_HUI);assert(ct_daw==0);
 event(0xb0,0,0,64,0);event(0xb0,0,32,0,0);assert(trk[0].p[P_LEVEL]==63);
 event(0xb0,0,15,14,0);event(0xb0,0,47,67,0);sm_controller_tick(fm1_ms);assert(transport_req==2);
 sm_controller_mode(CT_MIDI);assert(!event(0xe0,0,0,0,0));
 sm_controller_override=0;sm_controller_attach(0x09e8,1,"APC mini","MIDI");uint32_t note=0;assert(ct_grid==1&&!sm_controller_event(0x90,0,&note,100,0,0)&&note==36);
 sm_controller_disconnect();assert(!ct_connected&&!midi_out_control_only);
 puts("Controllers: generic keyboard, Launchkey virtual DAW cable, MCU mixer/transport, HUI fader/transport, manual MIDI bypass, APC pads and detach passed");return 0;
}
