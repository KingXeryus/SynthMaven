/* SPDX-License-Identifier: GPL-3.0-only */
/* Extension after the preserved 64-byte legacy settings tail. */
#define CUSTOM_SCALE_MAGIC 0x314C4353u
#define CUSTOM_SCALE_OFF 64u
typedef struct { uint32_t magic,step[4],crc; } custom_scale_record_t;
_Static_assert(sizeof(custom_scale_record_t)==24u,"custom scale storage layout");
static void custom_scale_fill(custom_scale_record_t *r) {
 r->magic=CUSTOM_SCALE_MAGIC;memcpy(r->step,custom_scale,sizeof r->step);
 r->crc=st_crc32(r,sizeof *r-4u);
}
static int custom_scale_take(const custom_scale_record_t *r) {
 uint32_t i;if(r->magic!=CUSTOM_SCALE_MAGIC || r->crc!=st_crc32(r,sizeof *r-4u))return 1;
 for(i=0;i<4;i++)if(r->step[i]&0xFF000000u)return 1;
 memcpy(custom_scale,r->step,sizeof r->step);return 0;
}
