/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define SM_PACKED_WT 1
#define SM_WT_TEST 1
#include "felucca_wavetables.h"
#include "wt_rom.h"
int main(void) {
 uint8_t scratch[8192];int16_t decoded[WT_NF*WT_NH*2];unsigned t;
 for(t=0;t<WT_NFACTORY;t++) if(!wt_rom_decode(t,scratch,decoded) || memcmp(decoded,WT_SPEC[t],sizeof decoded))return 1;
 puts("WAVE ROM: all 16 tables / 256 frames decode bit-for-bit identically PASS");
 return 0;
}
