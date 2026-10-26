/* SPDX-License-Identifier: GPL-3.0-only */
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
