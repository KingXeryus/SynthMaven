/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void){
 check(sm_part_gain(PART_MAX,15000000)==MIX_MAX&&sm_part_gain(-PART_MAX,15000000)==-MIX_MAX,"maximum extended gain saturates without polarity wrap");
 check(sm_part_gain(100,4096)==100,"ordinary part gain is transparent");
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();
 program_layer=1;song.sel=0;song.solo=0;
 for(uint32_t i=0;i<4;i++){program_route[i].enabled=1;trk[i].p[P_LEVEL]=104;trk[i].p[P_MUTE]=0;}
 check(sm_solo_part()==-1,"layered output retains the intended mixer balance");
 song.solo=4;check(sm_solo_part()==2,"solo selects fixed compensation for its part");song.solo=0;program_layer=0;song.sel=3;check(sm_solo_part()==3,"layer OFF uses selected part compensation");
 program_solo_gain[3]=0;sm_solo_gain_cur=4096;int32_t legacy_l[CTL]={100},legacy_r[CTL]={-100};sm_solo_output(legacy_l,legacy_r,CTL);check(legacy_l[0]==100&&legacy_r[0]==-100,"legacy zero metadata is unity, not maximum compensation");
 program_solo_gain[3]=21;sm_solo_gain_cur=4096;int32_t l[CTL],r[CTL];for(uint32_t b=0;b<120;b++){for(uint32_t i=0;i<CTL;i++){l[i]=100;r[i]=-100;}sm_solo_output(l,r,CTL);}
 check(l[CTL-1]>=995&&l[CTL-1]<=1005&&r[CTL-1]==-l[CTL-1],"fixed +20 dB gain, stereo linked and independent of measured RMS");
 for(uint32_t phase=0;phase<3;phase++){int32_t sum=0;for(uint32_t k=0;k<20;k++)sum+=abs(SM_TP_Q15[phase][k]);check(sum<73728,"interpolation bound permits safe integer accumulator and quiet-block shortcut");}
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
 printf("Fixed solo gain, layer bypass, arithmetic bounds and transparent/overloaded limiter %s\n",fails?"FAIL":"PASS");return fails?1:0;
}
