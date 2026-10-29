/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define SM_BLE_HOST_TEST 1
#define BLE_API static
#define RING_PUBLISH() do{}while(0)
static uint32_t fm1_ms;
static uint8_t sm_bluetooth=1,sm_ble_started=1,sm_ble_on=1,connected=1;
static int ble_connected(void){return connected;}
static int ble_midi_ready(void){return connected;}
static void fm1_ble_irqs_hold(int x){(void)x;}
#include "../firmware/src/synthmaven_ble_editor.h"
static void feed(const uint8_t *s,uint32_t n){
 for(uint32_t off=0;off<n;off+=3){uint32_t k=n-off;if(k>3)k=3;uint32_t p=off+k==n?4+k:4;for(uint32_t i=0;i<k;i++)p|=(uint32_t)s[off+i]<<(8*(i+1));ble_app_midi_in(p,0,0);}
}
int main(void){
 uint8_t frame[2050]={240,125,70,76,1};const uint8_t *p;uint32_t n;
 for(uint32_t size=6;size<=2050;size++){
  for(uint32_t i=5;i<size-1;i++)frame[i]=(uint8_t)(i&127);frame[size-1]=247;
  feed(frame,size);assert(sm_ble_editor_frame(&p,&n)&&n==size-2&&!memcmp(p,frame+1,n));
  uint8_t other[6]={240,125,70,76,2,247};feed(other,6);assert(p[3]==1); /* pending request cannot be overwritten */
  sm_ble_editor_done();sm_ble_editor_send(frame,size);uint8_t out[2050];uint32_t used=0,pkt,ts;
  while(ble_app_midi_peek(&pkt,&ts)){uint32_t cin=pkt&15,k=cin==4?3:cin-4;for(uint32_t i=0;i<k;i++)out[used++]=(uint8_t)(pkt>>(8*(i+1)));ble_app_midi_pop();}
  assert(used==size&&!memcmp(out,frame,size)&&!sm_ble_editor_busy());
 }
 uint8_t invalid[]={240,1,2,3,4,247};feed(invalid,sizeof invalid);assert(!sm_ble_editor_frame(&p,&n));
 uint8_t start[]={240,125,70};feed(start,3);assert(sm_rx_open);connected=0;ble_app_state();assert(!sm_rx_open&&!sm_rx_ready&&!sm_ble_editor_busy());
 puts("BLE editor: all frame lengths, fragmentation, immutable pending request, exact output, foreign envelope and disconnect reset PASS");
}
