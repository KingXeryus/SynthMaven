/* SPDX-License-Identifier: GPL-3.0-only */
/* Render the actual program loader, not a duplicate of its recipes. */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
#include <assert.h>
static int render(unsigned slot,unsigned keys,unsigned base,unsigned velocity,unsigned solo)
{
 reset();assert(!sm_factory_program(slot));
 if(solo)for(unsigned k=1;k<4;k++)trk[k].p[P_MUTE]=1;
 for(unsigned k=0;k<4;k++){trk[k].engine=trk[k].eng_req;if(!eng_state_prepare(&trk[k]))return 2;}
 int32_t out[2*CTL];double energy=0;unsigned peak=0,clipped=0;
 mix_block(out,CTL);
 FILE *wav=0;const char *directory=getenv("SM_FACTORY_WAV_DIR");
 if(directory){char path[512];snprintf(path,sizeof path,"%s/P%u-%ukeys-n%u-v%u-%s.wav",directory,slot+1,keys,base,velocity,solo?"solo":"layered");wav=fopen(path,"wb");if(!wav)return 3;wav_hdr(wav,CTL*(FS*3/CTL));}
 midi_event_source=1;for(unsigned n=0;n<keys;n++)midi_event(0x90,0,base+4*n,velocity);
 for(unsigned b=0;b<FS*3/CTL;b++){
  mix_block(out,CTL);
  if(wav)for(unsigned i=0;i<CTL;i++)wav_put(wav,out[2*i],out[2*i+1]);
  for(unsigned i=0;i<2*CTL;i++){unsigned a=out[i]<0?-(int64_t)out[i]:out[i];if(a>peak)peak=a;if(a>=32767)clipped++;energy+=(double)out[i]*out[i];}
 }
 for(unsigned n=0;n<keys;n++)midi_event(0x80,0,base+4*n,0);midi_event_source=0;if(wav)fclose(wav);
 for(unsigned b=0;b<FS*2/CTL;b++)mix_block(out,CTL);
 double rms=sqrt(energy/(2.0*CTL*(FS*3/CTL)));
 printf("P%u %-12s %u keys: peak %u RMS %.1f dBFS rail samples %u\n",slot+1,sm_program_name,keys,peak,20*log10((rms+1e-9)/32768),clipped);fflush(stdout);
 return peak<20||clipped?1:0;
}
int main(void){
 int bad=0,status;unsigned runs=0,extended=getenv("SM_FACTORY_EXTENDED")!=0;
 for(unsigned s=0;s<8;s++)for(unsigned solo=0;solo<1+extended;solo++)
 for(unsigned reg=0;reg<(extended?3:1);reg++)for(unsigned vel=0;vel<(extended?3:1);vel++)
 for(unsigned keys=1;keys<=3;keys+=2){
  pid_t p=fork();if(!p)_exit(render(s,keys,extended?36+24*reg:48,extended?40+40*vel:100,solo));
  runs++;if(p<0||waitpid(p,&status,0)!=p||!WIFEXITED(status)||WEXITSTATUS(status))bad++;
 }
 printf("Factory programs: %u renders, %d failed; host rendering does not establish device CPU timing or subjective timbre quality\n",runs,bad);return !!bad;
}
