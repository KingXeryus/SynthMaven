/* SPDX-License-Identifier: GPL-3.0-only */
/* Four loaded synth parts are the working program; the selected part is UI focus. */
static void program_capture(program_t *p) {
    uint32_t i;memset(p,0,sizeof *p);p->magic=PROGRAM_MAGIC;p->size=sizeof *p;
    fm1_irq_off();p->active=song.sel%4u;p->layer=program_layer;p->tuning=tune_cfg;memcpy(p->custom,custom_scale,sizeof p->custom);
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
    fm1_irq_off();program_mode=1;program_layer=p->layer;song.sel=p->active;tune_cfg=p->tuning;memcpy(custom_scale,p->custom,sizeof custom_scale);custom_scale_dirty=1;
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
