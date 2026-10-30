/* SPDX-License-Identifier: GPL-3.0-only */
/* Automatic USB controller identity + MCU/HUI decoding. Protocol facts and product
 * sources: docs/MIDI_CONTROLLERS.md. No vendor application or driver executes here. */
/* Ported from SynthMaven e49239e; MIDI ownership and recording use Melodee's
 * existing bounded queues and recorder. */
static uint8_t sm_controller_override,sm_controller_port;
static uint32_t midi_out_room(void){return usb_midi_out_ready()&&mo_w-mo_r<MQ?MQ-(mo_w-mo_r):0;}
static int midi_out_try(uint32_t packet){if(!midi_out_room())return 0;midi_out_q[mo_w%MQ]=packet;RING_PUBLISH();mo_w++;return 1;}
static int sm_key_accept(uint32_t port){return !port;}
enum { CT_AUTO,CT_MIDI,CT_MCU,CT_HUI };
static const char *const CT_MODE_NAME[]={"AUTO","MIDI","MCU","HUI"};
static uint8_t ct_confirmed,ct_connected,ct_mode=CT_MIDI,ct_daw=255,ct_focus=255,ct_dirty;
static uint16_t ct_vid,ct_pid;
static uint8_t ct_port_hint=255;
static char ct_product[48],ct_family[24],ct_iface[32];
static uint8_t ct_zone=255,ct_zone_pending,ct_msb[8],ct_msb_valid,ct_touch;
static uint32_t ct_ping_at,ct_feedback_at,ct_zone_at;
static uint8_t ct_last[4][6],ct_last_transport=255,ct_query;
static uint32_t ct_count;
static volatile uint8_t ct_transport_pending,ct_record_pending;
static uint8_t ct_grid,ct_grid_init,ct_grid_mode_sent;
static volatile uint8_t ct_pad_on[64];
static volatile uint32_t ct_pad_dirty[2];
static int ct_sysex_to(uint32_t cable,const uint8_t *data,uint32_t n);
#include "controller_profiles.h"
static void ct_copy(char *out,const char *in,uint32_t n){uint32_t i=0;while(i+1u<n&&in[i]){out[i]=in[i];i++;}out[i]=0;}
static void ct_normal(char *out,const char *in,uint32_t n){uint32_t k=0;for(uint32_t i=0;in[i]&&k+1u<n;i++){uint8_t c=(uint8_t)in[i];if(c>='A'&&c<='Z')c+=32;if((c>='a'&&c<='z')||(c>='0'&&c<='9'))out[k++]=(char)c;}out[k]=0;}
static int ct_contains(const char *s,const char *q){for(uint32_t i=0;s[i];i++){uint32_t j=0;while(q[j]&&s[i+j]==q[j])j++;if(!q[j])return 1;}return 0;}
static void ct_refresh(void){memset(ct_last,255,sizeof ct_last);ct_last_transport=255;ct_dirty=1;}
static void sm_controller_attach(uint16_t vid,uint16_t pid,const char *product,const char *iface){
 char name[64],inf[40];ct_normal(name,product,sizeof name);ct_normal(inf,iface,sizeof inf);
 ct_port_hint=255;ct_focus=255;ct_vid=vid;ct_pid=pid;ct_connected=1;ct_confirmed=0;ct_copy(ct_iface,iface,sizeof ct_iface);ct_copy(ct_product,product,sizeof ct_product);ct_copy(ct_family,"GENERIC USB MIDI",sizeof ct_family);
 ct_daw=255;ct_mode=CT_MIDI;ct_zone=255;ct_zone_pending=0;ct_msb_valid=ct_touch=0;ct_count=0;
 for(uint32_t i=0;i<NELEM(CT_PROFILE);i++){const ct_profile_t *p=&CT_PROFILE[i];if((vid==p->vid||(vid==0x09e8&&p->vid==0x13b2))&&ct_contains(name,p->match)){ct_copy(ct_family,p->family,sizeof ct_family);ct_daw=p->daw;if(vid==0x0a73)ct_mode=ct_contains(name,"hui")?CT_HUI:CT_MCU;break;}}
 if(ct_contains(inf,"daw")||ct_contains(inf,"mackie")||ct_contains(inf,"hui")){ct_daw=0;ct_mode=ct_contains(inf,"hui")?CT_HUI:CT_MCU;}
 if(vid==0x1c75 && (ct_contains(name,"minilab3")||ct_contains(name,"minilab37")))ct_daw=2;
 if(sm_controller_override){ct_mode=sm_controller_override;ct_daw=ct_mode==CT_MIDI?255:0;}
 else if(ct_daw!=255&&ct_mode==CT_MIDI)ct_mode=CT_MCU;
 if(sm_controller_port && sm_controller_override!=CT_MIDI)ct_daw=(uint8_t)(sm_controller_port-1u);
 ct_confirmed=(uint8_t)(sm_controller_override==CT_MCU||ct_mode==CT_HUI);
 midi_out_control_only=(uint8_t)(ct_daw==0u&&ct_mode>=CT_MCU);
 ct_grid=0;ct_grid_init=0;ct_grid_mode_sent=0;memset((void*)ct_pad_on,0,sizeof ct_pad_on);ct_pad_dirty[0]=ct_pad_dirty[1]=0;
 if(!sm_controller_override && vid==0x09e8 && ct_contains(name,"apcmini"))ct_grid=1;
 if(!sm_controller_override && vid==0x1235){
  if(ct_contains(name,"launchpadx")||ct_contains(name,"lpx"))ct_grid=12;
  else if((ct_contains(name,"launchpadmini")||ct_contains(name,"lpmini"))&&ct_contains(name,"mk3"))ct_grid=13;
  else if((ct_contains(name,"launchpadpro")||ct_contains(name,"lppro"))&&ct_contains(name,"mk3"))ct_grid=14;
 }
 if(ct_grid){ct_daw=255;ct_mode=CT_MIDI;midi_out_control_only=1;}
 ct_query=(uint8_t)(vid==0x0a73 && ct_daw!=255 && ct_mode==CT_MCU);ct_ping_at=fm1_ms-1000u;ct_feedback_at=fm1_ms;ct_refresh();
}
static void sm_controller_hint(uint32_t hint){ct_port_hint=hint<16u?(uint8_t)hint:255;if(ct_port_hint<16u&&!ct_grid&&!sm_controller_port&&sm_controller_override!=CT_MIDI){ct_daw=ct_port_hint;if(ct_mode==CT_MIDI)ct_mode=CT_MCU;midi_out_control_only=(uint8_t)(ct_daw==0);}}
static void sm_controller_disconnect(void){ct_transport_pending=ct_record_pending=0;midi_out_control_only=0;ct_grid=ct_grid_init=ct_grid_mode_sent=0;ct_connected=0;ct_vid=ct_pid=0;ct_product[0]=ct_family[0]=0;ct_daw=255;ct_mode=CT_MIDI;ct_zone=255;ct_zone_pending=ct_msb_valid=ct_touch=0;ct_focus=255;ct_query=0;ct_refresh();}
static void sm_controller_mode(uint32_t mode){if(mode>CT_HUI)return;sm_controller_override=(uint8_t)mode;ct_zone_pending=ct_msb_valid=ct_touch=0;if(ct_connected){char n[48];ct_copy(n,ct_product,sizeof n);uint8_t hint=ct_port_hint;sm_controller_attach(ct_vid,ct_pid,n,ct_iface);sm_controller_hint(hint);}}
static uint32_t ct_packet(uint32_t st,uint32_t a,uint32_t b){return(ct_daw==255?0u:(uint32_t)ct_daw<<4)|(st>>4)|(st<<8)|(a<<16)|(b<<24);}
/* Main-loop output only. Reserve complete pairs/frames before publishing. */
static int ct_output(const uint32_t *packets,uint32_t n){int ok=0;uint32_t guard=motion_guard();if(midi_out_room()>=n){for(uint32_t i=0;i<n;i++)(void)midi_out_try(packets[i]);ok=1;}motion_unguard(guard);return ok;}
static int ct_send(uint32_t st,uint32_t a,uint32_t b){uint32_t p=ct_packet(st,a,b);return ct_output(&p,1);}
static int ct_sysex_to(uint32_t cable,const uint8_t *data,uint32_t n){
 uint32_t packets=(n+2u)/3u;if(cable>15u||!n||n>32u)return 0;int ok=0;
 uint32_t guard=motion_guard();if(midi_out_room()>=packets){for(uint32_t i=0;i<n;){uint32_t left=n-i,k=left>3u?3u:left,cin=left>3u?4u:4u+k,p=(cable<<4)|cin;for(uint32_t j=0;j<k;j++)p|=(uint32_t)data[i++]<<(8u*(j+1u));(void)midi_out_try(p);}ok=1;}motion_unguard(guard);return ok;
}
static int ct_sysex(const uint8_t *data,uint32_t n){return ct_sysex_to(ct_daw,data,n);}
static int ct_pad_mode(uint32_t mode){const uint8_t b[]={0xF0,0,0x20,0x29,2,ct_grid,14,(uint8_t)mode,0xF7};return ct_grid>=12u?ct_sysex_to(0,b,sizeof b):1;}
static uint32_t ct_pad_raw(uint32_t index){return ct_grid==1?index:11u+(index/8u)*10u+index%8u;}
static void ct_pad_led(uint32_t raw,uint32_t on,uint32_t note){uint32_t st=ct_grid==1?0x96u:0x90u;uint32_t color=on||note%12u==0u?5u:0u;uint32_t guard=motion_guard();(void)midi_out_try(9u|(st<<8)|(raw<<16)|(color<<24));motion_unguard(guard);}
static void sm_controller_mackie(const uint8_t *d,uint32_t n,uint32_t cable){
 if(!ct_connected||sm_controller_override==CT_MIDI||n<7u||n>32u||d[0]!=0xF0u||d[1]||d[2]||d[3]!=0x66u||d[n-1]!=0xF7u)return;
 if(d[4]!=0x14u&&d[4]!=0x10u&&d[4]!=5u)return;
 if((sm_controller_override==CT_MCU&&d[4]==5u)||(sm_controller_override==CT_HUI&&d[4]!=5u))return;
 ct_daw=(uint8_t)cable;ct_mode=d[4]==5u?CT_HUI:CT_MCU;ct_confirmed=1;midi_out_control_only=(uint8_t)(ct_daw==0);
 if(d[5]==1u&&n==18u&&ct_mode==CT_MCU){uint8_t out[18];memcpy(out,d,18);out[5]=2;const uint8_t *c=d+13;
  out[13]=(uint8_t)((c[0]+(c[1]^10u)-c[3])&127u);out[14]=(uint8_t)(((c[2]>>4)^(c[0]+c[3]))&127u);
  out[15]=(uint8_t)(((c[3]-(c[2]<<2))^(c[0]|c[1]))&127u);out[16]=(uint8_t)((c[1]-c[2]+(0xF0u^(c[3]<<4)))&127u);ct_sysex(out,18);
 }ct_refresh();
}
static void ct_transport(uint32_t action){ct_transport_pending=(uint8_t)(action+1u);}
static void ct_transport_apply(uint32_t action){if(action==0){transport_req=2;for(uint32_t p=0;p<4;p++)if((song.rec>>p)&1u)rec_set(&trk[p],0);}else if(action==1){if(!song.playing)transport_req=1;}else{if(song.rec){for(uint32_t p=0;p<4;p++)if((song.rec>>p)&1u)rec_set(&trk[p],0);}else{rec_set(&trk[song.sel],1);if(!song.playing)transport_req=1;}}ct_dirty=1;}
static void ct_strip(uint32_t part,uint32_t op,int32_t value){if(part>=4u)return;track_t *t=&trk[part];if(op==0)t->p[P_LEVEL]=(int16_t)clamp((int32_t)value,0,127);else if(op==1)t->p[P_PAN]=(int16_t)clamp(t->p[P_PAN]+(int32_t)value,-64,63);else if(op==2)t->p[P_MUTE]^=1;else if(op==3)track_solo^=(uint8_t)(1u<<part);else if(op==4)ct_focus=(uint8_t)part;else if(op==5)ct_record_pending^=(uint8_t)(1u<<part);ct_dirty=1;}
static int sm_controller_event(uint32_t st,uint32_t ch,uint32_t *note,uint32_t b,uint32_t port,uint32_t cable){
 uint32_t a=*note;
 if(!port&&ct_connected&&ct_grid&&!cable&&sm_key_accept(port)&&!song.g[G_ROUTE]){
  if(st==0x90u||st==0x80u||st==0xA0u){
   uint32_t index=a;
   if(ct_grid==1){if(a>=64u)return 1;}
   else {uint32_t row=a/10u,col=a%10u;if(row<1u||row>8u||col<1u||col>8u)return 1;index=(row-1u)*8u+col-1u;}
   *note=36u+index;if(st!=0xA0u){ct_pad_on[index]=(uint8_t)(st==0x90u&&b);ct_pad_dirty[index/32u]|=1u<<(index%32u);}return 0;
  }
  if(ct_grid==1&&!ch&&st==0xB0u&&a>=48u&&a<56u){ct_strip(a-48u,0,(int32_t)b);return 1;}
 }
 if(sm_controller_override==CT_MIDI)return 0;
 if(port || !ct_connected || ct_daw==255 || cable!=ct_daw || !sm_key_accept(port) || song.g[G_ROUTE])return 0;
 if(!ch && st==0x90u && !a && b==127u && sm_controller_override!=CT_MCU && fm1_ms-ct_ping_at<500u){ct_mode=CT_HUI;ct_confirmed=1;ct_refresh();ct_count++;return 1;}
 if(!ch && st==0xB0u && a==15u){ct_zone=(uint8_t)b;ct_zone_at=fm1_ms;ct_zone_pending=1;ct_count++;return 1;}
 if(!ch && st==0xB0u && a==47u && ct_zone_pending && fm1_ms-ct_zone_at<250u && ct_zone<=29u && sm_controller_override!=CT_MCU){if(ct_mode!=CT_HUI)ct_refresh();ct_mode=CT_HUI;ct_confirmed=1;ct_zone_pending=0;}
 if(ct_mode==CT_HUI){
  if(st==0xB0u&&!ch){
   if(a<8u){ct_msb[a]=(uint8_t)b;ct_msb_valid|=(uint8_t)(1u<<a);}
   else if(a>=32u&&a<40u){uint32_t p=a-32u;if(ct_msb_valid&(1u<<p)){ct_strip(p,0,((uint32_t)ct_msb[p]*128u+b)*127u/16383u);ct_msb_valid&=(uint8_t)~(1u<<p);}}
   else if(a>=64u&&a<72u){int32_t delta=b>64u?(int32_t)b-64:-(int32_t)b;ct_strip(a-64u,1,delta);}
   else if(a==47u){uint32_t key=b&15u,on=b&64u;if(ct_zone<8u){if(!key){if(on)ct_touch|=(uint8_t)(1u<<ct_zone);else ct_touch&=(uint8_t)~(1u<<ct_zone);}else if(on){if(key==1)ct_strip(ct_zone,4,0);else if(key==2)ct_strip(ct_zone,2,0);else if(key==3)ct_strip(ct_zone,3,0);else if(key==7)ct_strip(ct_zone,5,0);}}else if(on&&ct_zone==14u){if(key==3)ct_transport(0);else if(key==4)ct_transport(1);else if(key==5)ct_transport(2);}}
  }
  ct_count++;return 1;
 }
 if(ct_mode==CT_MCU){
  ct_confirmed=1;
  if(st==0xE0u){ct_strip(ch,0,(a+128u*b)*127u/16383u);}
  else if(!ch&&st==0xB0u&&a>=16u&&a<24u){int32_t delta=(int32_t)(b&63u);if(b&64u)delta=-delta;ct_strip(a-16u,1,delta);}
  else if(!ch&&(st==0x90u||st==0x80u)){
   uint32_t on=st==0x90u&&b;
   if(a>=104u&&a<112u){if(on)ct_touch|=(uint8_t)(1u<<(a-104u));else ct_touch&=(uint8_t)~(1u<<(a-104u));}
   else if(on){if(a<8u)ct_strip(a,5,0);else if(a<16u)ct_strip(a-8u,3,0);else if(a<24u)ct_strip(a-16u,2,0);else if(a<32u)ct_strip(a-24u,4,0);else if(a==93u)ct_transport(0);else if(a==94u)ct_transport(1);else if(a==95u)ct_transport(2);}
  }
  ct_count++;return 1;
 }
 return 0;
}
static int ct_hui_led(uint32_t zone,uint32_t port,uint32_t on){uint32_t p[2]={ct_packet(0xB0,12,zone),ct_packet(0xB0,44,(on?64u:0u)|port)};return ct_output(p,2);}
static void sm_controller_tick(uint32_t now){
 if(!ct_connected)return;
 {uint32_t guard=motion_guard();uint8_t transport=ct_transport_pending,record=ct_record_pending;ct_transport_pending=ct_record_pending=0;
  if(transport)ct_transport_apply(transport-1u);
  for(uint32_t p=0;p<4;p++)if(record&(1u<<p))rec_set(&trk[p],!((song.rec>>p)&1u));
  motion_unguard(guard);
 }
 if(ct_grid){
  if(!ct_grid_mode_sent){if(!ct_pad_mode(1))return;ct_grid_mode_sent=1;}
  for(uint32_t i=0;i<8u&&ct_grid_init<64u&&midi_out_room();i++,ct_grid_init++)ct_pad_led(ct_pad_raw(ct_grid_init),ct_pad_on[ct_grid_init],36u+ct_grid_init);
  for(uint32_t bank=0;bank<2u;bank++){
   uint32_t guard=motion_guard();uint32_t pending=ct_pad_dirty[bank];ct_pad_dirty[bank]=0;motion_unguard(guard);
   for(uint32_t bit=0;bit<32u;bit++)if(pending&(1u<<bit)){uint32_t key=bank*32u+bit;
    if(midi_out_room())ct_pad_led(ct_pad_raw(key),ct_pad_on[key],36u+key);
    else{uint32_t guard=motion_guard();ct_pad_dirty[bank]|=1u<<bit;motion_unguard(guard);}
   }
  }
  return;
 }
 if(ct_daw==255||ct_mode<CT_MCU)return;
 if((ct_mode==CT_HUI||!ct_confirmed)&&now-ct_ping_at>=1000u&&midi_out_room()>=1u){ct_send(0x90,0,0);ct_ping_at=now;}
 if(ct_query){const uint8_t query[]={0xF0,0,0,0x66,0x14,0,0xF7};if(ct_sysex(query,sizeof query))ct_query=0;}
 if(now-ct_feedback_at<100u)return;ct_feedback_at=now;
 for(uint32_t p=0;p<4u;p++){
  uint8_t state[6]={(uint8_t)trk[p].p[P_LEVEL],(uint8_t)(trk[p].p[P_PAN]+64),(uint8_t)trk[p].p[P_MUTE],(uint8_t)((track_solo>>p)&1u),(uint8_t)(song.sel==p),(uint8_t)((song.rec>>p)&1u)};
  for(uint32_t k=0;k<6u;k++)if(state[k]!=ct_last[p][k] && !(k==0&&(ct_touch&(1u<<p))) && midi_out_room()>=2u){
   int sent=0;
   if(ct_mode==CT_MCU){if(!k){uint32_t v=state[k]*16383u/127u;sent=ct_send(0xE0u+p,v&127u,v>>7);}else if(k==1)sent=ct_send(0xB0,48u+p,1u+(uint32_t)state[k]*10u/127u);else{uint32_t note=k==2?16u+p:k==3?8u+p:k==4?24u+p:p;sent=ct_send(0x90,note,state[k]?127:0);}}
   else if(ct_mode==CT_HUI){if(!k){uint32_t v=state[k]*16383u/127u;uint32_t pair[2]={ct_packet(0xB0,p,v>>7),ct_packet(0xB0,32u+p,v&127u)};sent=ct_output(pair,2);}else if(k==1)sent=ct_send(0xB0,16u+p,1u+(uint32_t)state[k]*10u/127u);else sent=ct_hui_led(p,k==2?2u:k==3?3u:k==4?1u:7u,state[k]);}
   if(sent)ct_last[p][k]=state[k];
  }
 }
 uint8_t transport=(uint8_t)((!!song.playing)|((!!song.rec)<<1));if(transport!=ct_last_transport){
  uint32_t packets[6],n=0;
  for(uint32_t i=0;i<3;i++){uint32_t on=i==0?!song.playing:i==1?!!song.playing:!!song.rec;
   if(ct_mode==CT_MCU)packets[n++]=ct_packet(0x90,93u+i,on?127:0);
   else{packets[n++]=ct_packet(0xB0,12,14);packets[n++]=ct_packet(0xB0,44,(on?64u:0u)|(3u+i));}}
  if(ct_output(packets,n))ct_last_transport=transport;
 }
}
