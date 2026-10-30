/* SPDX-License-Identifier: GPL-3.0-only */
#define SYNTHMAVEN_FACTORY_RESET 1
#define main upstream_loader_main
#include "ldr_test.c"
#undef main
#include <assert.h>
int main(void){
 static uint8_t cur[4096];uint32_t open=0;
 memset(nor,0x5A,sizeof nor);bad_range=0;
 assert(ldr_factory_reset(cur,1,&open)==0&&open);
 for(uint32_t p=0;p<sizeof nor;p++){
  int erased=(p>=0x93000u&&p<0xE0000u)||(p>=0xE5000u&&p<0xFB000u)||(p>=0xFC000u&&p<0xFF000u)||(p>=LDR_MARK_SEC&&p<LDR_MARK_SEC+4096u);
  assert(nor[p]==(erased?255:0x5A));
 }
 erases=0;assert(!ldr_factory_reset(cur,1,&open)&&!erases);
 /* If the boot marker cannot be invalidated, no musical data may be erased. */
 memset(nor,0x5A,sizeof nor);open=0;marker_erase_stuck=1;
 assert(ldr_factory_reset(cur,1,&open)==-12);
 for(uint32_t p=0x93000u;p<sizeof nor;p++)assert(nor[p]==0x5A);
 puts("Factory reset: all musical ranges erased/verified, staging/head/reserved preserved, idempotent retry, failed marker stops reset");return 0;
}
