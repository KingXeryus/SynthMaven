/* Helpers for the compact SC_SYN performance/filter pages. */
static const param_desc_t *sm_filter_desc(uint32_t f,int16_t **vp){
 static param_desc_t d;static int16_t value[7];static const char *fixed[1];track_t *t=TSEL;uint32_t part=song.sel%4;const sm_filter_cap_t *c=&SM_FILTER_CAP[t->eng_req%NENGINES];
 d=(param_desc_t){"--",F_ENUM,0,0,0,N_DASH,0};*vp=&value[f];value[f]=0;
 if(f==5){d=(param_desc_t){"COMP",F_ONOFF,0,(c->res>=0||c->drive>=0||t->eng_req==ENGI_HYBRID)?1:0,0,0,0};value[f]=(sm_filter_comp_mask>>part)&1;return &d;}
 if(f==6){d=(param_desc_t){"TRIM",F_HALFDB,-64,127,0,0,0};*vp=&t->p[P_ED_FX];return &d;}
 if(t->eng_req==ENGI_HYBRID && (f<3 || f==4)){
  const hy_patch_t *p=hy_gen[part]!=hy_gen_seen[part]?&hy_in[part]:&hy_cur[part];
  d=(param_desc_t){f==0?"F1 MODEL":f==1?"F1 CUT":f==2?"F1 RES":"DRIVE",f==0?F_ENUM:f==1?F_CUTOFF:F_PCT,0,f==0?HF_N-1:127,0,f==0?N_HY_FLT:0,0};value[f]=f==0?p->flt[0].type:f==1?p->flt[0].cut:f==2?p->flt[0].res:p->drive;return &d;
 }
 int32_t id=f==0?c->model:f==1?c->cut:f==2?c->res:f==3?c->env:c->drive;
 if(id>=0){*vp=&t->p[id];d=*track_desc(t,(uint32_t)id);if(f==0)d.label="MODEL";return &d;}
 if(f==0 && c->cut>=0){fixed[0]=c->fixed;d.label="MODEL";d.names=fixed;}
 return &d;
}
static void sm_filter_changed(uint32_t f,int16_t value){uint32_t part=song.sel%4;if(f==5){sm_filter_comp_mask=(uint8_t)((sm_filter_comp_mask&~(1u<<part))|((value!=0)<<part));return;}if(TSEL->eng_req==ENGI_HYBRID&&(f<3||f==4)){hy_patch_t p=hy_gen[part]!=hy_gen_seen[part]?hy_in[part]:hy_cur[part];if(f==0)p.flt[0].type=(uint8_t)value;if(f==1)p.flt[0].cut=(uint8_t)value;if(f==2)p.flt[0].res=(uint8_t)value;if(f==4)p.drive=(uint8_t)value;hy_set_patch(part,&p);}}
static uint32_t sm_mod_template(uint32_t which){track_t *t=TSEL;if(which>3)return 1;if(which==0){t->p[P_VELF]=32;return 0;}if(which==3){if(SM_FILTER_CAP[t->eng_req%NENGINES].env<0)return 1;t->p[SM_FILTER_CAP[t->eng_req%NENGINES].env]=32;return 0;}if(t->eng_req!=ENGI_HYBRID){t->p[which==1?P_ATDST:P_MWDST]=MD_VIB;return 0;}uint32_t part=song.sel%4,i;hy_patch_t p=hy_gen[part]!=hy_gen_seen[part]?hy_in[part]:hy_cur[part];for(i=0;i<HY_NSLOT;i++)if(p.mod[i].src==HS_NONE && p.mod[i].dst==HD_NONE)break;if(i==HY_NSLOT)return 2;p.mod[i]=(hy_slot_t){HS_LFO1,which==1?HS_AT:HS_MW,HD_PITCH,3};t->p[P_LRATE]=64;t->p[P_LWAVE]=0;hy_set_patch(part,&p);return 0;}
static void sm_text_add(char *b,const char *s){uint32_t n=str_len(b);str_cpy(b+n,s,32-n);}
static void sm_text_num(char *b,uint32_t n){char v[12];fmt_int(v,(int32_t)n);sm_text_add(b,v);}
static void sm_message_text(char *b,int candidate){uint8_t m[4];uint32_t i;for(i=0;i<4;i++)m[i]=candidate?sm_candidate[i]:sm_last[i];uint32_t type=candidate?(m[0]==1?0xB0:0xC0):m[0],ch=candidate?m[3]:m[1],n=candidate?m[1]:m[2],v=candidate?m[2]:m[3];str_cpy(b,type==0xB0?"CC CH":type==0xC0?"PC CH":"MIDI CH",32);sm_text_num(b,ch);sm_text_add(b," N");sm_text_num(b,n);if(type!=0xC0){sm_text_add(b," V");sm_text_num(b,v);}}
