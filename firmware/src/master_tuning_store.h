/* SPDX-License-Identifier: GPL-3.0-only */
/* Appended settings record: absent records retain default enabled tuning. */
typedef struct {uint32_t magic,bypass,crc;} master_tuning_record_t;
static void master_tuning_fill(master_tuning_record_t *r){r->magic=0x3154554Du;r->bypass=master_tuning_bypass&15u;r->crc=st_crc32(r,8);}
static int master_tuning_take(const master_tuning_record_t *r){if(r->magic!=0x3154554Du || r->bypass>=16u || r->crc!=st_crc32(r,8))return 1;master_tuning_bypass=(uint8_t)r->bypass;return 0;}
