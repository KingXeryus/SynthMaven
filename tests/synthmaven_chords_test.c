/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
int main(void){
 host_tracks_init();track_t *t=&trk[0];host_preset(t,ENGI_FM6,0);t->p[P_VOICE]=V_POLY;t->p[P_CHRD]=CH_NEUTRAL;
 sm_tuning=(sm_tuning_t){.enabled=1,.scale=1,.apply=15,.mask=4095};
 uint8_t notes[CHORD_MAX];int32_t root;uint16_t shape;
 assert(sm_chord_micro(t));assert(track_desc(t,P_CHRD)->names[CH_MAJ][0]==0);assert(track_desc(t,P_CHRD)->names[CH_NEUTRAL][0]);
 assert(chord_make(t,60,notes,&root,&shape)==3&&notes[0]==60&&notes[1]==64&&notes[2]==67);
 assert(sm_tuning_cents(&sm_tuning,0,notes[1])==-50);
 for(unsigned scale=1;scale<SM_TUNE_CUSTOM;scale++)for(unsigned tonic=0;tonic<12;tonic++){
  sm_tuning.scale=scale;sm_tuning.root=tonic;
  for(unsigned mode=CH_OPEN5;mode<=CH_NEUTRAL;mode++){t->p[P_CHRD]=mode;
   for(unsigned key=48;key<84;key++){unsigned n=chord_make(t,key,notes,&root,&shape);assert(n>=1&&n<=CHORD_MAX);
    for(unsigned i=0;i<n;i++){assert(sm_tuning_mask(&sm_tuning)&(1u<<((notes[i]+12-tonic)%12)));if(i)assert(notes[i]>notes[i-1]);}
   }
  }
 }
 sm_tuning.scale=1;sm_tuning.root=0;t->p[P_CHRD]=CH_MAJ;
 unsigned n=chord_make(t,60,notes,&root,&shape);assert(n==3&&notes[1]==67&&notes[2]==72);assert(t->p[P_CHRD]==CH_MAJ);
 sm_tuning.apply=14;assert(!sm_chord_micro(t)&&track_desc(t,P_CHRD)->names[CH_MAJ][0]);
 sm_tuning.apply=15;sm_tuning.octave=5;assert(chord_make(t,48,notes,&root,&shape)==3&&notes[1]==52);
 sm_tuning.enabled=0;assert(!sm_chord_micro(t));
 puts("Scale-safe chords: both neutral choices, every maqam/root/key, descriptor filtering, saved-mode fallback, bypass and octave threshold passed");return 0;
}
