/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint8_t nor[0x100000];
static int st_read(uint32_t off,void *p,uint32_t n){memcpy(p,nor+off,n);return 0;}
static int st_erase(uint32_t off){memset(nor+off,255,4096);return 0;}
static int st_prog(uint32_t off,const void *p,uint32_t n){const uint8_t *s=p;while(n--)nor[off++]&=*s++;return 0;}
#include "../firmware/src/storage.c"
#include "../firmware/src/custom_scale_store.h"
int main(void){
 program_t a,b;custom_scale_record_t scales;
 memset(nor,255,sizeof nor);bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();
 outdir="build/host";song.sel=3;program_layer=1;custom_scale[2]=0x654321u;
 program_route[1]=(program_route_t){1,1,1,61,-12};trk[1].p[P_LEVEL]=80;trk[1].p[P_PAN]=-32;song.solo=2;
 program_capture(&a);check(program_valid(&a),"program with custom scales validates");
 check(!st_save(OBJ_PROJECT0,&a,sizeof a),"program saved with atomic object store");
 memset(&b,0,sizeof b);check(st_load(OBJ_PROJECT0,&b,sizeof b)==sizeof b && program_valid(&b) && !memcmp(&a,&b,sizeof a),"four-sound program round trips through flash");
 custom_scale[2]=0;program_apply(&b);check(custom_scale[2]==0x654321u && program_layer && song.sel==3,"program restores custom definitions, layer mode and focus");
 check(b.route[1].split&&b.route[1].upper&&b.route[1].point==61&&b.route[1].transpose==-12&&trk[1].p[P_LEVEL]==80&&trk[1].p[P_PAN]==-32&&song.solo==2,"program restores split zone, transpose, level, pan and solo");
 program_v1_t old;memcpy(&old,&a,sizeof old-4);old.magic=0x31475250u;old.size=sizeof old;old.sum=program_hash(&old,sizeof old-4);
 check(program_import(&b,&old,sizeof old)&&b.route[1].enabled&&!b.route[1].split&&b.route[1].transpose==0&&b.sound[1].p[P_LEVEL]==80&&b.sound[1].p[P_PAN]==-32,"PRG1 import preserves sounds and mix, initializes full keyboard routing");
 old.sum^=1;check(!program_import(&b,&old,sizeof old),"corrupt legacy program is rejected");
 b=a;b.route[3].transpose=25;b.sum=program_hash(&b,sizeof b-4);check(!program_valid(&b),"invalid routing rejected even with valid checksum");
 b=a;b.sound[3].engine=255;b.sum=program_hash(&b,sizeof b-4);check(!program_valid(&b),"bad fourth engine rejected even with valid checksum");
 b=a;b.custom[0]=0xFF000000;b.sum=program_hash(&b,sizeof b-4);check(!program_valid(&b),"out-of-range custom scale bits rejected");
 custom_scale_fill(&scales);memset(custom_scale,0,sizeof custom_scale);check(!custom_scale_take(&scales) && custom_scale[2]==0x654321u,"four custom scales round trip through settings record");
 scales.crc^=1;check(custom_scale_take(&scales),"corrupt custom settings rejected");
 return fails?1:0;
}
