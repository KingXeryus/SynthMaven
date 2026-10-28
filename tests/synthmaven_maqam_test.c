/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void) {
 uint32_t sys,root,key;int16_t *vp;FILE *out=fopen("build/host/maqam-pitches.json","w");int comma=0;
 assert(out);fputs("[",out);
 outdir="build/host";bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();palette_set(4);go_home();frames(2);
 check(TU_RAST==6 && TU_BAYATI==7 && TU_USER==13 && TU_N==62,"old tuning IDs preserved; 46 maqam settings");
 for(sys=0;sys<TU_N;sys++)if(tune_is_maqam(sys))for(root=0;root<12;root++) {
  tune_cfg.sys=sys;tune_cfg.root=root;tune_cfg.a4=0;tune_build();
  fprintf(out,"%s{\"sys\":%u,\"root\":%u,\"pitches\":[",comma++?",":"",sys,root);
  for(key=0;key<128;key++) {
   fprintf(out,"%s%d",key?",":"",tun_tab[key]);
   int32_t cents=maqam_cents(sys,(key+12-root)%12);
   assert(tun_tab[key]==(key<12?0:cents*4096/100));
   assert(tun_at(key*16)==tun_tab[key]);
   if(key>=12 && key<116)assert(tun_tab[key]==tun_tab[key+12]);
  }
  assert(maqam_cents(sys,0)==0);fputs("]}",out);
 }
 fputs("]\n",out);fclose(out);
 tune_cfg.sys=TU_RAST;tune_cfg.root=0;tune_build();
 for(key=0;key<128;key++)check(tun_tab[key]==((key>=12 && (key%12==4||key%12==11))?-2048:0),"Rast C alters E and B only; all black keys intact");
 tune_cfg.sys=TU_BAYATI;tune_cfg.root=2;tune_build();
 for(key=0;key<128;key++)check(tun_tab[key]==(key>=12 && key%12==4?-2048:0),"Bayati D alters E only");
 tune_cfg.sys=TU_HIJAZ;tune_build();for(key=0;key<128;key++)check(tun_tab[key]==0,"Hijaz chromatic fingering has no quarter-tone offset");
 tune_cfg.sys=TU_SIKAH;tune_cfg.root=1;tune_cfg.a4=-8;tune_build();
 for(key=0;key<128;key++)check(tun_tab[key]==(key<12?0:maqam_cents(TU_SIKAH,(key+11)%12)*4096/100)+TU_A4[32],"A4 adds uniformly to maqam offsets");
 open_family(FAM_GLO);while(cur_page()->id[0]!=X_TSYS)open_family(FAM_GLO);
 syn_desc(X_TSYS,&vp);*vp=TU_SABA;syn_changed(X_TSYS,vp);
 syn_desc(X_TROOT,&vp);*vp=2;syn_changed(X_TROOT,vp);
 check(tune_cfg.sys==TU_SABA && tune_cfg.root==2 && settings_later,"hardware edits maqam and root and schedules save");
 ui.force=1;frame();ppm("maqam-saba-root-d");
 tune_cfg.sys=TU_BASTANIKAR;tune_build();ui.force=1;frame();ppm("maqam-bastanikar");
 tune_cfg.sys=TU_USER;tune_cfg.root=0;tune_cfg.a4=0;tune_cfg.n=3;tune_cfg.period=19200;tune_cfg.deg[1]=6400;tune_cfg.deg[2]=11200;tune_build();
 check(tun_tab[61]==3*4096,"legacy USER ID 13 retains Scala degree mapping");
 puts("Maqam: 46 settings x 12 roots x 128 notes, unaffected keys, octave/tonic, A4, audio lookup, hardware UI and legacy Scala PASS");
 return fails?1:0;
}
