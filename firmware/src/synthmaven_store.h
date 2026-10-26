/* SPDX-License-Identifier: GPL-3.0-only */
typedef struct { uint32_t magic; uint8_t patch_port,key_port,bluetooth,reserved; uint32_t crc; } sm_record_t;
static void sm_record_fill(sm_record_t *r) {
 memset(r,0,sizeof *r); r->magic=0x314D5353u;
 r->patch_port=sm_patch_port; r->key_port=sm_key_port; r->bluetooth=sm_bluetooth;
 r->crc=st_crc32(r,sizeof *r-4u);
}
static int sm_record_take(const sm_record_t *r) {
 if(r->magic!=0x314D5353u || r->crc!=st_crc32(r,sizeof *r-4u)) return 1;
 return (int)sm_set(r->patch_port,r->key_port,r->bluetooth);
}

/* Appended record: older routing settings retain their existing size and CRC. */
typedef struct {uint32_t magic;uint8_t map[4][4];uint32_t crc;} sm_pedal_record_t;
static void sm_pedal_fill(sm_pedal_record_t *r) {
 r->magic=0x31444C50u;memcpy(r->map,sm_pedal,sizeof sm_pedal);
 r->crc=st_crc32(r,sizeof *r-4u);
}
static int sm_pedal_take(const sm_pedal_record_t *r) {
 uint32_t i;
 if(r->magic!=0x31444C50u || r->crc!=st_crc32(r,sizeof *r-4u))return 1;
 for(i=0;i<4u;i++)if(r->map[i][0]>2u || r->map[i][1]>127u || r->map[i][2]>127u || r->map[i][3]>16u)return 1;
 memcpy(sm_pedal,r->map,sizeof sm_pedal);return 0;
}
