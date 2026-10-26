/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
typedef uint8_t u8;typedef int8_t s8;
typedef struct {u8 event_type,address_type,address[6];s8 rssi;u8 length;u8 data[31];} adv_report_t;
static const u8 midi_svc_le[16]={0x00,0xC7,0xC4,0x4E,0xE3,0x6C,0x51,0xA7,0x33,0x4B,0xE8,0xED,0x5A,0x0E,0xB8,0x03};
#include "../firmware/vendor/fm1-base/midi/ble_midi_discovery.h"
static void named(adv_report_t*r,const char*name){u8 n=(u8)strlen(name);r->data[0]=3;r->data[1]=3;r->data[2]=0x12;r->data[3]=0x18;r->data[4]=n+1;r->data[5]=9;memcpy(r->data+6,name,n);r->length=n+6;}
int main(void){
 adv_report_t r={0};r.rssi=-86;r.address[0]=1;
 /* Reconstructed from the user's observed Plus advertisement: advertised service 1812, name FootCtrlPlus. */
 named(&r,"FootCtrlPlus");mc_adv(&r);assert(mc_n==1&&!strcmp(mc_dev[0].name,"FootCtrlPlus"));
 r.rssi=-48;mc_adv(&r);assert(mc_n==1&&mc_dev[0].rssi==-48);
 r.address[0]=2;named(&r,"Keyboard");mc_adv(&r);assert(mc_n==1);
 named(&r,"FootCtrl");mc_adv(&r);assert(mc_n==2);
 r.address[0]=3;named(&r,"Chocolate");mc_adv(&r);assert(mc_n==3);
 r.address[0]=4;named(&r,"footctrlplus");mc_adv(&r);assert(mc_n==4);
 r.address[0]=5;r.data[0]=17;r.data[1]=7;memcpy(r.data+2,midi_svc_le,16);r.length=18;mc_adv(&r);assert(mc_n==5);
 named(&r,"Other MIDI");mc_adv(&r);assert(mc_n==5&&!strcmp(mc_dev[4].name,"Other MIDI"));
 r.address_type=1;named(&r,"FootCtrlPlus");mc_adv(&r);assert(mc_n==6&&mc_dev[5].addr_type==1);
 r.address[0]=6;r.data[0]=30;r.length=3;mc_adv(&r);assert(mc_n==6);
 r.length=0;mc_adv(&r);assert(mc_n==6);
 for(int i=0;i<10;i++){r.address[0]=(u8)(10+i);named(&r,"FootCtrlPlus");mc_adv(&r);}assert(mc_n==MC_MAX);
 puts("BLE discovery: observed HID-only FootCtrlPlus, Chocolate names, standard MIDI, scan response, address types, bounds and capacity PASS");return 0;
}
