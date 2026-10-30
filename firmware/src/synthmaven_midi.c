/* SPDX-License-Identifier: GPL-3.0-only */
/* Bounded audio-side match/capture only. Foreground dispatch performs loads. */
#define SM_MAP_COUNT 24u
#define SM_MAP_BYTES 12u
#define SM_MAP_Q 16u
static uint8_t sm_maps[SM_MAP_COUNT][SM_MAP_BYTES];
static uint32_t sm_map_last_ms[SM_MAP_COUNT],sm_map_seen,sm_map_released;
static uint32_t sm_map_q[SM_MAP_Q];
static volatile uint32_t sm_map_w,sm_map_r;
static volatile uint8_t sm_map_learn,sm_map_generation;
static volatile uint32_t sm_midi_learned;
static volatile uint32_t sm_midi_last,sm_midi_serial,sm_map_hits,sm_map_drops;
static volatile uint8_t sm_midi_match=127;
static uint8_t sm_setlist[32],sm_setlist_count,sm_setlist_pos;
/* Record byte 11: input source, 0 any (legacy), 1 USB, 2 TRS.
 * Pedal actions can be isolated from keyboard notes on the other connector. */
/* actions: patch +/-; program +/-; scene +/-; layers toggle; setlist +/-; panic */
static int sm_map_valid(const uint8_t *m)
{
    return m[0]<=1&&m[1]<=2&&m[2]<=16&&m[3]<=127&&m[4]<=128&&m[5]<=1&&m[6]<=4&&
      (m[5]?m[7]<P_COUNT:m[7]<10)&&((uint32_t)m[8]|(uint32_t)m[9]<<8)<=2000u&&
      (m[10]<NENGINES||m[10]==255)&&m[11]<=2&&(!m[5]||m[1]==0);
}
static int sm_map_conflict(uint32_t slot,const uint8_t *m)
{
    if(!m[0])return -1;
    for(uint32_t i=0;i<SM_MAP_COUNT;i++){
        const uint8_t *p=sm_maps[i];if(i==slot||!p[0])continue;
        if((!p[11]||!m[11]||p[11]==m[11])&&p[1]==m[1]&&p[3]==m[3]&&(p[2]==m[2]||p[2]==16||m[2]==16)&&
           (p[4]==m[4]||p[4]==128||m[4]==128))return (int)i;
    }
    return -1;
}
static int sm_map_receive(uint32_t st,uint32_t ch,uint32_t number,uint32_t value,uint32_t source)
{
    uint32_t type=st==0xb0?0:st==0xc0?1:(st==0x90||st==0x80)?2:3;
    if(type==3)return 0;if(type==1)value=0;if(st==0x80)value=0;
    uint32_t release=type==2&&!value;
    sm_midi_last=number|(value<<7)|(ch<<14)|(type<<18)|(source<<20);sm_midi_match=127;
    RING_PUBLISH();sm_midi_serial++;
    if(sm_map_learn){if(sm_map_learn==1&&!release&&(type!=0||value)){sm_midi_learned=sm_midi_last;sm_map_learn=2;}return 0;}
    for(uint32_t i=0;i<SM_MAP_COUNT;i++){
        const uint8_t *m=sm_maps[i];if(!m[0]||(m[11]&&m[11]!=source)||m[1]!=type||m[3]!=number||(m[2]!=16&&m[2]!=ch))continue;
        /* A released switch rearms the next tap, even inside the duplicate window. */
        if(release||(!m[5]&&type==0&&!value&&m[4]!=0)){sm_map_released|=1u<<i;return 1;}
        if(m[4]!=128&&m[4]!=value)continue;
        sm_midi_match=(uint8_t)i;uint32_t bit=1u<<i,window=m[8]|(uint32_t)m[9]<<8;
        if(!m[5]&&(sm_map_seen&bit)&&!(sm_map_released&bit)&&window&&fm1_ms-sm_map_last_ms[i]<window)return 1;
        sm_map_last_ms[i]=fm1_ms;sm_map_seen|=bit;sm_map_released&=~bit;
        uint32_t w=sm_map_w;if(w-sm_map_r>=SM_MAP_Q){sm_map_drops++;return 1;}
        sm_map_q[w%SM_MAP_Q]=i|(value<<5)|((uint32_t)sm_map_generation<<12);RING_PUBLISH();sm_map_w=w+1;sm_map_hits++;return 1;
    }
    return 0;
}
