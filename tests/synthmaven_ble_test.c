#define SM_BLE_HOST_TEST 1
/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define SM_BLE 1
#define NPART 3
#define NVOICE 8
#define RING_PUBLISH() ((void)0)
static uint32_t fm1_ms;
static uint8_t sm_radio_quiet,flash_ok=1;
static struct {int playing;} song;
static struct {struct {int active;} v[NVOICE];} trk[NPART];
static struct {int in_audio;} felucca_dbg;
static struct {int force;} ui;
static void str_cpy(char*d,const char*s,unsigned n){if(n){snprintf(d,n,"%s",s);}}
static void fm1_irq_off(void){} static void fm1_irq_on(void){} static void fm1_reboot(void){}
static uint32_t fm1_ticks(void){static uint32_t t;return ++t;}
#define FM1_TICKS_PER_US 24u
static void fm1_wdt_feed(void){} static void usb_poll(void){}
static void fm1_guard_unlock_top(void){} static void fm1_guard_lock_top(void){}
static void fm1_irq_attach(uint32_t n,void(*h)(void),uint32_t p){(void)n;(void)h;(void)p;}
static void fm1_guard_stack_floor(uint32_t n){(void)n;}
static void ui_message(const char*s){(void)s;}
static uint8_t nor[0x100000],st_buf[3840];
static int writes,fail_write;
static int st_read(uint32_t o,void*d,uint32_t n){assert(o+n<=sizeof nor);memcpy(d,nor+o,n);return 0;}
static int st_erase(uint32_t o){assert(o==0xFB000);memset(nor+o,255,4096);return 0;}
static int st_prog(uint32_t o,const void*s,uint32_t n){uint32_t i;assert(o>=0xFB000 && o+n<=0xFC000);if(++writes==fail_write)return -1;for(i=0;i<n;i++)nor[o+i]&=((const uint8_t*)s)[i];return 0;}
static uint32_t st_crc32(const void*p,uint32_t n){uint32_t h=2166136261u;while(n--)h=(h^*(const uint8_t*)p)*16777619u,p=(const uint8_t*)p+1;return h;}
#include "../firmware/src/synthmaven_midi.h"
#include "../firmware/src/synthmaven_ble.c"
static int fake_state,connected_index=-1,starts;
void shim_timers(void){} void shim_run(void){} void shim_os_init(void){}
uint32_t shim_stacks_lo(void){return 0x1c50000;}
void btstack_init(void){starts++;} void bt_ble_init(void){} void lmp_set_sniff_disable(void){}
void bt_max_pwr_set(int a,int b,int c,int d){(void)a;(void)b;(void)c;(void)d;}
void lib_make_ble_address(uint8_t*a,uint8_t*b){memcpy(a,b,6);} int le_controller_set_mac(void*p){(void)p;return 0;}
int syscfg_read(uint16_t i,void*p,uint16_t n){(void)i;(void)p;(void)n;return -1;}
int syscfg_write(uint16_t i,void*p,uint16_t n){(void)i;(void)p;return n;}
void ble_midi_enable(int on){if(!on)fake_state=0;} void ble_midi_poll(void){}
void ble_midi_scan(int on){fake_state=on?1:0;} void ble_midi_disconnect(void){fake_state=0;}
int ble_midi_found(int i,char*n,int*r){if(i<0||i>1)return 0;str_cpy(n,i?"Chocolate Plus":"Chocolate",32);*r=-50;return 1;}
int ble_midi_connect(int i){if(i<0||i>1)return -1;connected_index=i;fake_state=2;return 0;}
static int peripheral_linked,peripheral_ready;
int ble_midi_central_state(void){return fake_state;}
int ble_midi_peripheral_connected(void){return peripheral_ready;}
int ble_midi_peripheral_linked(void){return peripheral_linked;}
void ble_midi_disconnect_peripheral(void){peripheral_linked=peripheral_ready=0;}
int main(void){
 uint8_t one[20],two[24],out[1536];uint32_t packet;struct sm_bt_event ready={3,{0},0};
 memset(nor,0xA5,sizeof nor);memset(nor+0xFB000,255,4096);memset(one,3,sizeof one);memset(two,5,sizeof two);
 assert(shim_store_load(out,sizeof out)==-1);assert(shim_store_save(one,sizeof one)==sizeof one);
 trk[0].v[0].active=1;sm_radio_flush();assert(writes==0);trk[0].v[0].active=0;sm_radio_flush();
 assert(shim_store_load(out,sizeof out)==sizeof one && !memcmp(out,one,sizeof one));
 shim_store_save(two,sizeof two);fail_write=writes+2;sm_radio_flush();
 assert(shim_store_load(out,sizeof out)==sizeof one && !memcmp(out,one,sizeof one));
 fail_write=0;sm_radio_flush();assert(shim_store_load(out,sizeof out)==sizeof two && !memcmp(out,two,sizeof two));
 assert(nor[0xFAFFF]==0xA5 && nor[0xFC000]==0xA5);
 assert(sm_set(1,1,1)==0);sm_ble_poll();assert(starts==1 && sm_ble_state()==1 && sm_radio_quiet);
 shim_bt_event(2,&ready,0);assert(!sm_ble_up);
 shim_bt_event(3,0,0);assert(!sm_ble_up);
 fm1_ms=16001;sm_ble_poll();assert(sm_ble_state()==7);
 assert(sm_ble_scan_start()==4 && sm_bluetooth); /* enabled but initialization timed out */
 shim_bt_event(3,&ready,0);sm_ble_poll();assert(sm_ble_state()==2 && !sm_radio_quiet);
 assert(!sm_ble_failed); /* late completion must recover, rather than report BT disabled */
 assert(sm_ble_scan_start()==0 && connected_index==-1);sm_ble_poll();assert(sm_ble_state()==3);
 assert(sm_ble_connect_device(1)==0 && connected_index==1 && sm_ble_state()==4);
 fake_state=4;assert(sm_ble_state()==6);
 fake_state=0;peripheral_linked=1;assert(sm_ble_state()==5);
 peripheral_ready=1;assert(sm_ble_state()==6);
 sm_ble_disconnect_device();assert(sm_ble_state()==2);
 fake_state=4;
 ble_midi_receive(0x0011C01Cu,123);assert(sm_ble_read(&packet));assert(((packet>>4)&15)==2 && sm_patch_accept(2) && !sm_patch_accept(0));
 assert(!sm_key_accept(2));
 ble_midi_receive(0x0011C01Cu,124);sm_bluetooth=0;sm_ble_poll();assert(!sm_ble_pending() && sm_ble_state()==0);
 puts("BLE adapter: journal/power interruption, musical-data bounds, deferred writes, explicit selection, zero-length SDK ready callback, late startup recovery, real-state reporting and cable-2 routing PASS");return 0;
}
