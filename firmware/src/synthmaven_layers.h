/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
/* Admission remains the engine's shared weighted voice budget. Layer routing
 * never raises it. Splits are inclusive MIDI input ranges, before tuning. */
typedef struct { uint8_t enabled, low[4], high[4]; } sm_layers_t;
static sm_layers_t sm_layers={.high={127,127,127,127}};
static uint32_t sm_layer_mask(uint32_t note,uint32_t parts)
{
    uint32_t mask=0;
    for(uint32_t i=0;i<4u && i<parts;i++)
        if(note>=sm_layers.low[i] && note<=sm_layers.high[i])mask|=1u<<i;
    return mask;
}
#define SM_LAYERS_WIRE 16u
static int sm_layers_valid(const sm_layers_t *s){if(s->enabled>1)return 0;for(uint32_t i=0;i<4;i++)if(s->low[i]>s->high[i]||s->high[i]>127)return 0;return 1;}
static void sm_layers_encode(uint8_t *b,const sm_layers_t *s){for(uint32_t i=0;i<16;i++)b[i]=0;b[0]='S';b[1]='M';b[2]='L';b[3]=1;b[4]=s->enabled;for(uint32_t i=0;i<4;i++){b[5+i]=s->low[i];b[9+i]=s->high[i];}}
static int sm_layers_decode(sm_layers_t *s,const uint8_t *b){if(b[0]!='S'||b[1]!='M'||b[2]!='L'||b[3]!=1||b[13]||b[14]||b[15])return 0;sm_layers_t v={.enabled=b[4]};for(uint32_t i=0;i<4;i++){v.low[i]=b[5+i];v.high[i]=b[9+i];}if(!sm_layers_valid(&v))return 0;*s=v;return 1;}
