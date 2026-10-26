/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint32_t gated(uint32_t p,uint32_t n){uint32_t count=0;for(uint32_t v=0;v<NVOICE;v++)count+=trk[p].v[v].active&&trk[p].v[v].gate&&trk[p].v[v].note==n;return count;}
static void midi(uint32_t status,uint32_t n,uint32_t velocity){midi_in_q[mi_w++%MQ]=0x10u|(status>>4)|(status<<8)|(n<<16)|(velocity<<24);frames(2);}
int main(void){
 outdir="build/host";bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();go_home();frames(3);song.sel=0;program_layer_set(1);midi_in_ch=0;
 for(uint32_t i=0;i<4;i++){set_engine_of(&trk[i],0);apply_preset_to(&trk[i],2);trk[i].p[P_AMODE]=0;trk[i].p[P_VOICE]=0;trk[i].p[P_TRANS]=0;}
 trk[0].p[P_QUANT]=3;trk[0].p[P_SCALE]=1;trk[0].p[P_ROOT]=0;frames(3);
 fm1_in.notes|=1u<<8;frames(2);for(uint32_t i=0;i<4;i++)check(gated(i,60),"ALL quantizes the physical keyboard into the common scale");fm1_in.notes&=~(1u<<8);frames(2);
 midi(0x90,61,100);for(uint32_t i=0;i<4;i++)check(gated(i,60),"ALL quantizes incoming MIDI across all four parts");
 midi(0x90,60,90);midi(0x80,61,0);for(uint32_t i=0;i<4;i++)check(gated(i,60),"releasing a snapped neighbour preserves another held owner");
 trk[0].p[P_ROOT]=2;song.sel=1;midi(0x80,60,0);for(uint32_t i=0;i<4;i++)check(!gated(i,60),"note off retains original pitch after changing root and focus");
 song.sel=0;trk[0].p[P_ROOT]=2;check(program_quantize(0)==1,"bottom MIDI boundary selects a valid scale pitch");check(program_quantize(127)<=127,"top MIDI boundary remains valid");
 trk[0].p[P_ROOT]=0;program_route[1].transpose=1;trk_note_on(TSEL,64,100);frames(2);check(gated(0,64)&&gated(1,65),"part transpose is quantized into the common scale");trk_note_off(TSEL,64);
 trk[0].p[P_QUANT]=0;midi(0x90,61,100);check(gated(0,61),"OFF preserves chromatic MIDI");midi(0x80,61,0);
 uint8_t a[2]={2,0},reply[3];check(master_tuning_bypass==0,"master tuning defaults enabled for every sound");
 check(program_tuning_request(a,2,reply)==3&&!reply[0]&&!reply[2]&&master_tuning_bypass==4,"one part can bypass master tuning");a[0]=1;program_tuning_request(a,1,reply);check(reply[2],"other parts keep applying master tuning");
 a[0]=2;a[1]=2;check(program_tuning_request(a,2,reply)==2&&reply[0]&&master_tuning_bypass==4,"invalid toggle changes nothing");
 program_t saved;program_capture(&saved);master_tuning_bypass=0;program_apply(&saved);check(master_tuning_bypass==4,"program save/load preserves per-sound tuning bypass");
 int16_t *v;syn_desc(X_MTUNE,&v);*v=0;syn_changed(X_MTUNE,v);check(master_tuning_bypass&1,"hardware master switch edits selected sound");
 return fails?1:0;
}
