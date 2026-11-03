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
 /* Over SynthMaven 3.5+ (settings carry the owner tag, either copy): nothing is erased. */
 marker_erase_stuck=0;
 for(uint32_t copy=0;copy<2u;copy++){
  memset(nor,0x5A,sizeof nor);uint8_t *h=nor+0xFC000u+copy*4096u;memset(h,0xFF,4096u);
  const uint32_t w[]={0x554C4546u,(copy<<16),7u,604u,0u,0u,0u};memcpy(h,w,sizeof w);
  uint32_t c=ldr_crc32(h,28u);memcpy(h+28,&c,4);memcpy(h+368,"MDS\1",4);
  erases=0;open=0;assert(!ldr_factory_reset(cur,1,&open)&&!erases&&!open);
  /* a broken header or another owner: still reset */
  h[8]^=1;erases=0;assert(!ldr_factory_reset(cur,1,&open)&&erases);
 }
 memset(nor,0x5A,sizeof nor);memset(nor+0xFC000u,0xFF,4096u);
 {const uint32_t w[]={0x554C4546u,0u,7u,604u,0u,0u,0u};memcpy(nor+0xFC000u,w,sizeof w);uint32_t c=ldr_crc32(nor+0xFC000u,28u);memcpy(nor+0xFC000u+28,&c,4);memcpy(nor+0xFC000u+368,"XYZ\1",4);}
 erases=0;assert(!ldr_factory_reset(cur,1,&open)&&erases);
 puts("Update over SynthMaven 3.5+: musical storage kept; other or broken settings: reset");
 puts("Factory reset: all musical ranges erased/verified, staging/head/reserved preserved, idempotent retry, failed marker stops reset");return 0;
}
