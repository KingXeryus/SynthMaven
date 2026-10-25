/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
static int st_read(uint32_t o,void*d,uint32_t n){return -1;}
static int st_erase(uint32_t o){return -1;}
static int st_prog(uint32_t o,const void*d,uint32_t n){return -1;}
#include "../firmware/src/storage.c"
#include "../firmware/src/synthmaven_midi.h"
#include "../firmware/src/synthmaven_store.h"
int main(void) {
 sm_record_t r; sm_set(0,1,0); sm_record_fill(&r); sm_set(2,2,0);
 if(sm_record_take(&r) || sm_patch_port!=0 || sm_key_port!=1) return 1;
 r.crc ^=1; sm_set(2,2,0);
 if(!sm_record_take(&r) || sm_patch_port!=2 || sm_key_port!=2) return 2;
 memset(&r,255,sizeof r);
 if(!sm_record_take(&r)) return 3;
 puts("SynthMaven settings: round-trip, corrupt CRC, erased legacy record PASS"); return 0;
}
