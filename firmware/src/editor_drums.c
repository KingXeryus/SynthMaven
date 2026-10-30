/* SPDX-License-Identifier: GPL-3.0-only */
/* Independent drums. 0 read, 1 edit packed config, 2 run/stop, 3 audition,
 * 4 save stopped. Playback state is not part of the stored configuration. */
#define ED_SM_DRUMS 116u
#define ED_SM_TRANSPORT 117u
static int ed_sm_drums_handle(uint32_t cmd,const uint8_t *a,uint32_t n)
{
    if(cmd==ED_SM_TRANSPORT){
        uint32_t op=n?a[0]:127u,rc=1;
        if(n==1&&op<=2){if(op)transport_req=op;rc=0;}
        ed_b(op);ed_b(rc);ed_b(song.playing||seq_counting());return 1;
    }
    if(cmd!=ED_SM_DRUMS)return 0;
    uint32_t op=n?a[0]:127u,rc=1;uint8_t raw[SM_DRUM_WIRE];
    if(op==0&&n==1)rc=0;
    else if(op==1){
        sm_drum_config_t next;
        if(ed_unpack7(a+1,n-1,raw,sizeof raw)==sizeof raw&&sm_drum_decode(&next,raw)){
            uint32_t guard=motion_guard();next.enabled=sm_drums.enabled;sm_drums=next;motion_unguard(guard);rc=0;
        }
    }else if(op==2&&n==2&&a[1]<2){
        uint32_t guard=motion_guard();sm_drums.enabled=a[1];if(!a[1])sm_drum_stop();motion_unguard(guard);rc=0;
    }else if(op==3&&n==3&&a[1]<8&&a[2]<128){
        uint32_t guard=motion_guard();sm_drum_hit(a[1],a[2]);motion_unguard(guard);rc=0;
    }else if(op==4&&n==1)rc=(uint32_t)sm_config_save();
    ui.force=1;ed_b(op);ed_b(rc);ed_b(sm_drums.enabled);ed_b(sm_drum_audio.running?(sm_drum_audio.step+15u)%16u:127u);
    sm_drum_encode(raw,&sm_drums);ed_bk_pack(raw,sizeof raw);return 1;
}
