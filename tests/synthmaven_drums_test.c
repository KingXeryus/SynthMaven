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
    /* factory grooves: browsing keeps the player's pattern, an edit makes a groove the player's */
    {
        uint16_t own[SM_DRUM_LANES];
        sm_drums.steps[5]=0x0F0F;memcpy(own,sm_drums.steps,sizeof own);sm_drum_groove=0;
        for(uint32_t i=0;i<SM_GROOVES;i++){
            sm_groove_turn(1);
            assert(sm_groove_current()==i+1u&&sm_drums.steps[0]==sm_groove_steps[i][0]);
            assert(sm_groove_steps[i][0]&1u);          /* every groove opens on its kick */
            if(i>=4u)assert(sm_groove_steps[i][5]==sm_groove_steps[i][0]&&sm_groove_steps[i][1]==sm_groove_steps[i][7]&&sm_groove_steps[i][2]==sm_groove_steps[i][7]);   /* Arabic: dum KICK+TOM, tak RIM+SNARE+HAT */
        }
        sm_groove_turn(1);
        assert(!sm_groove_current()&&!memcmp(sm_drums.steps,own,sizeof own));
        sm_groove_turn(-1);assert(sm_groove_current()==SM_GROOVES);
        sm_drums.steps[1]^=1u;assert(!sm_groove_current());
        sm_groove_turn(1);sm_groove_turn(-1);
        assert(!sm_groove_current()&&sm_drums.steps[1]==(sm_groove_steps[SM_GROOVES-1u][1]^1u));
    }
    /* the arrangement: a section changes the groove or stops the drums; STOP puts back what was there */
    {
        chain_row_t r = {0, 1}; chain_config_t c;
        chain_row_set_pattern(&r, 3); chain_row_set_groove(&r, 5);
        assert(chain_row_pattern(&r) == 3 && chain_row_groove(&r) == 5);
        chain_defaults(&c); c.count = 1; c.row[0] = r; assert(chain_valid(&c));
        chain_row_set_groove(&c.row[0], CHAIN_DRUMS_OFF); assert(chain_valid(&c));
        chain_row_set_groove(&c.row[0], CHAIN_DRUMS_MINE); assert(chain_valid(&c));
        chain_row_set_groove(&c.row[0], 11); assert(!chain_valid(&c));
        uint16_t before[SM_DRUM_LANES]; uint8_t on = sm_drums.enabled = 0;
        sm_drums.steps[6] = 0x0F0F; memcpy(before, sm_drums.steps, sizeof before);
        sm_chain_drums_begin();
        sm_chain_drums_section(0); assert(!sm_drums.enabled && !memcmp(sm_drums.steps, before, sizeof before));
        sm_chain_drums_section(5); assert(sm_drums.enabled && sm_drums.steps[7] == 0x1044);
        sm_chain_drums_section(7); assert(sm_drums.enabled && sm_drums.steps[0] == 0x0105);
        sm_chain_drums_section(CHAIN_DRUMS_MINE); assert(sm_drums.enabled && !memcmp(sm_drums.steps, before, sizeof before));
        sm_chain_drums_section(CHAIN_DRUMS_OFF); assert(!sm_drums.enabled);
        sm_chain_drums_end(); assert(sm_drums.enabled == on && !memcmp(sm_drums.steps, before, sizeof before));
    }
    printf("Independent drums: audible, bounded, clocked, no four-part mutation; runtime %zu bytes, scratch %zu bytes\n",sizeof sm_drum_audio,sizeof sm_drum_scratch);
    return 0;
}
