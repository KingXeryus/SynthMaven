/* SPDX-License-Identifier: GPL-3.0-only */
/* Render factory programs through the production four-part routing, synthesis and stereo effects. */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(int argc,char **argv){
 if(argc!=6)return 1;uint32_t slot=(uint32_t)atoi(argv[1]),scene=(uint32_t)atoi(argv[2]),solo=(uint32_t)atoi(argv[3]);
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir="build/host";program_t p;if(!sm_factory_program(&p,slot))return 2;
 program_apply(&p);frames(4);transport_req=panic_req=0;song.master_q12=4096;song.g[G_BPM]=112;if(scene<4)sm_scene_apply(scene);song.solo=(uint8_t)(solo<4?1u<<solo:0);
 if(getenv("SM_PROGRAM_LAYER_OFF")&&solo<4){program_layer=0;song.sel=solo;song.solo=0;}
 if(getenv("SM_PROGRAM_RAW_SOLO"))memset(program_solo_gain,0,sizeof program_solo_gain);
 FILE *wav=fopen(argv[4],"wb"),*raw=fopen(argv[5],"wb");if(!wav||!raw)return 3;wav_hdr(wav,0);uint32_t count=0,held[4]={0},nheld=0,maxvoices=0;
 const uint8_t chords[4][4]={{60,64,67,71},{57,60,64,69},{53,57,60,65},{55,59,62,67}};
 const uint8_t melody[16]={60,62,64,67,69,67,64,62,60,64,67,72,71,69,67,64};
 const uint8_t oriental[16]={60,61,64,65,67,65,64,61,60,64,65,67,68,67,65,64};
 const uint8_t bass[16]={36,36,43,36,39,41,43,39,36,48,43,41,39,38,36,36};
 uint32_t total=FS*12u,step=FS/2u,last=0xFFFFFFFFu;int32_t o[CTL*2];
 for(uint32_t frame=0;frame<total;frame+=CTL){
  uint32_t beat=frame/step;
  if(beat!=last){
   if(slot!=6 || !(beat%4u) || beat>=16u){for(uint32_t k=0;k<nheld;k++)trk_note_off(&trk[0],held[k]);nheld=0;}
   if(beat<16u){
    uint32_t vel=getenv("SM_PROGRAM_STRESS")?127u:beat<4?64u:beat<8?86u:beat<12?105u:116u;
    if(slot==2||slot==3||slot==7){held[nheld++]=slot==7?bass[beat]:slot==3?oriental[beat]:melody[beat];}
    else if(slot==6){if(!(beat%4u)){for(uint32_t k=0;k<3;k++)held[nheld++]=chords[beat/4u][k]-12u;}}
    else if(!(beat%2u)){uint32_t nn=slot==0||slot==4?4u:3u;for(uint32_t k=0;k<nn;k++)held[nheld++]=chords[beat/4u][k];}
    for(uint32_t k=0;k<nheld;k++)trk_note_on(&trk[0],held[k],vel);
   }last=beat;
  }
  if(slot!=6 && frame%step>step*4u/5u){for(uint32_t k=0;k<nheld;k++)trk_note_off(&trk[0],held[k]);nheld=0;}
  fm6_poll();mix_block(o,CTL);
  uint32_t voices=0;for(uint32_t j=0;j<4;j++)for(uint32_t k=0;k<NVOICE;k++)voices+=trk[j].v[k].active!=0;if(voices>maxvoices)maxvoices=voices;
  for(uint32_t k=0;k<CTL;k++){wav_put(wav,o[2*k],o[2*k+1]);fwrite(&o[2*k],4,2,raw);count++;}
 }
 fseek(wav,0,SEEK_SET);wav_hdr(wav,count);fclose(wav);fclose(raw);
 printf("P%u scene %u solo %u: %u samples, peak active voices %u\n",slot+1,scene,solo,count,maxvoices);return maxvoices>8u?4:0;
}
