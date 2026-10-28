/* SPDX-License-Identifier: GPL-3.0-only */
/* Appended settings record: absent records retain default enabled tuning. */
typedef struct {uint32_t magic,bypass,crc;} master_tuning_record_t;
static void master_tuning_fill(master_tuning_record_t *r){r->magic=0x3254554Du;r->bypass=(master_tuning_bypass&15u)|((uint32_t)tune_floor_octave<<4);r->crc=st_crc32(r,8);}
static int master_tuning_take(const master_tuning_record_t *r){if((r->magic!=0x3154554Du && r->magic!=0x3254554Du) || (r->magic==0x3154554Du?r->bypass>=16u:(r->bypass>>4)>8u) || r->crc!=st_crc32(r,8))return 1;master_tuning_bypass=(uint8_t)(r->bypass&15u);tune_floor_octave=r->magic==0x3154554Du?0u:(uint8_t)(r->bypass>>4);return 0;}
