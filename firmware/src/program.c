/* SPDX-License-Identifier: GPL-3.0-only */
static char program_names[8][13]={"PROGRAM 1","PROGRAM 2","PROGRAM 3","PROGRAM 4","PROGRAM 5","PROGRAM 6","PROGRAM 7","PROGRAM 8"};
static uint8_t program_io_rc,program_editor_name_mask;
static char program_editor_names[8][13];
static uint8_t program_extra_used,program_draft_slot=255,program_name_cursor;
static char program_name_draft[13];
static void program_name_focus(uint32_t slot){slot%=8u;if(program_draft_slot!=slot){memcpy(program_name_draft,(program_editor_name_mask&(1u<<slot))?program_editor_names[slot]:program_names[slot],13);program_draft_slot=(uint8_t)slot;program_name_cursor=0;}}
static void program_name_edit(int32_t delta){
 static const char letters[]=" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";uint32_t n=sizeof letters-1u,k;program_name_focus((uint32_t)song.g[G_SLOT]-1u);
 for(k=0;k<program_name_cursor;k++)if(!program_name_draft[k])program_name_draft[k]=' ';
 char c=program_name_draft[program_name_cursor];for(k=0;k<n && letters[k]!=c;k++);if(k==n)k=0;
 program_name_draft[program_name_cursor]=letters[((int32_t)k+delta%(int32_t)n+(int32_t)n)%(int32_t)n];program_name_draft[12]=0;program_editor_name_mask&=(uint8_t)~(1u<<program_draft_slot);ui.arm=ui.arm_t=0;ui.force=1;
}
/* Four loaded synth parts are the working program; the selected part is UI focus. */
static void program_capture(program_t *p) {
    uint32_t i;memset(p,0,sizeof *p);p->magic=PROGRAM_MAGIC;p->size=sizeof *p;
    fm1_irq_off();p->active=song.sel%4u;p->layer=program_layer;p->reserved[0]=song.solo&15u;p->reserved[1]=master_tuning_bypass&15u;memcpy(p->route,program_route,sizeof p->route);p->tuning=tune_cfg;memcpy(p->custom,custom_scale,sizeof p->custom);
    for(i=0;i<4;i++) {
        const track_t *t=&trk[i];program_sound_t *s=&p->sound[i];
        memcpy(s->p,t->p,sizeof s->p);memcpy(s->ms,t->ms,sizeof s->ms);
        s->engine=t->eng_req;s->preset=t->preset;s->user=t->user;
        if(program_patch_pending&(1u<<i))memcpy(s->patch,program_patch[i],sizeof s->patch);
        else if(s->engine==ENGI_HYBRID)memcpy(s->patch,hy_gen[i]!=hy_gen_seen[i]?&hy_in[i]:&hy_cur[i],sizeof(hy_patch_t));
        else if(s->engine==ENGI_FMX)memcpy(s->patch,&fmx_cur[i],sizeof(fmx_patch_t));
        else if(s->engine==ENGI_FM6)memcpy(s->patch,fm6_patch[i],sizeof s->patch);
    }
    fm1_irq_on();p->sum=program_hash(p,sizeof *p-4u);
}
static void program_apply(const program_t *p) {
    uint32_t i;transport_req=2;panic_req=(1u<<NPART)-1u;
    fm1_irq_off();program_mode=1;program_layer=p->layer;song.sel=p->active;song.solo=p->reserved[0]&15u;master_tuning_bypass=p->reserved[1]&15u;memcpy(program_route,p->route,sizeof program_route);tune_cfg=p->tuning;memcpy(custom_scale,p->custom,sizeof custom_scale);custom_scale_dirty=1;
    for(i=0;i<4;i++) {
        track_t *t=&trk[i];const program_sound_t *s=&p->sound[i];sound_fade_begin(t);
        t->eng_req=s->engine;t->preset=s->preset;t->user=s->user;
        memcpy(t->p,s->p,sizeof s->p);memcpy(t->ms,s->ms,sizeof s->ms);
        memcpy(program_patch[i],s->patch,sizeof s->patch);
        fm6_slot[i]=(uint8_t)t->p[P_E7];fm6_pend&=(uint8_t)~(1u<<i);fmx_reload&=(uint8_t)~(1u<<i);
    }
    RING_PUBLISH();program_patch_pending=15u;fm1_irq_on();
    tune_build();settings_later=1;sync_reload=1;ui.force=1;
}
static void program_layer_set(uint32_t on) {
    if(program_layer!=(on!=0u)){panic_req=(1u<<NPART)-1u;program_layer=(uint8_t)(on!=0u);ui.force=1;}
}

/* Protocol 20: one sound's master tuning switch; no argument reads are ambiguous. */
static uint32_t program_tuning_request(const uint8_t *a,uint32_t na,uint8_t reply[3]) {
    uint32_t i=na?a[0]:4u,rc=i>=4u || (na!=1u && na!=2u) || (na==2u && a[1]>1u);
    reply[0]=(uint8_t)rc;reply[1]=(uint8_t)i;
    if(rc)return 2;
    if(na==2u){master_tuning_bypass=(uint8_t)((master_tuning_bypass&~(1u<<i))|((!a[1])<<i));settings_later=1;ui.force=1;}
    reply[2]=(uint8_t)!(master_tuning_bypass&(1u<<i));return 3;
}
/* Protocol 19: read one layer, or update its routing and mixer atomically. */
static uint32_t program_mix_request(const uint8_t *a,uint32_t na,uint8_t reply[11]) {
    uint32_t i=na?a[0]:4u,rc=(i>=4u || (na!=1u && na!=10u));
    if(!rc && na==10u)
        rc=a[1]>1u || a[2]>1u || a[3]>1u || a[4]>127u || a[5]>48u || a[6]>127u || a[7]>127u || a[8]>1u || a[9]>1u;
    reply[0]=(uint8_t)rc;reply[1]=(uint8_t)i;
    if(rc)return 2;
    fm1_irq_off();
    program_route_t *r=&program_route[i];
    if(na==10u) {
        if(r->enabled!=a[1] || r->split!=a[2] || r->upper!=a[3] || r->point!=a[4] || r->transpose!=(int32_t)a[5]-24)panic_req|=1u<<i;
        r->enabled=a[1];r->split=a[2];r->upper=a[3];r->point=a[4];r->transpose=(int8_t)((int32_t)a[5]-24);
        trk[i].p[P_LEVEL]=a[6];trk[i].p[P_PAN]=(int16_t)a[7]-64;trk[i].p[P_MUTE]=a[8];
        song.solo=(uint8_t)((song.solo&~(1u<<i))|(a[9]<<i));ui.force=1;
    }
    reply[2]=r->enabled;reply[3]=r->split;reply[4]=r->upper;reply[5]=r->point;reply[6]=(uint8_t)(r->transpose+24);
    reply[7]=(uint8_t)trk[i].p[P_LEVEL];reply[8]=(uint8_t)(trk[i].p[P_PAN]+64);reply[9]=(uint8_t)trk[i].p[P_MUTE];reply[10]=(uint8_t)((song.solo>>i)&1u);
    fm1_irq_on();return 11;
}

/* Protocol 21: twelve printable ASCII name bytes, staged until that slot is saved. */
static uint32_t program_name_request(const uint8_t *a,uint32_t n,uint8_t reply[15]){
 uint32_t slot=n?a[0]:8u,k,ended=0;int bad=slot>=8u || (n!=1u && n!=13u);
 if(!bad && n==13u)for(k=1;k<13;k++){if(!a[k])ended=1;else if(ended || a[k]<32u || a[k]>126u)bad=1;}
 reply[0]=(uint8_t)bad;reply[1]=(uint8_t)slot;if(bad)return 2;
 if(n==13u){memcpy(program_editor_names[slot],a+1,12);program_editor_names[slot][12]=0;program_editor_name_mask|=(uint8_t)(1u<<slot);program_draft_slot=255;if((uint32_t)song.g[G_SLOT]==slot+1u)program_name_focus(slot);ui.arm=ui.arm_t=0;ui.force=1;}
 reply[2]=(uint8_t)project_used(slot);const char *name=(program_editor_name_mask&(1u<<slot))?program_editor_names[slot]:program_draft_slot==slot?program_name_draft:program_names[slot];
 for(k=0;k<12;k++)reply[3+k]=(uint8_t)name[k];return 15;
}
