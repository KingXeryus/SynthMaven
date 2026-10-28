/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint32_t st_crc32(const void *data,uint32_t n){const uint8_t*p=data;uint32_t c=~0u;while(n--){c^=*p++;for(int i=0;i<8;i++)c=(c>>1)^(0xEDB88320u&-(c&1));}return ~c;}
#include "../firmware/src/synthmaven_store.h"
static int event(uint32_t st,uint32_t ch,uint32_t a,uint32_t b,uint32_t cable){return sm_controller_event(st,ch,&a,b,0,cable);}
int main(void){
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir="build/host";palette_set(0);program_mode=1;sm_key_port=2;song.g[G_ROUTE]=0;
 uint32_t identities=1;for(uint32_t i=0;i<NELEM(CT_PROFILE);i++){sm_controller_attach(CT_PROFILE[i].vid,42,CT_PROFILE[i].match,"");if(strcmp(ct_family,CT_PROFILE[i].family))identities=0;}
 check(identities,"every sourced controller family identity rule selects its matching profile");
 sm_controller_attach(0x13b2,42,"Alesis VI49","");check(!strcmp(ct_family,"ALESIS VI")&&ct_mode==CT_MIDI,"Alesis controller is identified automatically, standard MIDI retained");
 transport_req=0;check(!event(0x90,0,93,127,0)&&!transport_req,"ordinary keyboard note 93 is never mistaken for STOP");
 sm_controller_attach(0x1235,42,"Launchkey 49 MK4","");check(ct_daw==1&&!strcmp(ct_family,"NOVATION LAUNCHKEY"),"Novation music and DAW ports are separate");
 check(!event(0x90,0,94,127,0),"music-port note 94 stays musical");event(0x90,0,93,127,1);check(transport_req==2,"MCU DAW-port STOP works without sounding a note");transport_req=0;
 event(0xB0,0,15,14,1);event(0xB0,0,47,68,1);check(ct_mode==CT_HUI&&transport_req==1,"HUI zone/port sequence auto-detected and PLAY decoded");transport_req=0;
 event(0xB0,0,15,14,1);event(0xB0,0,47,4,1);check(!transport_req,"HUI release never retriggers transport");
 trk[0].p[P_MUTE]=0;event(0xB0,0,15,0,1);event(0xB0,0,47,66,1);check(trk[0].p[P_MUTE]==1,"HUI mute maps to program part 1");
 trk[0].p[P_LEVEL]=77;event(0xB0,0,32,127,1);check(trk[0].p[P_LEVEL]==77,"unpaired HUI fader LSB ignored");event(0xB0,0,0,127,1);event(0xB0,0,32,127,1);check(trk[0].p[P_LEVEL]==127,"HUI paired fader reaches full scale");
 trk[0].p[P_PAN]=0;event(0xB0,0,64,65,1);check(trk[0].p[P_PAN]==1,"HUI signed relative encoder direction is correct");
 sm_controller_attach(0x0a73,16,"MCU Pro","");check(ct_mode==CT_MCU&&ct_daw==0&&midi_out_control_only,"Mackie USB identity selects MCU and isolates music output from surface LEDs");
 part_ctl[0].bend=321;event(0xE0,0,127,127,0);check(trk[0].p[P_LEVEL]==127&&part_ctl[0].bend==321,"MCU pitch-bend fader changes volume instead of detuning sound");
 event(0x90,0,26,127,0);check(ct_focus==2,"MCU select requests part 3 on main loop");trk[0].p[P_PAN]=0;event(0xB0,0,16,65,0);check(trk[0].p[P_PAN]==-1,"MCU relative encoder has its distinct correct direction");
 uint8_t hello[]={0xF0,0,0,0x66,0x14,1,'1','2','3','4','5','6','7',1,2,3,4,0xF7};
 usb.up=1;usb.config=1;mo_r=mo_w=0;
 sm_controller_mackie(hello,sizeof hello,0);check(ct_confirmed&&ct_mode==CT_MCU,"Mackie manufacturer handshake recognized with fixed-length bounds");
 check(mo_w-mo_r==6u && ((midi_out_q[4]>>24)&127u)==5u && ((midi_out_q[5]>>8)&127u)==123u && ((midi_out_q[5]>>16)&127u)==47u,"MCU challenge reply is complete and matches the documented four-byte response");mo_r=mo_w;

 sm_performance_record_t saved;sm_controller_override=3;sm_controller_port=16;sm_performance_fill(&saved);sm_controller_override=0;sm_controller_port=0;
 check(!sm_performance_take(&saved)&&sm_controller_override==3&&sm_controller_port==16,"controller protocol and port survive settings roundtrip");
 saved.reserved=17u<<2;saved.crc=st_crc32(&saved,sizeof saved-4);check(sm_performance_take(&saved)&&sm_controller_port==16,"invalid persisted port is rejected atomically");
 sm_controller_override=0;sm_controller_port=0;sm_controller_attach(0x0a73,16,"MCU Pro","");
 uint32_t key=93;transport_req=0;check(!sm_controller_event(0x90,0,&key,127,1,0)&&!transport_req,"TRS notes are not consumed by a USB surface profile");
 mo_w=mo_r+MQ;uint8_t old_transport=ct_last_transport;sm_controller_tick(fm1_ms+1000);
 check(ct_last_transport==old_transport&&mo_w-mo_r==MQ,"full output queue preserves feedback for retry without overflow");
 mo_r=mo_w;sm_controller_tick(fm1_ms+1200);check(ct_last_transport!=255&&mo_w-mo_r<=MQ,"feedback retries when output queue drains");mo_r=mo_w;
 sm_controller_attach(0x09e8,42,"APC mini mk2","");check(ct_grid==1&&ct_daw==255,"APC Mini grid profile detected without a manual model choice");
 uint32_t n=0;check(!sm_controller_event(0x90,0,&n,100,0,0)&&n==36,"APC bottom-left pad maps to C2");n=63;sm_controller_event(0x80,0,&n,0,0,0);check(n==99,"APC top-right pad note-off uses the same chromatic map");n=60;sm_controller_event(0x90,0,&n,100,0,1);check(n==60,"APC Note Mode port is not remapped");event(0xB0,0,48,90,0);check(trk[0].p[P_LEVEL]==90,"APC first fader controls program part 1");
 sm_controller_attach(0x1235,42,"Launchpad X","");check(ct_grid==12,"Launchpad X gets automatic programmer-mode setup");n=11;sm_controller_event(0x90,0,&n,100,0,0);check(n==36,"Launchpad grid bottom-left maps to C2");n=88;sm_controller_event(0xA0,0,&n,100,0,0);check(n==99,"Launchpad poly-aftertouch follows the mapped key");
 sm_controller_attach(0x1c75,42,"MiniLab 3","");check(!strcmp(ct_family,"ARTURIA MINILAB"),"Arturia MiniLab family detected automatically");
 check(ct_daw==2,"MiniLab 3 control port skips its separate DIN Thru port");sm_controller_hint(3);check(ct_daw==3,"named USB control jack takes precedence over the family port fallback");sm_controller_mode(3);check(ct_daw==3,"protocol changes preserve the detected USB jack port");sm_controller_mode(0);
 uint32_t at=mi_w%MQ;midi_in_event(0xF9u|(0x90u<<8)|(60u<<16)|(100u<<24));check(((midi_in_q[at]>>4)&15u)==0&&midi_in_cable[at]==15,"all 16 USB MIDI cables retain USB source identity");mi_r=mi_w;
 sm_controller_attach(0xffff,42,"Unknown Keyboard","");check(ct_mode==CT_MIDI&&ct_daw==255,"unknown class-compliant keyboards keep a safe standard-MIDI fallback");
 open_family(FAM_GLO);while(cur_page()->graph!=GR_CONTROLLER)open_family(FAM_GLO);ui.force=1;frames(3);ppm("controller-detected");
 sm_controller_disconnect();check(!ct_connected&&!midi_out_control_only&&ct_daw==255,"detach resets controller state and outgoing music routing");
 puts(fails?"Controller support FAIL":"Controller identity, MCU/HUI, pads and USB-cable support PASS");return fails?1:0;
}
