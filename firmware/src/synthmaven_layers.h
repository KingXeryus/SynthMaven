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

/* Device-wide MIDI assignments: layers 1..4, then the drummer (the first DRUM-engine part).
 * input: 0 any, 1 USB, 2 TRS (midi_event_source). channel: 0 off, 1..16, 17 all. Disabled: ROUT applies.
 * Note ranges stay sm_layers' (they travel with a program); two layers on one input/channel split or stack. */
#define SM_ROUTE_DRUM 4u
#define SM_ROUTE_ALL 17u
typedef struct { uint8_t enabled, input[5], channel[5]; } sm_routes_t;
#define SM_ROUTES_DEFAULT ((sm_routes_t){.channel={1,2,3,4,10}})
static sm_routes_t sm_routes=SM_ROUTES_DEFAULT;
static uint32_t sm_route_rows(const sm_routes_t *r,uint32_t source,uint32_t ch)
{
    uint32_t rows=0;
    for(uint32_t i=0;i<5u;i++)
        if((!r->input[i]||r->input[i]==source)&&r->channel[i]&&(r->channel[i]==SM_ROUTE_ALL||r->channel[i]==ch+1u))rows|=1u<<i;
    return rows;
}
#define SM_ROUTES_WIRE 16u
static int sm_routes_valid(const sm_routes_t *r){if(r->enabled>1)return 0;for(uint32_t i=0;i<5;i++)if(r->input[i]>2||r->channel[i]>SM_ROUTE_ALL)return 0;return 1;}
static void sm_routes_encode(uint8_t *b,const sm_routes_t *r){b[0]='S';b[1]='M';b[2]='R';b[3]=1;b[4]=r->enabled;for(uint32_t i=0;i<5;i++){b[5+i]=r->input[i];b[10+i]=r->channel[i];}b[15]=0;}
static int sm_routes_decode(sm_routes_t *r,const uint8_t *b){if(b[0]!='S'||b[1]!='M'||b[2]!='R'||b[3]!=1||b[15])return 0;sm_routes_t v={.enabled=b[4]};for(uint32_t i=0;i<5;i++){v.input[i]=b[5+i];v.channel[i]=b[10+i];}if(!sm_routes_valid(&v))return 0;*r=v;return 1;}
