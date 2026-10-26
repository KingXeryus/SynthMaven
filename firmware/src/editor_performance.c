/* Protocol 22: staged pedal learning, scenes, setlists and native filter capabilities. */
static int ed_performance(uint32_t cmd,const uint8_t *a,uint32_t n){
 uint32_t i,op=n?a[0]:0,rc=0;
 if(cmd==112u){
  if(op==1){rc=n!=2?1:sm_learn_arm(a[1]);}
  else if(op==2){rc=n!=1?1:sm_learn_accept();if(!rc)settings_later=1;}
  else if(op==3){if(n!=1)rc=1;else sm_learn_state=0;}
  else if(op==4){if((n!=2&&n!=6)||a[1]>=SM_ACTIONS)rc=1;else {i=a[1];if(n==6){if(a[2]>2||a[3]>127||a[4]>127||a[5]>16)rc=1;else if(sm_map_conflict(a+2,i))rc=2;else{fm1_irq_off();memcpy(sm_pedal[i],a+2,4);sm_repeat_valid[i]=0;fm1_irq_on();settings_later=1;}}ed_b(rc);ed_b(i);for(uint32_t k=0;k<4;k++)ed_b(sm_pedal[i][k]);return 1;}}
  else if(op==5){if(n!=3||(a[1]+128u*a[2])>500u)rc=1;else{sm_repeat_ms=(uint16_t)(a[1]+128u*a[2]);memset(sm_repeat_valid,0,sizeof sm_repeat_valid);settings_later=1;}}
  else if(op==6){if(n!=2||a[1]>1)rc=1;else{sm_usb_host=a[1];settings_later=1;}}
  else if(op!=0||n>1)rc=1;
  ed_b(rc);ed_b(sm_usb_host);ed_b(sm_usb_device_seen);ed_b(sm_usb_midi_seen);ed_b(sm_last[0]>>4);for(i=1;i<4;i++)ed_b(sm_last[i]);ed_b(sm_last_match);ed_b(sm_last_result);ed_b(sm_learn_state);ed_b(sm_learn_action);for(i=0;i<4;i++)ed_b(sm_candidate[i]);ed_b(sm_repeat_ms&127);ed_b(sm_repeat_ms>>7);ed_b(sm_trigger_count&127);ed_b((sm_trigger_count>>7)&127);ed_b(sm_action_rc);ed_b(sm_pending_program==255?127:sm_pending_program);ui.force=1;return 1;
 }
 if(cmd==113u){
  if(op==0 && n<=1){uint32_t mask=0;for(i=0;i<4;i++)if(sm_scenes[i].valid)mask|=1u<<i;ed_b(0);ed_b(mask);ed_b(sm_scene_index<4?sm_scene_index:127);return 1;}
  if(n<2||a[1]>=4)rc=1;
  else if(op==1 && n==2)rc=sm_scene_capture(a[1]);
  else if(op==2 && n==2)rc=sm_scene_apply(a[1]);
  else if(op==3 && n==2){memset(&sm_scenes[a[1]],0,sizeof(sm_scene_t));if(sm_scene_index==a[1])sm_scene_index=255;}
  else if(op==4 && n==2){ed_b(0);ed_b(a[1]);ed_pack7((const uint8_t *)&sm_scenes[a[1]],sizeof(sm_scene_t));return 1;}
  else if(op==5){sm_scene_t s;if(n!=57u||ed_unpack7(a+2,n-2,(uint8_t *)&s,sizeof s)!=sizeof s||!sm_scene_valid(&s))rc=1;else memcpy(&sm_scenes[a[1]],&s,sizeof s);}
  else rc=1;ed_b(rc);ed_b(n>=2?a[1]:127);ui.force=1;return 1;
 }
 if(cmd==114u){
  if(op==1){if(n<2||a[1]>32||n!=2u+a[1])rc=1;else{for(i=0;i<a[1];i++)if(a[2+i]>=8)rc=1;if(!rc){memset(sm_setlist,0,sizeof sm_setlist);memcpy(sm_setlist,a+2,a[1]);sm_setlist_len=a[1];sm_setlist_active=0;if(sm_setlist_pos>=sm_setlist_len)sm_setlist_pos=0;settings_later=1;}}}
  else if(op==2){if(n!=2||a[1]>1)rc=1;else rc=sm_performance_action(a[1]?9:8);}
  else if(op==3){if(n!=2||a[1]>=sm_setlist_len)rc=1;else rc=sm_program_request(sm_setlist[a[1]],a[1]);}
  else if(op!=0||n>1)rc=1;ed_b(rc);ed_b(sm_setlist_len);ed_b(sm_setlist_active?sm_setlist_pos:127);for(i=0;i<sm_setlist_len;i++)ed_b(sm_setlist[i]);ui.force=1;return 1;
 }
 if(cmd==115u){uint32_t e=n?a[0]:TSEL->eng_req;if(n>1||e>=NENGINES){ed_b(1);return 1;}const sm_filter_cap_t *c=&SM_FILTER_CAP[e];ed_b(0);ed_b(e);ed_b(c->cut<0?0:c->model==-2?3:c->model==P_FMODEL?2:c->model>=0?4:1);ed_b(c->cut<0?0:c->cut+1);ed_b(c->res<0?0:c->res+1);ed_b(c->env<0?0:c->env+1);ed_b(c->drive<0?0:c->drive+1);ed_str(c->fixed,16);return 1;}
 if(cmd==116u){
  if(op==1){if(n!=4||a[1]>6)rc=1;else{int16_t *vp;const param_desc_t *d=sm_filter_desc(a[1],&vp);int32_t v=ed_rv(a+2);if(!vp||v<d->min||v>d->max)rc=1;else{*vp=(int16_t)v;sm_filter_changed(a[1],*vp);}}}
  else if(op==2){if(n!=2)rc=1;else rc=sm_mod_template(a[1]);}
  else if(op!=0||n>1)rc=1;
  ed_b(rc);ed_b(TSEL->eng_req);for(i=0;i<7;i++){int16_t *vp;const param_desc_t *d=sm_filter_desc(i,&vp);char val[12];const char *unit;ed_v(*vp);ed_v(d->min);ed_v(d->max);ed_b(d->fmt);ed_str(d->label,12);param_format(d,*vp,val,&unit);ed_str(val,8);ed_str(unit,8);uint32_t count=d->fmt==F_ENUM?(uint32_t)(d->max-d->min+1):0;ed_b(count);for(uint32_t k=0;k<count;k++)ed_str(d->names[k],12);}ui.force=1;return 1;
 }
 return 0;
}
