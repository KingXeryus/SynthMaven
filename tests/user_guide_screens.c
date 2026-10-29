/* SPDX-License-Identifier: GPL-3.0-only */
/* Deterministic screenshots of the production renderer, using simulated panel and USB input. */
#define UI_PAGES_HARNESS_ONLY
#define UI_FACTORY_PROGRAMS 1
#define UI_GUIDE_SETUP 1
#include "ui_pages_test.c"
static void capture(const char *name){ui.msg_t=0;ui.arm=ui.arm_t=0;ui.force=1;frames(3);ppm(name);}
static void factory(uint32_t slot){program_t p;layer_unlock();ui.menu=ui.confirm=0;fm1_in.buttons=0;fm1_in.notes=0;sm_factory_program(&p,slot);program_apply(&p);song.g[G_SLOT]=(int16_t)(slot+1);go_home();frames(5);}
static void named_page(uint32_t fam,const char *name){open_family(fam);for(uint32_t i=0;i<NPAGES;i++)if(PAGES[i].fam==fam&&!strcmp(PAGES[i].title,name)){ui.page=(uint8_t)i;page_entered();break;}ui.force=1;}

static int setup_stage=-1;
static void guide_setup_tick(void){if(setup_stage<0)return;int k=setup_stage++;char name[40];snprintf(name,sizeof name,"calibration-%02d",k);ppm(name);if(k<NB)edges_btn=1u<<PANEL_DEFAULT.btn[k];else if(k<NB+NE)encs[PANEL_DEFAULT.enc[k-NB]]=1;if(setup_stage>=NB+NE)setup_stage=-1;}
static FILE *inventory;
static int inventory_first=1;
static const char *guide_engines[]={"ANALOG","DIGITAL","PHASE","LOFI","SAMPLE","VOICE","TRIO","WHEEL","GRAIN","FM6","PHYS","NOISE","HYBRID","WAVE","ACID","STRINGS","MACRO","VPS","TERRAIN","FMX","WSEQ","JUNO","VECTOR"};
static const char *guide_families[]={"HOME","ENV","LFO","FX","SEL","EDIT","GLO","SAVE","ARP","SEQ","TRACKS","CHORDS"};
static void jq(const char *s){fputc('"',inventory);for(;*s;s++){unsigned char c=*s;if(c=='"'||c=='\\')fputc('\\',inventory);if(c>=32)fputc(c,inventory);}fputc('"',inventory);}
static void guide_page(uint32_t i,int engine,int variant){
 const page_t *pg=&PAGES[i];char file[80];snprintf(file,sizeof file,"ref-%02u-e%02d-v%02d",i,engine+1,variant+1);
 ui.home=ui.menu=ui.confirm=ui.sheet=0;ui.page=(uint8_t)i;page_entered();ui.msg_t=0;ui.force=1;ui_draw();ppm(file);
 if(!inventory_first)fputc(',',inventory);inventory_first=0;
 fprintf(inventory,"{\"page_id\":%u,\"family\":",i);jq(guide_families[pg->fam]);fprintf(inventory,",\"title\":");jq(pg->title);
 fprintf(inventory,",\"engine\":");jq(engine<0?"All applicable engines":guide_engines[engine]);fprintf(inventory,",\"variant\":%d,\"scope\":%u,\"image\":",variant,pg->scope);jq(file);fprintf(inventory,",\"controls\":[");
 for(uint32_t k=0;k<4;k++){
  int16_t *vp;const param_desc_t *d=page_desc(pg,k,&vp);if(k)fputc(',',inventory);if(!d){fputs("null",inventory);continue;}
  fprintf(inventory,"{\"id\":%u,\"label\":",pg->id[k]);jq(d->label);fprintf(inventory,",\"min\":%d,\"max\":%d,\"default\":%d,\"format\":%u,\"formatted\":[",d->min,d->max,d->def,d->fmt);
  int vals[]={d->min,d->max,d->def};for(uint32_t j=0;j<3;j++){char b[32];const char *unit;param_format(d,vals[j],b,&unit);if(j)fputc(',',inventory);char full[80];snprintf(full,sizeof full,"%s%s%s",b,unit[0]?" ":"",unit);jq(full);}
  fputs("],\"choices\":[",inventory);if(d->fmt==F_ENUM&&d->names)for(int v=d->min;v<=d->max;v++){if(v>d->min)fputc(',',inventory);jq(d->names[v-d->min]);}fputs("]}",inventory);
 }fputs("]}",inventory);
}
static void guide_inventory(void){
 inventory=fopen("build/host/guide-inventory.json","wb");assert(inventory);fputc('[',inventory);
 factory(2);
 for(uint32_t i=0;i<NPAGES;i++){
  const page_t *pg=&PAGES[i];if(pg->fam==FAM_TRK||pg->scope==SC_SONG||pg->scope==SC_DRUM)continue;
  if(pg->scope==SC_ENGINE||pg->graph==GR_FILTER||pg->graph==GR_FMX){
   for(uint32_t e=0;e<NENGINES;e++){TSEL->eng_req=e;apply_preset_to(TSEL,0);frames(2);if(!page_hidden(i)){guide_page(i,e,-1);if(pg->scope==SC_ENGINE&&(e==3||e==10||e==11)){int hi=track_desc(TSEL,P_E0)->max;for(int v=0;v<=hi;v++){TSEL->p[P_E0]=v;guide_page(i,e,v);}if(e==3){TSEL->p[P_E1]=RW_WRAM;guide_page(i,e,hi+1);}}}}
   TSEL->eng_req=0;apply_preset_to(TSEL,0);frames(2);
  }else if(!strcmp(pg->title,"INSERT")){
   for(uint32_t v=0;v<sizeof(N_ITYPE)/sizeof(N_ITYPE[0]);v++){TSEL->p[P_ITYPE]=v;guide_page(i,-1,v);}TSEL->p[P_ITYPE]=0;
  }else guide_page(i,-1,-1);
 }
 fputs("]\n",inventory);fclose(inventory);
 factory(0);
 for(uint32_t k=0;k<MS_COUNT;k++){char name[40];snprintf(name,sizeof name,"system-%u",k);ui.menu=1;ui.menu_sel=MS_FIRST[k];ui.force=1;ui_draw();ppm(name);}
 ui.menu=2;ui.menu_sel=MI_ABOUT;ui.force=1;ui_draw();ppm("system-about");ui.menu=0;
 named_page(FAM_SEQ,"NOTES");for(uint32_t k=1;k<=6;k++){char name[40];snprintf(name,sizeof name,"note-sheet-%u",k);ui.sheet=k;ui.force=1;ui_draw();ppm(name);}ui.sheet=0;
 factory(0);press(B_SEQ);frames(135);capture("hold-seq");release(B_SEQ);layer_unlock();
 factory(0);press(B_ARP);frames(135);capture("hold-arp");release(B_ARP);layer_unlock();
 factory(0);ui.hold_kind=1;ui.hold_t0=fm1_ms-600;ui.force=1;ui_draw();ppm("hold-rec-clear");ui.hold_kind=0;
 factory(0);rec_wait=1;ui.force=1;ui_draw();ppm("record-armed");rec_wait=0;
 factory(0);named_page(FAM_SAVE,"PROGRAM");song.g[G_SLOT]=5;ui.force=1;ui_draw();ppm("program-slots-5-8");
 const int actions[]={G_LOAD,G_SAVE,G_CLRSEQ,G_INITSND,G_NEWPRJ};
 const char *anames[]={"confirm-load","confirm-save","confirm-clear-seq","confirm-init-sound","confirm-new-program"};
 for(uint32_t a=0;a<5;a++){named_page(FAM_SAVE,a<2?"PROGRAM":"TOOLS");ui.arm=actions[a];ui.arm_t=90;ui_say("AGAIN: ",GP[actions[a]].label);ui.force=1;ui_draw();ppm(anames[a]);}ui.arm=ui.arm_t=ui.msg_t=0;
 factory(0);named_page(FAM_GLO,"PEDAL LEARN");sm_usb_host=1;sm_usb_device_seen=sm_usb_midi_seen=1;sm_learn_state=0;ui.force=1;ui_draw();ppm("pedal-idle");
 sm_learn_arm(0);ui.force=1;ui_draw();ppm("pedal-armed");sm_pedal_receive(0xB0,0,64,127,0,1);ui.force=1;ui_draw();ppm("pedal-captured");sm_learn_accept();sm_pedal_receive(0xB0,0,64,0,0,1);sm_pedal_receive(0xB0,0,64,127,0,1);ui.force=1;ui_draw();ppm("pedal-test");sm_learn_state=0;
 factory(0);rec_wait=1;rec_tempo=1;rec_count=1;ci_on=1;ci_beat=1;ui.force=1;ui_draw();ppm("record-count-in");ci_on=0;rec_wait=0;
 ft_on=1;ft_trk=0;ft_t=FS*3;ui.force=1;ui_draw();ppm("record-free-take");ft_on=0;
 fm1_in.buttons=0;edges_btn=0;setup_stage=0;panel_setup();
}

int main(int argc,char **argv){
 bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();outdir=argc>1?argv[1]:"docs/screenshots/2.2";palette_set(0);
 for(uint32_t i=0;i<8;i++){factory(i);char b[20];snprintf(b,sizeof b,"program-p%u",i+1);capture(b);}
 factory(0);capture("home");
 open_family(FAM_SAVE);capture("save-program");
 factory(0);tap(B_ENV);capture("envelope");
 factory(0);tap(B_EDIT);capture("edit-engine");
 factory(2);named_page(FAM_EDIT,"FILTER");capture("edit-filter");
 named_page(FAM_EDIT,"FILTER GAIN");capture("filter-gain");
 factory(0);tap(B_FX);capture("effects");
 factory(0);tap(B_LFO);capture("lfo");named_page(FAM_LFO,"MOD PRESET");capture("mod-templates");
 named_page(FAM_LFO,"LFO FILTER");TSEL->p[P_LD_FLT]=24;capture("lfo-filter");
 factory(0);tap(B_SCL);capture("scale-browser");
 tune_cfg.sys=TU_RAST;tune_floor_octave=3;tune_build();named_page(FAM_GLO,"TUNING");capture("tuning-from-c3");
 factory(0);press(B_SCL);frames(195);release(B_SCL);custom_scale[0]=2u<<8;tune_cfg.sys=TU_CUSTOM1;tune_build();capture("custom-scale");tap(B_SCL);
 factory(0);press(B_ENV);frames(195);release(B_ENV);TSEL->p[P_CHORD]=1;capture("chords");
 factory(0);press(B_EDIT);frames(195);release(B_EDIT);capture("layer-mixer");tap(B_EDIT);
 factory(0);press(B_GLO);frames(130);ui.force=1;frames(1);ppm("layered-toggle");release(B_GLO);
 factory(0);named_page(FAM_GLO,"LAYER ROUTE");capture("layer-route");
 factory(0);named_page(FAM_GLO,"USB ROLE");capture("usb-role");
 named_page(FAM_GLO,"CONTROLLER");sm_controller_attach(0x1c75,42,"MiniLab 3","");capture("controller");sm_controller_disconnect();
 named_page(FAM_GLO,"PEDAL LEARN");sm_learn_arm(0);sm_pedal_receive(0xB0,0,64,127,0,1);capture("pedal-learn");sm_learn_state=0;
 named_page(FAM_GLO,"SCENES");capture("scenes");
 sm_setlist_len=8;for(uint32_t i=0;i<8;i++)sm_setlist[i]=(uint8_t)i;named_page(FAM_GLO,"SETLIST");capture("setlist");
 factory(0);tap(B_ARP);capture("arpeggiator");tap(B_SEQ);capture("sequencer");
 factory(0);press(B_HOME);frames(195);release(B_HOME);capture("system-menu");
 factory(0);tap(B_HOME);trk_note_on(&trk[0],60,90);trk_note_on(&trk[0],64,85);frames(7);capture("visualizer");trk_all_off(&trk[0]);
 guide_inventory();
 printf("User guide: native renderer screenshots captured\n");return fails?1:0;
}
