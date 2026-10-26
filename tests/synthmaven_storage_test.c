/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
static int st_read(uint32_t o,void*d,uint32_t n){return -1;}
static int st_erase(uint32_t o){return -1;}
static int st_prog(uint32_t o,const void*d,uint32_t n){return -1;}
#include "../firmware/src/storage.c"
static uint32_t fm1_ms;
#include "../firmware/src/synthmaven_midi.h"
#include "../firmware/src/synthmaven_store.h"
int main(void) {
 sm_pedal_record_t p;
 sm_pedal[0][0]=1;sm_pedal[0][1]=80;sm_pedal[0][2]=127;sm_pedal[0][3]=2;
 sm_pedal_fill(&p);memset(sm_pedal,0,sizeof sm_pedal);
 if(sm_pedal_take(&p) || sm_pedal[0][1]!=80 || sm_pedal[0][3]!=2)return 4;
 p.crc^=1;if(!sm_pedal_take(&p))return 5;
 memset(&p,255,sizeof p);if(!sm_pedal_take(&p))return 6;
 sm_pedal_fill(&p);p.map[3][3]=17;p.crc=st_crc32(&p,sizeof p-4u);if(!sm_pedal_take(&p))return 7;
 sm_record_t r; sm_set(0,1,0); sm_record_fill(&r); sm_set(2,2,0);
 if(sm_record_take(&r) || sm_patch_port!=0 || sm_key_port!=1) return 1;
 r.crc ^=1; sm_set(2,2,0);
 if(!sm_record_take(&r) || sm_patch_port!=0 || sm_key_port!=2) return 2;
 memset(&r,255,sizeof r);
 if(!sm_record_take(&r)) return 3;
 puts("SynthMaven settings: round-trip, corrupt CRC, erased legacy record PASS"); return 0;
}
