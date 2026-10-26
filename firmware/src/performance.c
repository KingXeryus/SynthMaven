/* Compact scene state belongs to the current four-part program; setlists reference saved slots. */
static uint8_t sm_pending_program=255,sm_pending_setpos=255,sm_action_rc;
static uint32_t sm_pending_at;
static int sm_perf_quiet(void){uint32_t p,i;for(p=0;p<NPART;p++)for(i=0;i<NVOICE;i++)if(trk[p].v[i].active)return 0;for(i=0;i<NDRUM;i++)if(drums.v[i].active)return 0;return 1;}
static uint32_t sm_scene_capture(uint32_t slot){
 uint32_t i,k;if(slot>=SM_SCENES)return 1;sm_scene_t *s=&sm_scenes[slot];memset(s,0,sizeof *s);s->valid=1;s->sys=tune_cfg.sys;s->root=tune_cfg.root;s->a4=tune_cfg.a4;s->bypass=master_tuning_bypass;s->comp=sm_filter_comp_mask;
 fm1_irq_off();for(i=0;i<4;i++){track_t *t=&trk[i];sm_scene_part_t *p=&s->part[i];p->level=t->p[P_LEVEL];p->mute=t->p[P_MUTE];p->dist=t->p[P_DIST];p->chor=t->p[P_CHOR];p->dly=t->p[P_DLY];p->rev=t->p[P_REV];for(k=0;k<4;k++){uint32_t id=t->eng_req==ENGI_HYBRID?P_E4+k:P_MAC1+k;p->macro[k]=(uint8_t)(t->p[id]-track_desc(t,id)->min);}}fm1_irq_on();sm_scene_index=(uint8_t)slot;ui.force=1;return 0;
}
static uint32_t sm_scene_apply(uint32_t slot){
 uint32_t i,k;if(slot>=SM_SCENES||!sm_scenes[slot].valid||!sm_scene_valid(&sm_scenes[slot]))return 1;const sm_scene_t *s=&sm_scenes[slot];
 fm1_irq_off();for(i=0;i<4;i++){track_t *t=&trk[i];const sm_scene_part_t *p=&s->part[i];t->p[P_LEVEL]=p->level;t->p[P_MUTE]=p->mute;t->p[P_DIST]=p->dist;t->p[P_CHOR]=p->chor;t->p[P_DLY]=p->dly;t->p[P_REV]=p->rev;for(k=0;k<4;k++){uint32_t id=t->eng_req==ENGI_HYBRID?P_E4+k:P_MAC1+k;t->p[id]=(int16_t)(p->macro[k]+track_desc(t,id)->min);}}tune_cfg.sys=s->sys;tune_cfg.root=s->root;tune_cfg.a4=s->a4;master_tuning_bypass=s->bypass;sm_filter_comp_mask=s->comp;fm1_irq_on();tune_build();sm_scene_index=(uint8_t)slot;settings_later=1;ui.force=1;return 0;
}
static uint32_t sm_scene_step(int dir){uint32_t i;int32_t base=sm_scene_index<4?sm_scene_index:(dir>0?3:0);for(i=1;i<=4;i++){uint32_t slot=(uint32_t)((base+dir*(int32_t)i+8)%4);if(sm_scenes[slot].valid)return sm_scene_apply(slot);}return 1;}
static uint32_t sm_program_request(uint32_t slot,uint32_t setpos){
 if(slot>=8||!project_used(slot)){ui_message("EMPTY PROGRAM SLOT");return 1;}
 if(sm_pending_program!=255)return 2;sm_pending_program=(uint8_t)slot;sm_pending_setpos=(uint8_t)setpos;sm_pending_at=fm1_ms;panic_req=(1u<<NTRK)-1u;transport_req=2;sm_action_rc=2;ui_message("PROGRAM WAIT");return 0;
}
static void sm_performance_tick(void){if(sm_pending_program==255)return;if((uint32_t)(fm1_ms-sm_pending_at)>5000u){sm_pending_program=255;sm_action_rc=3;ui_message("PROGRAM LOAD TIMEOUT");return;}if(song.playing||transport_req||!sm_perf_quiet())return;uint32_t slot=sm_pending_program;program_io_rc=0;project_load(slot);if(program_io_rc==1)return;if(!program_io_rc){song.g[G_SLOT]=(int16_t)(slot+1);sm_current_program=(uint8_t)slot;sm_setlist_active=sm_pending_setpos<sm_setlist_len;if(sm_setlist_active)sm_setlist_pos=sm_pending_setpos;ui_message("PROGRAM LOADED");}else ui_message("PROGRAM LOAD ERROR");sm_action_rc=program_io_rc;sm_pending_program=255;ui.force=1;}
static uint32_t sm_performance_action(uint32_t a){
 if(a==6||a==7)return sm_scene_step(a==6?1:-1);
 if(a==8||a==9){if(!sm_setlist_len)return 1;uint32_t pos=(uint32_t)((sm_setlist_active?sm_setlist_pos+(a==8?1:(int32_t)sm_setlist_len-1):(a==8?0:sm_setlist_len-1))%sm_setlist_len);return sm_program_request(sm_setlist[pos],pos);}
 if(a==4||a==5){uint32_t i,base=sm_current_program<8?sm_current_program:(a==4?7:0);for(i=1;i<=8;i++){uint32_t p=(base+(a==4?i:8-i))%8;if(project_used(p))return sm_program_request(p,255);}return 1;}
 return 1;
}
