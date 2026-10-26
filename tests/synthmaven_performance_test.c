/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint32_t st_crc32(const void *data,uint32_t n){const uint8_t*p=data;uint32_t c=~0u;while(n--){c^=*p++;for(int i=0;i<8;i++)c=(c>>1)^(0xEDB88320u&-(c&1));}return ~c;}
#include "../firmware/src/synthmaven_store.h"
static uint8_t wire[1024];static uint32_t wn;
static void ed_b(uint32_t x){wire[wn++]=(uint8_t)(x&127);}
static void ed_v(int32_t x){x+=8192;ed_b(x);ed_b(x>>7);}
static int32_t ed_rv(const uint8_t*p){return(p[0]|p[1]<<7)-8192;}
static void ed_str(const char*s,uint32_t n){while(n--&&*s)ed_b((uint8_t)*s++);ed_b(0);}
static void ed_pack7(const uint8_t*p,uint32_t n){while(n){uint32_t k=n<7?n:7,h=0;for(uint32_t i=0;i<k;i++)h|=(p[i]>>7)<<i;ed_b(h);for(uint32_t i=0;i<k;i++)ed_b(p[i]);p+=k;n-=k;}}
static uint32_t ed_unpack7(const uint8_t*p,uint32_t n,uint8_t*out,uint32_t max){uint32_t k=0;while(n&&k<max){uint8_t h=*p++;n--;for(uint32_t i=0;i<7&&n&&k<max;i++,n--)out[k++]=*p++|((h>>i&1)<<7);}return k;}
#include "../firmware/src/editor_performance.c"
int main(void){outdir="build/host";bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();go_home();frames(2);
 memset(sm_pedal,0,sizeof sm_pedal);sm_action_w=sm_action_r=0;sm_repeat_ms=0;fm1_ms=100;
 check(!sm_learn_arm(4),"arm program-next learn");sm_pedal_receive(0xB0,2,64,127,0,1);
 check(sm_learn_state==2&&sm_candidate[3]==3&&!sm_pedal[4][0]&&sm_action_w==sm_action_r,"learning stages message without assigning or triggering");
 check(!sm_learn_accept()&&sm_pedal[4][0]==1&&sm_pedal[4][1]==64,"explicit acceptance commits mapping");
 check(!sm_learn_arm(5),"second learn arms");sm_pedal_receive(0xB0,2,64,127,0,1);check(sm_learn_accept()==2&&!sm_pedal[5][0],"overlapping assignment refused");sm_learn_state=0;
 sm_repeat_ms=80;sm_pedal_receive(0xB0,2,64,127,0,1);check(sm_last_result==1,"first mapped tap triggers");fm1_ms+=10;sm_pedal_receive(0xB0,2,64,127,0,1);check(sm_last_result==2,"duplicate inside chosen window suppressed");
 sm_pedal_receive(0xB0,2,64,0,0,1);sm_pedal_receive(0xB0,2,64,127,0,1);check(sm_last_result==1,"CC release immediately re-arms intentional repeated tap");
 sm_action_w=sm_action_r=0;memset(sm_pedal,0,sizeof sm_pedal);sm_pedal[0][0]=2;sm_pedal[0][1]=10;sm_pedal[0][3]=1;sm_repeat_ms=0;for(int i=0;i<3;i++)sm_pedal_receive(0xC0,0,10,0,0,1);check(sm_action_w-sm_action_r==3,"default OFF preserves repeated PC taps");
 sm_performance_record_t rec;sm_repeat_ms=75;sm_setlist_len=3;sm_setlist[0]=1;sm_setlist[1]=0;sm_setlist[2]=1;sm_performance_fill(&rec);sm_repeat_ms=0;sm_setlist_len=0;check(!sm_performance_take(&rec)&&sm_repeat_ms==75&&sm_setlist_len==3,"global assignments and setlist round trip");rec.crc^=1;check(sm_performance_take(&rec),"corrupt performance settings rejected");
 song.sel=0;apply_preset_to(&trk[0],0);trk[0].p[P_LEVEL]=71;trk[0].p[P_MUTE]=1;trk[0].p[P_REV]=91;trk[0].p[P_MAC1]=-12;tune_cfg.root=5;master_tuning_bypass=4;check(!sm_scene_capture(0),"capture scene");trk[0].p[P_LEVEL]=110;trk[0].p[P_MUTE]=0;trk[0].p[P_REV]=0;trk[0].p[P_MAC1]=0;tune_cfg.root=0;check(!sm_scene_apply(0)&&trk[0].p[P_LEVEL]==71&&trk[0].p[P_MUTE]&&trk[0].p[P_REV]==91&&trk[0].p[P_MAC1]==-12&&tune_cfg.root==5&&master_tuning_bypass==4,"scene restores mix sends macros and selected tuning");
 program_t p;program_capture(&p);memset(sm_scenes,0,sizeof sm_scenes);program_apply(&p);check(sm_scenes[0].valid&&sm_scenes[0].part[0].level==71,"scenes persist inside program");
 sm_setlist_active=0;sm_setlist_len=2;sm_setlist[0]=1;sm_setlist[1]=0;sm_pending_program=255;check(!sm_performance_action(8)&&sm_pending_program==1&&sm_pending_setpos==0,"first setlist Next requests first entry");sm_pending_program=255;
 sm_current_program=0;song.g[G_SLOT]=8;check(!sm_performance_action(4)&&sm_pending_program==1,"program Next uses loaded program independent of SAVE cursor");sm_pending_program=255;
 apply_preset_to(&trk[0],0);int16_t *v;const param_desc_t*d=sm_filter_desc(1,&v);check(v==&trk[0].p[P_E4]&&d->max>d->min,"analog filter uses actual cutoff descriptor");trk[0].eng_req=1;d=sm_filter_desc(1,&v);check(d->max==d->min,"FM brightness does not expose nonexistent filter cutoff");sm_filter_comp_mask=0;check(!sm_filter_comp_steps(&trk[0]),"compensation OFF leaves calibrated level unchanged");
 wn=0;check(ed_performance(112,0,0)&&wn==22,"native status wire has complete 22-byte diagnostics");
 uint8_t badscene[58]={5,0};wn=0;check(ed_performance(113,badscene,58)&&wire[0]==1,"native scene put rejects excess packed bytes");
 uint8_t listbad[3]={1,1,8};sm_setlist_len=2;wn=0;ed_performance(114,listbad,3);check(wire[0]==1&&sm_setlist_len==2,"native malformed setlist write leaves list intact");
 apply_preset_to(&trk[0],0);wn=0;ed_performance(116,0,0);check(wire[0]==0&&wn>50&&wn<sizeof wire,"native filter descriptor reply fits bounded editor buffer");
 printf("Performance controls: %s\n",fails?"FAIL":"PASS");return fails!=0;}
