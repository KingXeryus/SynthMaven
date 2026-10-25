/* SPDX-License-Identifier: GPL-3.0-only */
/* Real USB EP1 event parser -> MIDI ring -> sequencer -> voices/controllers.
 * This tests USB device-mode data handling, not physical USB host enumeration. */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static void usb_event(uint32_t st,uint32_t a,uint32_t b){
 uint8_t bytes[4]={(uint8_t)(st>>4),(uint8_t)st,(uint8_t)a,(uint8_t)b};
 check(ep1_take(bytes,4)==1,"USB MIDI endpoint accepts event");frames(2);
}
static voice_t*playing(uint8_t note){for(uint32_t i=0;i<NVOICE;i++)if(trk[0].v[i].active&&trk[0].v[i].note==note&&trk[0].v[i].gate)return &trk[0].v[i];return 0;}
static uint32_t gates(void){uint32_t n=0;for(uint32_t i=0;i<NVOICE;i++)n+=trk[0].v[i].active&&trk[0].v[i].gate;return n;}
static void trs(uint32_t st,uint32_t a,uint32_t b){midi_in_q[mi_w++%MQ]=0x10u|(st>>4)|(st<<8)|(a<<16)|(b<<24);frames(2);}
int main(void){outdir="build/host";bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();midi_in_ch=0;go_home();frames(2);sm_set(0,0,0);trk[0].p[P_AMODE]=0;trk[0].p[P_VOICE]=0;
 for(uint32_t ch=0;ch<16;ch++){uint8_t note=48+ch;usb_event(0x90u+ch,note,1+ch*8);voice_t*v=playing(note);check(v&&v->vel==1+ch*8,"USB notes preserve velocity on all 16 channels");usb_event(0x80u+ch,note,64);check(!playing(note),"USB note-off releases key");}
 usb_event(0x90,60,127);check(playing(60)&&playing(60)->vel==127,"maximum velocity preserved");usb_event(0x90,60,0);check(!playing(60),"zero-velocity note-on releases key");
 usb_event(0x90,64,100);usb_event(0xB0,1,93);usb_event(0xB0,11,72);usb_event(0xD0,85,0);usb_event(0xA0,64,96);usb_event(0xE0,0,96);
 check(part_ctl[0].mw==93&&part_ctl[0].expr==72&&part_ctl[0].at==85&&part_ctl[0].bend==16384,"USB modulation, expression, channel pressure and 14-bit pitch bend");check(playing(64)&&playing(64)->pat==96,"USB polyphonic aftertouch reaches the matching note");
 usb_event(0xB0,64,127);usb_event(0x80,64,0);check(part_ctl[0].sus,"USB sustain pedal down");usb_event(0xB0,64,0);check(!part_ctl[0].sus&&!playing(64),"USB sustain release");
 usb_event(0xB0,121,0);trs(0xB0,1,111);trs(0xD0,111,0);trs(0xE0,0,127);check(!part_ctl[0].mw&&!part_ctl[0].at&&!part_ctl[0].bend,"USB-only keyboard ignores TRS performance controls");
 sm_set(0,1,0);usb_event(0xB0,1,99);usb_event(0xD0,99,0);usb_event(0xE0,0,127);check(!part_ctl[0].mw&&!part_ctl[0].at&&!part_ctl[0].bend,"TRS-only keyboard ignores USB performance controls");
 trs(0xB0,1,88);trs(0xD0,77,0);check(part_ctl[0].mw==88&&part_ctl[0].at==77,"TRS articulation remains available");
 sm_set(0,0,0);usb_event(0xB0,64,127);usb_event(0x90,67,110);sm_set(0,1,0);usb_event(0x80,67,0);usb_event(0xB0,64,0);check(!part_ctl[0].sus&&!playing(67),"changing input still accepts old source key/pedal releases");
 sm_set(0,0,0);mpe_mode=1;usb_event(0x91,62,100);usb_event(0xE1,0,96);usb_event(0xD1,91,0);usb_event(0xB1,74,95);check(mpe_bend[1]==16384&&mpe_press[1]==91&&mpe_slide[1]==31,"USB MPE bend, pressure and slide");
 sm_set(0,1,0);usb_event(0xE1,0,64);usb_event(0xD1,12,0);usb_event(0xB1,74,64);check(mpe_bend[1]==16384&&mpe_press[1]==91&&mpe_slide[1]==31,"MPE follows keyboard source selection");
 usb_event(0xB0,120,0);check(!gates(),"all sound off remains a safety release from old input");
 printf("USB performance pipeline: %s\n",fails?"FAIL":"PASS");return fails!=0;}
