/* SPDX-License-Identifier: GPL-3.0-only */
/* Owned generated MIDI chords. Frozen notes survive mode/root changes until
 * release; overlapping generated tones end only after their final owner. */
static struct {uint8_t on,port,ch,key,part,group,n,note[4];} sm_mch[128] __attribute__((section(".pool")));
static uint8_t sm_mch_refs[NPART][128] __attribute__((section(".pool")));
static void sm_mch_release(uint32_t k) {
 uint32_t p=sm_mch[k].part,g=sm_mch[k].group;
 for(uint32_t j=0;j<sm_mch[k].n;j++){uint32_t note=sm_mch[k].note[j];if(sm_mch_refs[g][note] && !--sm_mch_refs[g][note])input_off(&trk[p],note);}
 sm_mch[k].on=0;
}
static void sm_mch_forget(uint32_t part) {
 if(part>=NPART)return;
 if(program_mode){memset(sm_mch,0,sizeof sm_mch);memset(sm_mch_refs,0,sizeof sm_mch_refs);for(uint32_t p=0;p<NPART;p++)chord_auto_forget(p);return;}
 for(uint32_t i=0;i<128;i++)if(sm_mch[i].on && sm_mch[i].part==part)sm_mch[i].on=0;
 memset(sm_mch_refs[part],0,128);chord_auto_forget(part);
}
static int sm_mch_event(track_t *t,uint32_t port,uint32_t ch,uint32_t key,uint32_t vel) {
 uint32_t i,slot=128,part=trk_index(t),group=program_mode?0u:part;
 for(i=0;i<128;i++)if(sm_mch[i].on && sm_mch[i].port==port && sm_mch[i].ch==ch && sm_mch[i].key==key){slot=i;break;}
 if(!vel){if(slot==128)return 0;sm_mch_release(slot);return 1;}
 if(slot<128)sm_mch_release(slot);
 if(part>=NPART || !t->p[P_CHORD] || !voice_poly(t->p[P_VOICE]) || chord_kit(t))return 0;
 if(slot==128)for(i=0;i<128;i++)if(!sm_mch[i].on){slot=i;break;}
 if(slot==128)return 1; /* bounded input ownership; never start unowned notes */
 sm_mch[slot].on=1;sm_mch[slot].port=(uint8_t)port;sm_mch[slot].ch=(uint8_t)ch;sm_mch[slot].key=(uint8_t)key;sm_mch[slot].part=(uint8_t)part;sm_mch[slot].group=(uint8_t)group;
 sm_mch[slot].n=(uint8_t)chord_play_notes(t,key,0,sm_mch[slot].note);
 for(i=0;i<sm_mch[slot].n;i++){
  uint32_t note=sm_mch[slot].note[i];if(sm_mch_refs[group][note]++==0){
   in_chord=sm_mch[slot].n>1?sm_mch[slot].note:0;in_chord_n=sm_mch[slot].n;in_chord_i=i;input_on(t,note,vel);in_chord=0;
  }
 }
 return 1;
}
