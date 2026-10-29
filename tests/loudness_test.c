/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void){
 int32_t l[CTL],r[CTL];
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();program_mode=1;program_layer=1;song.solo=0;song.sel=0;
 for(uint32_t i=0;i<4;i++){program_route[i].enabled=1;trk[i].p[P_LEVEL]=104;trk[i].p[P_MUTE]=0;}
 check(sm_solo_part()==-1,"four enabled layers use mix headroom");
 song.solo=2;check(sm_solo_part()==1,"solo bypasses layered attenuation");song.solo=0;
 program_layer=0;check(sm_solo_part()==0,"layer OFF preserves selected solo gain");
 program_solo_gain[0]=21;sm_solo_gain_cur=4096;
 for(uint32_t b=0;b<120;b++){for(uint32_t i=0;i<CTL;i++){l[i]=100;r[i]=-100;}sm_solo_output(l,r,CTL);}
 check(l[CTL-1]>=995&&l[CTL-1]<=1005,"stored solo boost is preserved");
 program_layer=1;song.solo=0;
 for(uint32_t i=0;i<CTL;i++){l[i]=100;r[i]=-100;}sm_solo_output(l,r,CTL);
 check(sm_solo_gain_cur==4096&&l[0]==100&&r[0]==-100,"enabling layers clears stale solo boost immediately");
 program_mode=0;

 check(LIM_T==25500 && KNEE==30000,"solo output thresholds retained");
 /* Quiet material is unchanged except for exactly one control block of look-ahead. */
 lim_t s={.env=LIM_T,.g=32768};int32_t prev[CTL];memset(prev,0,sizeof prev);
 for(uint32_t b=0;b<20;b++){for(uint32_t i=0;i<CTL;i++){l[i]=(int32_t)(2000*sin((b*CTL+i)*.19));r[i]=-l[i];}int32_t next[CTL];memcpy(next,l,sizeof next);lim_block(&s,l,r,CTL);for(uint32_t i=0;i<CTL;i++)check(l[i]==prev[i]&&r[i]==-prev[i],"quiet signal is transparent through limiter look-ahead");memcpy(prev,next,sizeof prev);}
 FILE *f=fopen("build/host/limiter-stress.raw","wb");check(f!=0,"create numerical stress render");
 if(f){
  for(uint32_t pattern=0;pattern<4;pattern++){
   uint32_t clipped=0;memset(&s,0,sizeof s);s.env=LIM_T;s.g=32768;
   for(uint32_t b=0;b<1500;b++){
    for(uint32_t i=0;i<CTL;i++){
     uint32_t n=b*CTL+i;double v=pattern==0?sin(n*1.5707963267948966+.7853981633974483):pattern==1?sin(n*2.9):pattern==2?(n%128==0?1:0):sin(n*.0041);
     l[i]=(int32_t)(v*(pattern==3?1000000:200000));r[i]=pattern==1?-l[i]:l[i];
    }
    lim_block(&s,l,r,CTL);
    for(uint32_t i=0;i<CTL;i++){int32_t x[]={l[i],r[i]};fwrite(x,4,2,f);clipped+=(abs(l[i])>=32768||abs(r[i])>=32768);}
   }
   check(!clipped,"hot limiter pattern has no full-scale samples");
  }
  fclose(f);
 }
 printf("Restored transparent/overloaded limiter %s\n",fails?"FAIL":"PASS");return fails?1:0;
}
