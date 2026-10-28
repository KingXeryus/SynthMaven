/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static uint8_t wire[600];static uint32_t wn;
static void ed_b(uint32_t v){wire[wn++]=(uint8_t)v;}
static void ed_pack7(const uint8_t*p,uint32_t n){(void)p;(void)n;}
static uint32_t ed_unpack7(const uint8_t*a,uint32_t n,uint8_t*b,uint32_t m){(void)a;(void)n;(void)b;(void)m;return 0;}
#include "../firmware/src/editor_fmx.c"
int main(void){
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir="build/host";song.sel=0;palette_set(0);
 tune_cfg.sys=TU_RAST;tune_cfg.root=0;tune_cfg.a4=0;
 uint32_t valid=1;
 for(uint32_t oct=0;oct<9;oct++)for(uint32_t root=0;root<12;root++){
  tune_cfg.root=(uint8_t)root;
  tune_floor_octave=(uint8_t)oct;tune_build();uint32_t from=12*(oct+1);
  for(uint32_t key=0;key<128;key++){
   int32_t want=key<from?0:maqam_cents(TU_RAST,(key+12-root)%12)*4096/100;
   if(tun_tab[key]!=want || tun_at(key*16)!=want)valid=0;
   if(key<from && tun_at(key*16+15)!=0)valid=0;
  }
 }
 check(valid,"C0..C8: all MIDI notes and below-boundary glides stay equal below FROM");
 tune_cfg.root=0;
 int16_t *v;const param_desc_t *d=syn_desc(X_TFLOOR,&v);
 check(d->max==8 && !strcmp(d->names[0],"C0") && !strcmp(d->names[8],"C8"),"FROM has exactly nine octave names, no individual semitones");
 *v=3;syn_changed(X_TFLOOR,v);check(tune_floor_note()==48 && !tun_tab[40] && tun_tab[52]==-2048,"C3 means MIDI 48 inclusive; E2 is equal and E3 is Rast");
 open_family(FAM_GLO);while(cur_page()->id[0]!=X_TSYS)open_family(FAM_GLO);
 check(cur_page()->id[1]==X_TFLOOR,"physical Knob 2 controls FROM on TUNING");
 program_t a;program_packed_t packed;program_capture(&a);check(program_pack(&packed,&a,"OCTAVE"),"threshold fits existing named-program storage");
 tune_floor_octave=0;program_apply(&a);check(tune_floor_octave==3,"program restores its octave threshold");
 tune_floor_octave=0;check(program_unpack(&a,&packed),"named program with threshold unpacks");program_apply(&a);check(tune_floor_octave==3,"named program preserves threshold");
 sm_scene_capture(0);tune_floor_octave=0;sm_scene_apply(0);check(tune_floor_octave==3,"scene restores threshold");
 uint8_t put[]={TU_RAST,0,1,0,22,1,40,3};wn=0;ed_mseq_handle(ED_TUNE_PUT,put,sizeof put);
 check(!wire[0] && tune_floor_octave==3,"native MIDI tuning command accepts C3");
 wn=0;ed_mseq_handle(ED_TUNE_GET,0,0);check(wn==9 && wire[wn-1]==3,"native tuning read reports octave threshold");
 put[7]=9;wn=0;ed_mseq_handle(ED_TUNE_PUT,put,sizeof put);check(wire[0]==1 && tune_floor_octave==3,"invalid octave rejects atomically");
 wn=0;ed_mseq_handle(ED_TUNE_PUT,put,7);check(!wire[0] && tune_floor_octave==3,"legacy A4-only editor keeps threshold unchanged");
 ui.force=1;frames(3);ppm("tuning-from-c3");
 puts(fails?"Tuning octave threshold FAIL":"Tuning octave threshold PASS");return fails?1:0;
}
