/* SPDX-License-Identifier: GPL-3.0-only
 * SynthMaven independent drum machine, using Melodee's drum_808.c circuits.
 * Four melodic parts and their engine resources are never borrowed here.
 * This state is audio-owned; foreground edits use the existing audio guard.
 */
#define SM_DRUM_LANES 8u
#define SM_DRUM_STEPS 16u
static const uint8_t sm_drum_ins[SM_DRUM_LANES] = {DR_BD,DR_SD,DR_CH,DR_OH,DR_CP,DR_LT,DR_CB,DR_RS};
static const char *const sm_drum_names[SM_DRUM_LANES] = {"KICK","SNARE","HAT","OPEN","CLAP","TOM","BELL","RIM"};
typedef struct {
    uint16_t steps[SM_DRUM_LANES];
    uint8_t level[SM_DRUM_LANES], velocity[SM_DRUM_LANES];
    uint8_t volume, enabled;
} sm_drum_config_t;
static sm_drum_config_t sm_drums = {
    {0x1111,0x1010,0x5555,0,0,0,0,0},
    {100,90,70,70,80,80,70,80}, {110,100,85,90,100,100,95,100}, 90,0
};
/* Factory grooves: a 16-step bar of the lanes above, bit n = step n. Dum is KICK and tak RIM on the Arabic rhythms.
 * Index 0 of the browser is the player's own pattern, kept while grooves are browsed. */
#define SM_GROOVES 8u
/* the arranger's drums row, one word in a section's column */
static const char *const sm_groove_short[CHAIN_DRUMS_MINE] = {"BALL","HALF","ST 8","FLOOR","MAKS","WAHD","BALD","FOX","OFF","MINE"};
static const char *const sm_groove_names[SM_GROOVES] = {"BALLAD","HALF PIPE","STRAIGHT 8","ON THE FLOOR","MAKSOOM","WAHDA","BALADI","FOX"};
/* Not const: in RAM, because the arrangement switches grooves from the audio ISR (song_chain.c) */
static uint16_t sm_groove_steps[SM_GROOVES][SM_DRUM_LANES] = {
    {0x0401,0x1010,0x5555,0,0,0,0,0},
    {0x0481,0x0100,0x5555,0,0,0,0,0x2020},
    {0x0101,0x1010,0x5555,0,0,0,0,0},
    {0x1111,0,0x1111,0x4444,0x1010,0,0,0},
    {0x0101,0,0,0,0,0,0,0x1044},
    {0x0001,0,0,0,0,0,0,0x1040},
    {0x0105,0,0,0,0,0,0,0x1040},
    {0x0101,0,0,0,0,0,0,0x1010},
};
static uint16_t sm_drum_own[SM_DRUM_LANES];
static uint8_t sm_drum_groove;                  /* 0: the player's pattern, n: factory groove n */
/* the groove on the lanes: an edit, a program or the editor replacing the steps makes them the player's pattern */
static uint32_t sm_groove_current(void)
{
    if(sm_drum_groove && memcmp(sm_drums.steps,sm_groove_steps[sm_drum_groove-1u],sizeof sm_drums.steps))
        sm_drum_groove=0;
    return sm_drum_groove;
}
static void sm_groove_turn(int32_t dir)
{
    uint32_t next=(sm_groove_current()+(dir>0?1u:SM_GROOVES))%(SM_GROOVES+1u);
    if(!sm_drum_groove)memcpy(sm_drum_own,sm_drums.steps,sizeof sm_drum_own);
    memcpy(sm_drums.steps,next?sm_groove_steps[next-1u]:sm_drum_own,sizeof sm_drums.steps);
    sm_drum_groove=(uint8_t)next;
}
static struct {
    dr8_t lane[SM_DRUM_LANES];
    uint32_t remaining;
    uint8_t step, running;
} sm_drum_audio;
/* Dedicated scratch avoids aliasing any concurrently rendered part's kit. */
static dr_scratch_t sm_drum_scratch;
static const int16_t sm_drum_params[P_COUNT] = {
    [P_E1]=64,[P_E2]=64,[P_E3]=64,[P_E4]=64
};
/* The arrangement's drum grooves (song_chain.c, in the ISR): a section may change the groove or stop the drums, from its
 * first step; STOP puts back what was there before the song. */
static struct { uint16_t steps[SM_DRUM_LANES]; uint8_t enabled; } sm_drum_song;
static void sm_chain_drums_begin(void)
{
    for(uint32_t k=0;k<SM_DRUM_LANES;k++)sm_drum_song.steps[k]=sm_drums.steps[k];
    sm_drum_song.enabled=sm_drums.enabled;
    sm_drum_audio.step=0;sm_drum_audio.remaining=0;
}
static void sm_chain_drums_section(uint32_t groove)
{
    if(groove==CHAIN_DRUMS_OFF){sm_drums.enabled=0;return;}
    if(!groove||groove>CHAIN_DRUMS_MINE)return;
    for(uint32_t k=0;k<SM_DRUM_LANES;k++)sm_drums.steps[k]=groove==CHAIN_DRUMS_MINE?sm_drum_song.steps[k]:sm_groove_steps[groove-1u][k];
    sm_drums.enabled=1;
    sm_drum_audio.step=0;sm_drum_audio.remaining=0;
}
static void sm_chain_drums_end(void)
{
    for(uint32_t k=0;k<SM_DRUM_LANES;k++)sm_drums.steps[k]=sm_drum_song.steps[k];
    sm_drums.enabled=sm_drum_song.enabled;
}
static void sm_drum_hit(uint32_t lane,uint32_t velocity)
{
    if(lane>=SM_DRUM_LANES || !velocity)return;
    if(lane==2u)sm_drum_audio.lane[3].choke=1;
    dr8_hit(&sm_drum_audio.lane[lane],sm_drum_ins[lane],velocity>127u?127u:velocity,sm_drum_params);
}
static void sm_drum_stop(void)
{
    memset(&sm_drum_audio,0,sizeof sm_drum_audio);
}
/* Called once after the four parts have rendered, before master gain/limiting.
 * Splitting at step boundaries keeps the 16th-note clock sample accurate.
 * Disabled idle cost is constant; no heap, voice stealing or preset writes. */
static void sm_drum_mix(int32_t *left,int32_t *right,uint32_t n,uint32_t quarter,int playing)
{
    if(n>CTL)return;
    int run=playing && sm_drums.enabled;
    if(!run && sm_drum_audio.running)sm_drum_stop();
    if(run && !sm_drum_audio.running){sm_drum_audio.running=1;sm_drum_audio.step=0;sm_drum_audio.remaining=0;}
    for(uint32_t offset=0;offset<n;){
        if(run && !sm_drum_audio.remaining){
            for(uint32_t lane=0;lane<SM_DRUM_LANES;lane++)
                if(sm_drums.steps[lane]&(1u<<sm_drum_audio.step))sm_drum_hit(lane,sm_drums.velocity[lane]);
            sm_drum_audio.step=(sm_drum_audio.step+1u)%SM_DRUM_STEPS;
            sm_drum_audio.remaining=quarter/4u;
            if(!sm_drum_audio.remaining)sm_drum_audio.remaining=1;
        }
        uint32_t count=n-offset;
        if(run && count>sm_drum_audio.remaining)count=sm_drum_audio.remaining;
        for(uint32_t lane=0;lane<SM_DRUM_LANES;lane++){
            if(!sm_drum_audio.lane[lane].on)continue;
            int32_t out[CTL];
            dr8_run_into(&sm_drum_audio.lane[lane],sm_drum_params,out,count,&sm_drum_scratch);
            int32_t gain=(int32_t)sm_drums.level[lane]*sm_drums.volume;
            for(uint32_t i=0;i<count;i++){
                int32_t sample=(soft_knee(clamp(out[i],-262143,262143),24000)*gain)/(127*127);
                left[offset+i]+=sample;right[offset+i]+=sample;
            }
        }
        if(run)sm_drum_audio.remaining-=count;
        offset+=count;
    }
}

#define SM_DRUM_WIRE 34u
static void sm_drum_encode(uint8_t *b,const sm_drum_config_t *c)
{
    for(uint32_t k=0;k<8;k++){b[k*2]=(uint8_t)c->steps[k];b[k*2+1]=(uint8_t)(c->steps[k]>>8);b[16+k]=c->level[k];b[24+k]=c->velocity[k];}
    b[32]=c->volume;b[33]=0;
}
static int sm_drum_decode(sm_drum_config_t *c,const uint8_t *b)
{
    if(b[33])return 0;
    for(uint32_t i=16;i<33;i++)if(b[i]>127)return 0;
    memset(c,0,sizeof *c);
    for(uint32_t k=0;k<8;k++){c->steps[k]=(uint16_t)(b[k*2]|b[k*2+1]<<8);c->level[k]=b[16+k];c->velocity[k]=b[24+k];}
    c->volume=b[32];return 1;
}
