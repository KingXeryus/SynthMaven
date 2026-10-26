/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint8_t nor[0x100000];
static int st_read(uint32_t off,void *p,uint32_t n){memcpy(p,nor+off,n);return 0;}
static int st_erase(uint32_t off){memset(nor+off,255,4096);return 0;}
static int st_prog(uint32_t off,const void *p,uint32_t n){const uint8_t *s=p;while(n--)nor[off++]&=*s++;return 0;}
#include "../firmware/src/storage.c"
#include "../firmware/src/custom_scale_store.h"
#include "../firmware/src/master_tuning_store.h"
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
 master_tuning_record_t mt;master_tuning_bypass=5;master_tuning_fill(&mt);
 check(!st_save(OBJ_SETTINGS,&mt,sizeof mt),"master tuning switches saved through atomic flash storage");
 master_tuning_bypass=0;memset(&mt,0,sizeof mt);check(st_load(OBJ_SETTINGS,&mt,sizeof mt)==sizeof mt&&!master_tuning_take(&mt)&&master_tuning_bypass==5,"master tuning switches restore after a settings reload");
 mt.crc^=1;master_tuning_bypass=0;check(master_tuning_take(&mt)&&!master_tuning_bypass,"corrupt switch settings retain default enabled tuning");
 memset(&mt,255,sizeof mt);check(master_tuning_take(&mt)&&!master_tuning_bypass,"erased legacy settings retain default enabled tuning");
 program_v2_t v2;memcpy(&v2,&a,sizeof v2-4);v2.magic=0x32475250u;v2.size=sizeof v2;v2.sum=program_hash(&v2,sizeof v2-4);check(program_import(&b,&v2,sizeof v2)&&!b.scenes[0].valid&&b.sound[1].p[P_LEVEL]==80,"real PRG2 migrates sounds and empty scenes");
 program_bank_v1_t oldbank;memset(&oldbank,0,sizeof oldbank);oldbank.magic=0x314B4250u;
 program_bank_t migration;memset(&migration,0,sizeof migration);migration.magic=PROGRAM_BANK_MAGIC;check(program_pack(&migration.slot[0],&a,"OLD PROGRAM"),"prepare named legacy fixture");memcpy(&oldbank.slot[0],&migration.slot[0],sizeof oldbank.slot[0]);
 check(program_bank_import(&migration,&oldbank,sizeof oldbank,&b)&&program_unpack(&b,&migration.slot[0])&&!b.scenes[0].valid&&b.sound[1].p[P_LEVEL]==80,"real PBK1 named pair migrates without touching sample regions");
 program_bank_t bank,loaded;memset(&bank,0,sizeof bank);bank.magic=PROGRAM_BANK_MAGIC;
 for(uint32_t e=0;e<NENGINES;e++)for(uint32_t k=0;k<8;k++){const param_desc_t *d=&ENGINES[e]->edit[k];check(d->min>=-128&&d->max<=127,"engine parameter range fits lossless packed program storage");}
 for(uint32_t k=0;k<P_E0;k++)check(TP[k].min>=-128&&TP[k].max<=127,"shared parameter range fits lossless packed program storage");
 for(uint32_t pair=0;pair<4;pair++){
  char name1[13]="NAMED LOW",name2[13]="NAMED HIGH";name1[9]=(char)('1'+pair);name2[10]=(char)('5'+pair);
  memset(&bank,0,sizeof bank);bank.magic=PROGRAM_BANK_MAGIC;
  check(program_pack(&bank.slot[0],&a,name1)&&program_pack(&bank.slot[1],&a,name2)&&program_bank_valid(&bank,&b),"two named programs validate in one existing flash object");
  check(!st_save(OBJ_PROJECT0+pair,&bank,sizeof bank),"named pair commits through existing A/B sectors");memset(&loaded,0,sizeof loaded);
  check(st_load(OBJ_PROJECT0+pair,&loaded,sizeof loaded)==sizeof loaded && program_bank_valid(&loaded,&b),"all eight named programs survive flash reload");
  check(program_unpack(&b,&loaded.slot[0])&&!memcmp(&a,&b,sizeof a)&&!strcmp(loaded.slot[0].name,name1),"lower named program round trips every parameter, patch and tuning bit");
  check(program_unpack(&b,&loaded.slot[1])&&!memcmp(&a,&b,sizeof a)&&!strcmp(loaded.slot[1].name,name2),"upper named program round trips every parameter, patch and tuning bit");
  b.sound[0].p[P_PAN]=17;b.sum=program_hash(&b,sizeof b-4u);check(program_pack(&loaded.slot[1],&b,"NEW UPPER")&&!st_save(OBJ_PROJECT0+pair,&loaded,sizeof loaded),"updating a paired upper program succeeds atomically");
  check(st_load(OBJ_PROJECT0+pair,&bank,sizeof bank)==sizeof bank&&program_unpack(&b,&bank.slot[0])&&!memcmp(&a,&b,sizeof a)&&!strcmp(bank.slot[0].name,name1),"saving upper slot preserves paired original slot and its name");
 }
 bank.slot[1].name[0]=1;check(!program_bank_valid(&bank,&b),"invalid program name rejects the bank before writing");
 check(!program_pack(&bank.slot[1],&a,"TOO LONG NAME!"),"overlength program names are rejected");
 check(!st_save(OBJ_PROJECT0,&a,sizeof a),"legacy PRG2 program remains writable at its original address");
 memset(&bank,0,sizeof bank);bank.magic=PROGRAM_BANK_MAGIC;check(program_pack(&bank.slot[0],&a,"ORIGINAL")&&program_pack(&bank.slot[1],&a,"ADDED SLOT"),"first migration preserves the original program alongside its added pair");
 check(!st_save(OBJ_PROJECT0,&bank,sizeof bank),"migration uses the existing atomic A/B commit");st_hdr_t head;int current=st_current(OBJ_PROJECT0,&head);nor[st_sector(OBJ_PROJECT0,(uint32_t)current)+ST_PAYLOAD_OFF]^=1u;
 memset(&b,0,sizeof b);check(st_load(OBJ_PROJECT0,&b,sizeof b)==sizeof b&&program_valid(&b)&&!memcmp(&a,&b,sizeof a),"interrupted/corrupt migration falls back to the intact original PRG2 record");
 /* Both old D/E IDs retain their full saved sound while receiving the compact catalogue ID. */
 for(uint32_t w=0;w<SM_OR_WORLDS;w++)for(uint32_t style=0;style<2;style++){
  uint32_t pi,e=sm_or_at(w,&pi),base=ENGINES[e]->npresets;set_engine_of(&trk[0],e);apply_preset_to(&trk[0],pi);trk[0].p[P_MAC1]=(int16_t)(style?12:-6);program_capture(&a);
  uint32_t old=base+2u*(pi-base)+style;a.sound[0].preset=(uint8_t)old;a.sound[0].reserved&=(uint8_t)~2u;a.sum=program_hash(&a,sizeof a-4u);
  check(program_valid(&a),"legacy regional preset ID remains valid");program_apply(&a);check(trk[0].preset==pi&&!memcmp(trk[0].p,a.sound[0].p,sizeof trk[0].p),"old D/E program preserves complete sound with consolidated ID");
  program_capture(&b);check((b.sound[0].reserved&2u)&&b.sound[0].preset==pi&&program_valid(&b),"saving migrated program marks compact catalogue IDs");
 }
 for(uint32_t i=0;i<FAV_N;i++)fav[i]=FAV_NONE;fav[0]=NBASE;fav[1]=NBASE+1;fav[2]=NBASE+86;fav[3]=NBASE+87;uint32_t favorites[FAV_N];check(fav_list(favorites)==2&&favorites[0]==NBASE&&favorites[1]==NBASE+43,"old D/E favourites map to consolidated recipes and deduplicate");
 {uint32_t pi,e=sm_or_at(0,&pi);set_engine_of(TSEL,e);apply_preset_to(TSEL,pi);check(fav_has(fav_key()),"legacy expressive favourite is recognized");fav_toggle(0);check(fav_list(favorites)==1&&favorites[0]==NBASE+43,"removing a consolidated favourite removes both legacy variants");}
 printf("Named program bank size %zu bytes; original project addresses and sample regions unchanged\n",sizeof bank);
 return fails?1:0;
}
