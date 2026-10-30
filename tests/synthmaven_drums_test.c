/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
int main(void)
{
    host_tracks_init();
    int32_t l[CTL]={0},r[CTL]={0};
    track_t saved[NPART];memcpy(saved,trk,sizeof saved);
    sm_drum_stop();sm_drums.enabled=0;
    sm_drum_mix(l,r,CTL,22050,1);
    for(uint32_t i=0;i<CTL;i++)assert(!l[i]&&!r[i]);
    sm_drums.enabled=1;
    uint32_t energy=0;
    for(uint32_t b=0;b<100;b++){
        memset(l,0,sizeof l);memset(r,0,sizeof r);
        sm_drum_mix(l,r,CTL,22050,1);
        for(uint32_t i=0;i<CTL;i++){energy|=(uint32_t)abs(l[i]);assert(l[i]==r[i]);assert(abs(l[i])<262144);}
    }
    assert(energy);assert(!memcmp(saved,trk,sizeof saved));
    assert(sm_drum_audio.step==((100u*CTL-1u)/(22050u/4u)+1u)%16u);
    sm_drums.enabled=0;sm_drum_mix(l,r,CTL,22050,1);
    assert(!sm_drum_audio.running);
    for(uint32_t lane=0;lane<SM_DRUM_LANES;lane++)assert(!sm_drum_audio.lane[lane].on);
    sm_drum_stop();memset(l,0,sizeof l);memset(r,0,sizeof r);
    sm_drum_hit(0,127);sm_drum_mix(l,r,CTL,22050,0);
    assert(sm_drum_audio.lane[0].on);
    uint8_t wire[SM_DRUM_WIRE];sm_drum_config_t decoded;
    sm_drum_encode(wire,&sm_drums);assert(sm_drum_decode(&decoded,wire));
    assert(!decoded.enabled && decoded.steps[0]==sm_drums.steps[0]);
    wire[33]=1;assert(!sm_drum_decode(&decoded,wire));wire[33]=0;
    wire[16]=128;assert(!sm_drum_decode(&decoded,wire));
    printf("Independent drums: audible, bounded, clocked, no four-part mutation; runtime %zu bytes, scratch %zu bytes\n",sizeof sm_drum_audio,sizeof sm_drum_scratch);
    return 0;
}
