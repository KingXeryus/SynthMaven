/* SPDX-License-Identifier: GPL-3.0-only */
#define BACKUP_TEST_NO_MAIN
#include "backup_test.c"
#include <assert.h>
int main(void){
 reset();memset(sm_maps,0,sizeof sm_maps);memset(sm_program_meta,0,sizeof sm_program_meta);
 trk[0].p[P_LEVEL]=71;assert(!sm_scene_store(1,"VERSE"));assert(!sm_program_save(0,"PIANO",0));
 trk[0].p[P_LEVEL]=82;assert(!sm_program_save(1,"EP",1));
 sm_setlist_count=3;sm_setlist[0]=0;sm_setlist[1]=1;sm_setlist[2]=0;assert(!sm_config_save());
 static uint8_t archive[SM_PROGRAM_BANK];uint32_t len=0,crc=0;assert(!list(2,&len,&crc));assert(len==sizeof archive);
 for(uint32_t off=0;off<len;off+=256){uint32_t n=len-off>256?256:len-off;assert(!get(2,off,n));assert(ed_unpack7(rep+9,rep_n-9,archive+off,n)==n);}
 assert(st_crc32(archive,sizeof archive)==crc);
 assert(!put_all(3,archive,sizeof archive,crc));assert(!sm_program_load(2));assert(trk[0].p[P_LEVEL]==71);assert(!sm_program_load(3));assert(trk[0].p[P_LEVEL]==82);assert(!sm_scene_load(1));assert(trk[0].p[P_LEVEL]==71);
 uint8_t old=archive[4];archive[4]=99;assert(put_all(3,archive,sizeof archive,st_crc32(archive,sizeof archive))==2);archive[4]=old;assert(!sm_program_load(3)&&trk[0].p[P_LEVEL]==82);
 /* Runtime restore stages into the inactive copy without replacing saved programs. */
 static uint8_t runtime[BANK_STORE_SIZE];
 proj_cur=0;
 assert(!list(0,&len,&crc));assert(len==sizeof runtime);
 for(uint32_t off=0;off<len;off+=256){uint32_t n=len-off>256?256:len-off;assert(!get(0,off,n));assert(ed_unpack7(rep+9,rep_n-9,runtime+off,n)==n);}
 assert(!put_all(0,runtime,sizeof runtime,crc));
 assert(!sm_program_load(0)&&trk[0].p[P_LEVEL]==71);
 assert(!sm_program_load(1)&&trk[0].p[P_LEVEL]==82);
 persist_t preferences;settings_export(&preferences);
 preferences.ext.usb_off=WORK_BUDGET_TAG|(24u<<8); /* IN+OUT plus saved workload budget */
 assert(!put_all(1,&preferences,sizeof preferences,st_crc32(&preferences,sizeof preferences)));
 assert(settings_budget==24);
 sm_factory_unpack(&preferences,sizeof preferences,SM_FACTORY_SETTINGS,sizeof SM_FACTORY_SETTINGS);
 assert(settings_legacy_valid(&preferences));
 assert((preferences.ext.usb_off&3u)==0);
 uint8_t config[SM_CONFIG_BYTES];sm_factory_unpack(config,sizeof config,SM_FACTORY_CONFIG,sizeof SM_FACTORY_CONFIG);assert(sm_config_valid(config));
 printf("Program archive: %u-byte paired bank, USB export/restore, scenes and invalid-upload rollback passed\n",(unsigned)sizeof archive);return 0;
}
