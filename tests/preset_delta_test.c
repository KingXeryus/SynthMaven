/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define FM6_PR(name,kind)
#include "melodee_fm6.h"
#include "melodee_prophet_factory.h"
int main(void){
 uint8_t row[128];unsigned bad=0;
 for(unsigned k=0;k<8+FM6_NSTOCK;k++){
  FM6_DELTA_read(k,row);
  bad+=memcmp(row,k<8?FM6_FACTORY[k]:FM6_STOCK[k-8],128)!=0;
 }
 for(unsigned k=0;k<sizeof P5_FPACK/P5_FACTORY_BYTES;k++){
  P5_DELTA_read(k,row);bad+=memcmp(row,P5_FPACK+k*P5_FACTORY_BYTES,P5_FACTORY_BYTES)!=0;
 }
 printf("Native factory delta packing: %u byte-mismatch rows\n",bad);return !!bad;
}
