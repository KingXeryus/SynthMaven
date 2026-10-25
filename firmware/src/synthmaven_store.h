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
