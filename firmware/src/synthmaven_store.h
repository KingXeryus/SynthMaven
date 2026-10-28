/* SPDX-License-Identifier: GPL-3.0-only */
typedef struct { uint32_t magic; uint8_t patch_port,key_port,bluetooth,reserved; uint32_t crc; } sm_record_t;
static void sm_record_fill(sm_record_t *r) {
 memset(r,0,sizeof *r); r->magic=0x314D5353u;
 r->patch_port=sm_patch_port; r->key_port=sm_key_port; r->bluetooth=0; r->reserved=sm_usb_host;
 r->crc=st_crc32(r,sizeof *r-4u);
}
static int sm_record_take(const sm_record_t *r) {
 if(r->magic!=0x314D5353u || r->crc!=st_crc32(r,sizeof *r-4u)) return 1;
 if(r->key_port>2u || r->reserved>1u)return 1;
 sm_usb_host=r->reserved; return (int)sm_set(0,r->key_port,0);
}

/* Appended record: older routing settings retain their existing size and CRC. */
typedef struct {uint32_t magic;uint8_t map[4][4];uint32_t crc;} sm_pedal_record_t;
static void sm_pedal_fill(sm_pedal_record_t *r) {
 r->magic=0x31444C50u;memcpy(r->map,sm_pedal,sizeof r->map);
 r->crc=st_crc32(r,sizeof *r-4u);
}
static int sm_pedal_take(const sm_pedal_record_t *r) {
 uint32_t i;
 if(r->magic!=0x31444C50u || r->crc!=st_crc32(r,sizeof *r-4u))return 1;
 for(i=0;i<4u;i++)if(r->map[i][0]>2u || r->map[i][1]>127u || r->map[i][2]>127u || r->map[i][3]>16u)return 1;
 memcpy(sm_pedal,r->map,sizeof r->map);return 0;
}

#define SM_PERF_MAGIC 0x31465250u
/* Settings tail v1: no sample space or new flash sectors. */
typedef struct {uint32_t magic;uint16_t repeat;uint8_t len,reserved;uint8_t setlist[32],map[SM_ACTIONS-4][4];uint32_t crc;} sm_performance_record_t;
static void sm_performance_fill(sm_performance_record_t *r){memset(r,0,sizeof *r);r->magic=SM_PERF_MAGIC;r->repeat=sm_repeat_ms;r->len=sm_setlist_len;r->reserved=sm_controller_override|(sm_controller_port<<2);memcpy(r->setlist,sm_setlist,32);memcpy(r->map,sm_pedal+4,sizeof r->map);r->crc=st_crc32(r,sizeof *r-4);}
static int sm_performance_take(const sm_performance_record_t *r){uint32_t i;if(r->magic!=SM_PERF_MAGIC||r->crc!=st_crc32(r,sizeof *r-4)||r->repeat>500||r->len>32||(r->reserved>>2)>16u)return 1;for(i=0;i<32;i++)if(r->setlist[i]>=8)return 1;for(i=0;i<SM_ACTIONS-4;i++)if(r->map[i][0]>2||r->map[i][1]>127||r->map[i][2]>127||r->map[i][3]>16)return 1;sm_controller_override=r->reserved&3u;sm_controller_port=r->reserved>>2;sm_repeat_ms=r->repeat;sm_setlist_len=r->len;memcpy(sm_setlist,r->setlist,32);memcpy(sm_pedal+4,r->map,sizeof r->map);memset(sm_repeat_valid,0,sizeof sm_repeat_valid);return 0;}
