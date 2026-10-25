/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
#define PROJ_HOST 1
#include "../firmware/src/project.c"
int main(int argc,char**argv){
 outdir="build/host";bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();go_home();frames(2);midi_in_ch=0;
 check(SM_OR_COUNT==88u&&SM_OR_WORLDS==44u,"all 88 world articulations are present");
 FILE*out=fopen(argc>1?argv[1]:"build/host/oriental-loaded.json","w");fprintf(out,"[");
 for(uint32_t n=0;n<SM_OR_COUNT;n++){
  uint32_t p,e=base_at(NBASE+n,&p),w,v,total;set_engine(e);apply_preset(p);
  check(sm_or_find(e,p,&w,&v)&&w*2u+v==n,"world engine/preset ID resolves reversibly");
  check(TSEL->preset==p&&base_pos(&total)==NBASE+n,"hardware browser highlights loaded world patch");
  check(total==NBANK+up_count()&&str_eq(base_kind(NBASE+n),"ORIENTAL"),"world bank and user slots counted correctly");
  check(str_len(sm_factory_name(e,p))<=12u,"OLED and editor name fits 12 characters");
  fprintf(out,"%s{\"engine\":%u,\"preset\":%u,\"name\":\"%s\",\"values\":[",n?",":"",e,p,sm_factory_name(e,p));for(uint32_t k=0;k<P_COUNT;k++)fprintf(out,"%s%d",k?",":"",TSEL->p[k]);fprintf(out,"]}");
  {static project_t saved;int16_t values[P_COUNT];memcpy(values,TSEL->p,sizeof values);proj_capture(&saved);set_engine(0);proj_apply(&saved,1);check(TSEL->preset==p&&TSEL->eng_req==e&&!memcmp(values,TSEL->p,sizeof values),"world patch and high preset IDs survive project recall");}
  midi_in_q[mi_w++%MQ]=0x09u|0x90u<<8|60u<<16|100u<<24;frames(18);
  uint32_t live=0;for(uint32_t k=0;k<NVOICE;k++)live+=trk[0].v[k].active;check(live>0,"world preset responds to MIDI notes");
  midi_in_q[mi_w++%MQ]=0x08u|0x80u<<8|60u<<16;frames(2);
 }
 fprintf(out,"]\n");fclose(out);
 if(argc>2){out=fopen(argv[2],"w");fprintf(out,"[");for(uint32_t e=0;e<NENGINES;e++){fprintf(out,"%s[",e?",":"");for(uint32_t p=0;p<ENGINES[e]->npresets;p++)fprintf(out,"%s\"%s\"",p?",":"",ENGINES[e]->presets[p].name);fprintf(out,"]");}fprintf(out,"]\n");fclose(out);}
 br_eng=BR_ORIENTAL;{uint32_t pos;check(br_count(&pos)==SM_OR_COUNT,"ORIENTAL filter contains all 88 sounds");}
 printf("Oriental collection: %s\n",fails?"FAIL":"PASS");return fails!=0;
}
