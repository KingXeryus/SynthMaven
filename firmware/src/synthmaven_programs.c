/* SPDX-License-Identifier: GPL-3.0-only */
/* SynthMaven sound programs are distinct from Melodee sequencer projects.
 * Each A/B flash object holds two versioned programs. Legacy project objects
 * remain protected: this API never silently replaces an FBKK/FBKL project. */
#define SM_PROGRAM_COUNT 8u
#define SM_PROGRAM_META 500u
#define SM_SOUND_BYTES (12u+sizeof(tmpl_t))
#define SM_PROGRAM_RECORD (SM_SOUND_BYTES+SM_PROGRAM_META)
#define SM_CONFIG_BYTES 384u
#define SM_CONFIG_OFF (16u+2u*SM_PROGRAM_RECORD)
#define SM_PROGRAM_BANK (SM_CONFIG_OFF+SM_CONFIG_BYTES)
#define SM_SCENE_BYTES 112u
static uint8_t sm_program_meta[SM_PROGRAM_META] __attribute__((section(".pool")));
static uint8_t sm_program_current=255,sm_scene_current=255;
static char sm_program_name[13];
static uint8_t sm_program_boot_slot=255;
static const uint8_t SM_SCENE_PARAM[8]={P_LEVEL,P_PAN,P_MUTE,P_FXSEND,P_E4,P_E5,P_E6,P_E7};
static int sm_program_meta_valid(const uint8_t *b)
{
    sm_layers_t layers;sm_tuning_t tuning;
    if(b[0]>1||b[1]>3||b[2]>15||b[3]||!sm_layers_decode(&layers,b+4)||!sm_tuning_decode(&tuning,b+20))return 0;
    for(uint32_t s=0;s<4;s++){
        const uint8_t *v=b+48+s*SM_SCENE_BYTES;
        if(!(b[2]&(1u<<s)))continue;
        for(uint32_t i=0;i<12;i++)if(v[i]&&(v[i]<32||v[i]>126))return 0;
        for(uint32_t part=0;part<4;part++)for(uint32_t i=0;i<8;i++){
            int16_t value=(int16_t)(v[12+part*16+i*2]|v[13+part*16+i*2]<<8);
            if(value<(i==1?-64:0)||value>(i==1?63:i==2?1:127))return 0;
        }
        if(!sm_tuning_decode(&tuning,v+76))return 0;
        for(uint32_t i=104;i<SM_SCENE_BYTES;i++)if(v[i]>127)return 0;
    }
    for(uint32_t i=496;i<SM_PROGRAM_META;i++)if(b[i])return 0;
    return 1;
}
static void sm_program_meta_capture(uint8_t *b,int tuning_override)
{
    memcpy(b,sm_program_meta,SM_PROGRAM_META);
    b[0]=(uint8_t)!!tuning_override;b[1]=sm_scene_current<4?sm_scene_current:0;b[2]&=15;b[3]=0;
    sm_layers_encode(b+4,&sm_layers);sm_tuning_encode(b+20,&sm_tuning);
    if(!tuning_override){sm_tuning_t neutral={.apply=15,.mask=4095};sm_tuning_encode(b+20,&neutral);}
    memset(b+496,0,4);
}
static int sm_scene_store(uint32_t slot,const char *name)
{
    if(slot>=4u)return 1;
    uint8_t *v=sm_program_meta+48+slot*SM_SCENE_BYTES;
    uint32_t guard=motion_guard();memset(v,0,SM_SCENE_BYTES);
    for(uint32_t i=0;i<12&&name[i];i++)v[i]=(uint8_t)(name[i]>=32&&name[i]<=126?name[i]:' ');
    for(uint32_t part=0;part<4;part++)for(uint32_t i=0;i<8;i++){
        int16_t value=i>=4&&trk[part].eng_req!=ENGI_HYBRID?0:trk[part].p[SM_SCENE_PARAM[i]];
        v[12+part*16+i*2]=(uint8_t)value;v[13+part*16+i*2]=(uint8_t)(value>>8);
    }
    for(uint32_t part=0;part<4;part++){v[104+part*2]=(uint8_t)trk[part].p[P_FXSEND2];v[105+part*2]=(uint8_t)trk[part].p[P_FXSEND3];}
    sm_tuning_encode(v+76,&sm_tuning);sm_program_meta[2]|=1u<<slot;sm_scene_current=(uint8_t)slot;
    motion_unguard(guard);return 0;
}
static int sm_scene_load(uint32_t slot)
{
    if(slot>=4u||!(sm_program_meta[2]&(1u<<slot)))return 1;
    uint8_t *v=sm_program_meta+48+slot*SM_SCENE_BYTES;sm_tuning_t tuning;
    if(!sm_tuning_decode(&tuning,v+76))return 1;
    uint32_t guard=motion_guard();
    for(uint32_t part=0;part<4;part++)for(uint32_t i=0;i<8;i++){
        if(i>=4&&trk[part].eng_req!=ENGI_HYBRID)continue;
        trk[part].p[SM_SCENE_PARAM[i]]=(int16_t)(v[12+part*16+i*2]|v[13+part*16+i*2]<<8);
    }
    for(uint32_t part=0;part<4;part++){trk[part].p[P_FXSEND2]=v[104+part*2];trk[part].p[P_FXSEND3]=v[105+part*2];}
    sm_tuning=tuning;sm_scene_current=(uint8_t)slot;motion_unguard(guard);sync_reload=1;ui.force=1;return 0;
}
#include "factory_program_pack.h"
/* Decode into the existing stopped-program transfer workspace, no extra audio RAM. */
static int sm_factory_program(uint32_t slot)
{
    if(slot>=8)return 1;if(transport_busy()||wire_waiting())return 4;
#if MELODEE_FLASH
    uint8_t *raw=(uint8_t *)proj_image_store();
#else
    static uint8_t raw[14356]; /* host fixtures have no flash-transfer workspace */
#endif
    uint32_t in=0,out=0;
    while(in<sizeof SM_FACTORY_LZ){
        uint32_t flags=SM_FACTORY_LZ[in++];
        for(uint32_t bit=0;bit<8&&in<sizeof SM_FACTORY_LZ;bit++){
            if(flags&(1u<<bit)){
                uint32_t distance=1u+SM_FACTORY_LZ[in]+((uint32_t)SM_FACTORY_LZ[in+1]<<8),n=SM_FACTORY_LZ[in+2]+3u;in+=3;
                if(distance>out||out+n>SM_FACTORY_OFF[8])return 1;
                while(n--){raw[out]=raw[out-distance];out++;}
            }else {if(out>=SM_FACTORY_OFF[8])return 1;raw[out++]=SM_FACTORY_LZ[in++];}
        }
    }
    if(out!=SM_FACTORY_OFF[8])return 1;
    const uint8_t *r=raw+SM_FACTORY_OFF[slot],*master=r+14,*meta=master+20,*part=meta+SM_PROGRAM_META;
    sm_layers_t layers;sm_tuning_t tuning;
    if(!sm_program_meta_valid(meta)||!sm_layers_decode(&layers,meta+4)||!sm_tuning_decode(&tuning,meta+20))return 1;
    uint32_t guard=motion_guard();panic_req|=15u;undo_depth++;
    for(uint32_t k=0;k<4;k++){
        track_t *t=&trk[k];set_engine_of(t,part[0]);t->preset=part[1];t->user=t->user_native=0;part+=2;
        for(uint32_t i=0;i<P_COUNT;i++){
            int16_t v=(int16_t)(part[i*2]|(uint16_t)part[i*2+1]<<8);
            if(i<P_SLEN||i>P_SGATE)t->p[i]=v;
        }part+=P_COUNT*2;
        if(t->eng_req==ENGI_HYBRID){hy_patch_t v;memcpy(&v,part,sizeof v);hy_set_patch(k,&v);part+=128;}
        else if(t->eng_req==ENGI_FM6){uint8_t v[FP_SIZE+1];fm6_unpack(part,v);fm6_set_patch(k,v);part+=128;memcpy(fm6_fn[k],part,FM6_NFN);part+=FM6_NFN;}
        else if(t->eng_req==ENGI_CZ){memcpy(cz_patch_of(k),part,sizeof(cz_patch_t));cz_track_accept(t);part+=sizeof(cz_patch_t);}
        else if(t->eng_req==ENGI_PROPHET){memcpy(&p5_patch[k],part,sizeof(p5_patch_t));p5_ready[k]=1;part+=sizeof(p5_patch_t);}
        else if(t->eng_req==ENGI_DRUM){memcpy(drum_patch_of(k),part,sizeof(drum_patch_t));part+=sizeof(drum_patch_t);}
        track_fx_normalize(t->p);
    }
    undo_depth--;
    for(uint32_t i=0;i<12;i++)song.g[G_MFXTYPE+i]=master[i];
    song.g[G_FXRETURN]=master[12];song.g[G_FXMODE]=master[13];
    for(uint32_t i=0;i<3;i++)song.g[G_MFXTONE+i]=(int16_t)master[14+i]-64;
    sm_layers=layers;if(meta[0])sm_tuning=tuning;song.sel=r[12]<4?r[12]:0;
    memcpy(sm_program_meta,meta,SM_PROGRAM_META);memcpy(sm_program_name,r,12);sm_program_name[12]=0;
    sm_program_current=(uint8_t)slot;sm_scene_current=meta[1];motion_unguard(guard);
    undo_clear();list_forget();sync_reload=1;ui.force=1;return 0;
}
_Static_assert(14356u<=BANK_STORE_SIZE,"factory decoder workspace");
_Static_assert(P_COUNT==124u&&FM6_PACKED==128u,"factory capture layout");
static void sm_config_pack(uint8_t *b)
{
    memset(b,0,SM_CONFIG_BYTES);memcpy(b,"SMC4",4);b[329]=sm_controller_override;b[330]=sm_controller_port;b[328]=sm_program_current<8?sm_program_current+1:0;b[4]=sm_setlist_count;b[5]=sm_setlist_pos;
    memcpy(b+8,sm_setlist,32);memcpy(b+40,sm_maps,sizeof sm_maps);
    for(uint32_t k=0;k<8;k++){b[331+k*2]=(uint8_t)sm_drums.steps[k];b[332+k*2]=(uint8_t)(sm_drums.steps[k]>>8);b[347+k]=sm_drums.level[k];b[355+k]=sm_drums.velocity[k];}
    b[363]=sm_drums.volume; /* playback is never resumed by a boot/load */
}
static int sm_config_valid(const uint8_t *b)
{
    if((memcmp(b,"SMC1",4)&&memcmp(b,"SMC2",4)&&memcmp(b,"SMC3",4)&&memcmp(b,"SMC4",4))||b[4]>32||b[5]>31||b[6]||b[7])return 0;
    for(uint32_t i=0;i<32;i++)if(b[8+i]>7)return 0;
    for(uint32_t i=0;i<SM_MAP_COUNT;i++)if(!sm_map_valid(b+40+i*SM_MAP_BYTES))return 0;
    if(b[3]>='2'&&b[328]>8)return 0;
    if(b[3]>='3'&&(b[329]>3||b[330]>16))return 0;
    if(b[3]=='4'){for(uint32_t i=347;i<=363;i++)if(b[i]>127)return 0;}
    for(uint32_t i=b[3]=='4'?364:b[3]=='3'?331:b[3]=='2'?329:328;i<SM_CONFIG_BYTES;i++)if(b[i])return 0;return 1;
}
#if MELODEE_FLASH
_Static_assert(SM_PROGRAM_BANK<=ST_BANK_BASE_MAX+ST_BANK_EXT_MAX,"eight programs fit existing atomic project extents");
static int sm_program_bank(uint32_t bank,st_hdr_t *h,uint8_t *header)
{
    if(bank>=4)return -1;int copy=st_current(OBJ_BANK0+bank,h);if(copy<0)return -1;
    if(h->len!=SM_PROGRAM_BANK||st_read_range(OBJ_BANK0+bank,(uint32_t)copy,0,header,16))return -2;
    if(memcmp(header,"SMP5",4)||header[4]!=2||header[5]>3)return -2;
    for(uint32_t i=6;i<16;i++)if(header[i])return -2;
    return copy;
}
static void sm_config_load(void)
{
    st_hdr_t h;uint8_t header[16],b[SM_CONFIG_BYTES];int copy=sm_program_bank(0,&h,header);
    if(copy<0||st_read_range(OBJ_BANK0,(uint32_t)copy,SM_CONFIG_OFF,b,sizeof b)||!sm_config_valid(b))return;
    sm_program_boot_slot=b[3]>='2'&&b[328]?b[328]-1:255;
    sm_controller_override=b[3]>='3'?b[329]:0;sm_controller_port=b[3]>='3'?b[330]:0;
    if(b[3]=='4'){for(uint32_t k=0;k<8;k++){sm_drums.steps[k]=(uint16_t)(b[331+k*2]|b[332+k*2]<<8);sm_drums.level[k]=b[347+k];sm_drums.velocity[k]=b[355+k];}sm_drums.volume=b[363];sm_drums.enabled=0;sm_drum_stop();}
    sm_setlist_count=b[4];sm_setlist_pos=b[5];memcpy(sm_setlist,b+8,32);memcpy(sm_maps,b+40,sizeof sm_maps);
}
static int sm_config_save(void)
{
    if(transport_busy()||wire_waiting())return 4;
    proj_wire_gen++;
    st_hdr_t h;uint8_t header[16]={0},config[SM_CONFIG_BYTES];int copy=sm_program_bank(0,&h,header);
    if(copy==-2)return 3;sm_config_pack(config);if(!sm_config_valid(config))return 1;
    if(copy>=0){uint8_t previous[SM_CONFIG_BYTES];if(!st_read_range(OBJ_BANK0,(uint32_t)copy,SM_CONFIG_OFF,previous,sizeof previous)&&!memcmp(previous,config,sizeof config))return 0;}
    st_stream_t w;if(st_stream_begin(&w,OBJ_BANK0,SM_PROGRAM_BANK))return 2;
    uint8_t *chunk=wire_take();for(uint32_t off=16;off<SM_CONFIG_OFF;){uint32_t n=SM_CONFIG_OFF-off;if(n>sizeof proj_wire_u.raw)n=sizeof proj_wire_u.raw;
        if(copy>=0){if(st_read_range(OBJ_BANK0,(uint32_t)copy,off,chunk,n))return 2;}else memset(chunk,0,n);
        if(st_stream_write_at(&w,off,chunk,n))return 2;off+=n;
    }
    memcpy(header,"SMP5",4);header[4]=2;
    if(st_stream_write_at(&w,0,header,16)||st_stream_write_at(&w,SM_CONFIG_OFF,config,sizeof config))return 2;
    uint32_t crc=0xffffffffu;if(proj_stream_read_hash(w.obj,w.head.slot,0,SM_PROGRAM_BANK,0,&crc))return 2;
    st_stream_seal(&w,crc);return st_stream_publish(&w)?2:0;
}
/* Sound-only template payload: no sequencer banks or note recordings. The
 * existing template validator checks native patches without changing autosave. */
static int sm_program_pack_sound(const project_t *p)
{
    uint8_t *raw=(uint8_t *)proj_image_store();tmpl_t *out=(tmpl_t *)(raw+12);
    memcpy(raw,p->name,12);memset(out,0,sizeof *out);
    for(uint32_t i=0;i<PROJ_GLOBALS;i++)out->g[i]=GP[i].def;out->sel=p->sel<4?p->sel:0;out->ntrk=4;
    int8_t oct[4];for(uint32_t k=0;k<4;k++){
        oct[k]=p->t[k].mpc_octave;out->t[k].engine=p->t[k].engine;out->t[k].preset=p->t[k].preset;
        memcpy(out->t[k].p,p->t[k].p,sizeof out->t[k].p);
        for(uint32_t i=P_SLEN;i<=P_SGATE;i++)out->t[k].p[i]=TP[i].def;
    }
    out->g[TMPL_MPC_OCT_G]=(int16_t)mpc_octaves_pack(oct);
    memcpy(out->fm6,p->fm6,sizeof out->fm6);memcpy(out->fm6_fn,p->fm6_fn,sizeof out->fm6_fn);
    memcpy(out->cz,p->cz,sizeof out->cz);memcpy(out->p5,p->p5,sizeof out->p5);memcpy(out->drum,p->drum,sizeof out->drum);
    for(uint32_t i=0;i<12;i++)out->master[i]=(uint8_t)p->g[G_MFXTYPE+i];
    out->master[12]=(uint8_t)p->g[G_FXRETURN];out->master[13]=(uint8_t)p->g[G_FXMODE];
    for(uint32_t i=0;i<3;i++)out->master[14+i]=(uint8_t)(p->g[G_MFXTONE+i]+64);
    out->size=sizeof *out;out->magic=TMPL_MAGIC;return tmpl_blob_valid((const uint8_t *)out,sizeof *out);
}
static int sm_program_unpack_sound(project_t *p)
{
    const uint8_t *raw=(const uint8_t *)proj_image_store();const tmpl_t *in=(const tmpl_t *)(raw+12);
    if(!tmpl_blob_valid((const uint8_t *)in,sizeof *in))return 0;
    for(uint32_t i=0;i<12;i++)if(raw[i]&&(raw[i]<32||raw[i]>126))return 0;
    memset(p,0,sizeof *p);memcpy(p->name,raw,12);memcpy(p->g,in->g,sizeof in->g);p->sel=in->sel;
    for(uint32_t k=0;k<4;k++){p->t[k].engine=in->t[k].engine;p->t[k].preset=in->t[k].preset;memcpy(p->t[k].p,in->t[k].p,sizeof in->t[k].p);}
    memcpy(p->fm6,in->fm6,sizeof in->fm6);memcpy(p->fm6_fn,in->fm6_fn,sizeof in->fm6_fn);
    memcpy(p->cz,in->cz,sizeof in->cz);memcpy(p->p5,in->p5,sizeof in->p5);memcpy(p->drum,in->drum,sizeof in->drum);
    for(uint32_t i=0;i<12;i++)p->g[G_MFXTYPE+i]=in->master[i];
    p->g[G_FXRETURN]=in->master[12];p->g[G_FXMODE]=in->master[13];
    for(uint32_t i=0;i<3;i++)p->g[G_MFXTONE+i]=(int16_t)in->master[14+i]-64;
    return 1;
}
/* rc: 0 saved, 1 bad input, 2 storage error, 3 protected legacy project, 4 busy. */
static int sm_program_save(uint32_t slot,const char *name,int tuning_override)
{
    if(slot>=SM_PROGRAM_COUNT||!name||!name[0])return 1;
    if(transport_busy()||wire_waiting())return 4;
    proj_wire_gen++;
    st_hdr_t previous;uint8_t header[16]={0};int copy=sm_program_bank(slot/2,&previous,header);
    if(copy==-2)return 3;
    uint8_t meta[SM_PROGRAM_META];sm_program_meta_capture(meta,tuning_override);if(!sm_program_meta_valid(meta))return 1;
    uint32_t guard=motion_guard();project_capture(&proj_scratch);motion_unguard(guard);
    memset(proj_scratch.name,0,sizeof proj_scratch.name);
    for(uint32_t i=0;i<12&&name[i];i++){if(name[i]<32||name[i]>126)return 1;proj_scratch.name[i]=name[i];}
    proj_scratch.sum=proj_sum(&proj_scratch);
    if(!sm_program_pack_sound(&proj_scratch))return 1;
    /* Avoid erase cycles when an idle autosave sees an unchanged program. */
    if(copy>=0&&(header[5]&(1u<<(slot%2u)))){
        uint32_t base=16u+(slot%2u)*SM_PROGRAM_RECORD;uint8_t probe[128];int equal=1;
        for(uint32_t off=0;off<SM_SOUND_BYTES&&equal;off+=sizeof probe){uint32_t n=SM_SOUND_BYTES-off;if(n>sizeof probe)n=sizeof probe;
            if(st_read_range(OBJ_BANK0+slot/2,(uint32_t)copy,base+off,probe,n)||memcmp(probe,(const uint8_t *)proj_image_store()+off,n))equal=0;}
        for(uint32_t off=0;off<sizeof meta&&equal;off+=sizeof probe){uint32_t n=sizeof meta-off;if(n>sizeof probe)n=sizeof probe;
            if(st_read_range(OBJ_BANK0+slot/2,(uint32_t)copy,base+SM_SOUND_BYTES+off,probe,n)||memcmp(probe,meta+off,n))equal=0;}
        if(equal){sm_program_current=(uint8_t)slot;memcpy(sm_program_meta,meta,sizeof meta);proj_name_get(sm_program_name,(const uint8_t *)proj_scratch.name);return 0;}
    }
    st_stream_t w;if(st_stream_begin(&w,OBJ_BANK0+slot/2,SM_PROGRAM_BANK))return 2;
    uint32_t at=16u+(slot%2u)*SM_PROGRAM_RECORD;
    if(st_stream_write_at(&w,at,proj_image_store(),SM_SOUND_BYTES)||st_stream_write_at(&w,at+SM_SOUND_BYTES,meta,sizeof meta))return 2;
    uint8_t *chunk=wire_take();uint32_t other=16u+((slot%2u)^1u)*SM_PROGRAM_RECORD;
    for(uint32_t off=0;off<SM_PROGRAM_RECORD;){uint32_t n=SM_PROGRAM_RECORD-off;if(n>sizeof proj_wire_u.raw)n=sizeof proj_wire_u.raw;
        if(copy>=0){if(st_read_range(w.obj,(uint32_t)copy,other+off,chunk,n))return 2;}else memset(chunk,0,n);
        if(st_stream_write_at(&w,other+off,chunk,n))return 2;off+=n;
    }
    uint8_t config[SM_CONFIG_BYTES];sm_config_pack(config);
    if(st_stream_write_at(&w,SM_CONFIG_OFF,config,sizeof config))return 2;
    memcpy(header,"SMP5",4);header[4]=2;header[5]|=(uint8_t)(1u<<(slot%2u));
    if(st_stream_write_at(&w,0,header,sizeof header))return 2;
    uint32_t crc=0xffffffffu;if(proj_stream_read_hash(w.obj,w.head.slot,0,SM_PROGRAM_BANK,0,&crc))return 2;
    st_stream_seal(&w,crc);if(st_stream_publish(&w))return 2;
    sm_program_current=(uint8_t)slot;memcpy(sm_program_meta,meta,sizeof meta);proj_name_get(sm_program_name,(const uint8_t *)proj_scratch.name);
    return 0;
}
static int sm_program_read(uint32_t slot,uint8_t *meta)
{
    if(slot>=SM_PROGRAM_COUNT)return 1;
    st_hdr_t h;uint8_t header[16];int copy=sm_program_bank(slot/2,&h,header);
    if(copy<0||!(header[5]&(1u<<(slot%2u))))return copy==-2?3:1;
    uint32_t at=16u+(slot%2u)*SM_PROGRAM_RECORD;
    if(st_read_range(OBJ_BANK0+slot/2,(uint32_t)copy,at,proj_image_store(),SM_SOUND_BYTES)||
       st_read_range(OBJ_BANK0+slot/2,(uint32_t)copy,at+SM_SOUND_BYTES,meta,SM_PROGRAM_META))return 2;
    if(!sm_program_meta_valid(meta)||!sm_program_unpack_sound(&proj_scratch))return 1;
    return 0;
}
static int sm_program_stream_valid(uint32_t obj,uint32_t copy)
{
    uint8_t header[16],meta[SM_PROGRAM_META],config[SM_CONFIG_BYTES];
    if(st_read_range(obj,copy,0,header,16)||memcmp(header,"SMP5",4)||header[4]!=2||header[5]>3)return 0;
    for(uint32_t i=6;i<16;i++)if(header[i])return 0;
    for(uint32_t k=0;k<2;k++)if(header[5]&(1u<<k)){
        uint32_t off=16+k*SM_PROGRAM_RECORD;
        if(st_read_range(obj,copy,off,proj_image_store(),SM_SOUND_BYTES)||!sm_program_unpack_sound(&proj_scratch)||
           st_read_range(obj,copy,off+SM_SOUND_BYTES,meta,sizeof meta)||!sm_program_meta_valid(meta))return 0;
    }
    return !st_read_range(obj,copy,SM_CONFIG_OFF,config,sizeof config)&&sm_config_valid(config);
}
static int sm_program_load(uint32_t slot)
{
    if(transport_busy()||wire_waiting())return 4;
    proj_wire_gen++;
    uint8_t meta[SM_PROGRAM_META];int rc=sm_program_read(slot,meta);if(rc)return rc==1?sm_factory_program(slot):rc;
    /* A program changes the four sounds and FX, not the running sequencer banks. */
    project_t *p=&proj_scratch;sm_layers_t layers;sm_tuning_t tuning;
    if(!sm_layers_decode(&layers,meta+4)||!sm_tuning_decode(&tuning,meta+20))return 1;
    uint32_t guard=motion_guard();panic_req|=15u;
    for(uint32_t k=0;k<4;k++){
        track_t *t=&trk[k];t->eng_req=p->t[k].engine;t->preset=p->t[k].preset;t->user=0;t->user_native=0;
        for(uint32_t i=0;i<P_COUNT;i++)if(i<P_SLEN||i>P_SGATE){
            const param_desc_t *d=param_desc_of(t->eng_req,i);
            t->p[i]=param_reserved(i)?0:(int16_t)clamp(p->t[k].p[i],d->min,d->max);
        }
        track_fx_normalize(t->p);
        *cz_patch_of(k)=p->cz[k];cz_track_accept(t);p5_patch[k]=p->p5[k];p5_ready[k]=1;*drum_patch_of(k)=p->drum[k];
        if(t->eng_req==ENGI_HYBRID){hy_patch_t v;memcpy(&v,p->fm6[k],sizeof v);hy_set_patch(k,&v);}
        else if(t->eng_req==ENGI_FM6){uint8_t v[FP_SIZE+1];fm6_unpack(p->fm6[k],v);fm6_set_patch(k,v);}
        memcpy(fm6_fn[k],p->fm6_fn[k],FM6_NFN);
    }
    for(uint32_t i=0;i<FX_SLOTS*FX_PARAMS;i++)song.g[G_MFXTYPE+i]=p->g[G_MFXTYPE+i];
    song.g[G_FXRETURN]=p->g[G_FXRETURN];song.g[G_FXMODE]=p->g[G_FXMODE];for(uint32_t i=0;i<FX_SLOTS;i++)song.g[G_MFXTONE+i]=p->g[G_MFXTONE+i];
    sm_layers=layers;if(meta[0])sm_tuning=tuning;song.sel=p->sel<4?p->sel:0;motion_unguard(guard);
    memcpy(sm_program_meta,meta,sizeof meta);sm_program_current=(uint8_t)slot;sm_scene_current=meta[1];proj_name_get(sm_program_name,(const uint8_t *)p->name);
    undo_clear();list_forget();sync_reload=1;ui.force=1;return 0;
}
#else
static void sm_config_load(void){}
static int sm_config_save(void){return 2;}
static int sm_program_save(uint32_t s,const char *n,int t){(void)s;(void)n;(void)t;return 2;}
static int sm_program_load(uint32_t s){return sm_factory_program(s);}
#endif
static int sm_program_name_at(uint32_t slot,char *name)
{
    name[0]=0;
#if MELODEE_FLASH
    st_hdr_t h;uint8_t header[16];int copy=sm_program_bank(slot/2,&h,header);
    if(slot>=8)return 1;
    if(copy==-2)return 3;
    if(copy<0||!(header[5]&(1u<<(slot%2)))){memcpy(name,SM_FACTORY_PROGRAM_NAMES[slot],13);return 0;}
    uint8_t raw[12];if(st_read_range(OBJ_BANK0+slot/2,(uint32_t)copy,16u+(slot%2u)*SM_PROGRAM_RECORD,raw,12))return 2;
    proj_name_get(name,raw);return 0;
#else
    if(slot>=8)return 1;memcpy(name,SM_FACTORY_PROGRAM_NAMES[slot],13);return 0;
#endif
}

static int sm_program_step(int direction)
{
    char name[13];for(uint32_t n=1;n<=8;n++){
        uint32_t slot=(uint32_t)((int32_t)(sm_program_current<8?sm_program_current:direction>0?7:0)+direction*(int)n+16)%8;
        if(!sm_program_name_at(slot,name))return sm_program_load(slot);
    }return 1;
}
static int sm_scene_step(int direction)
{
    for(uint32_t n=1;n<=4;n++){uint32_t slot=(uint32_t)((int)(sm_scene_current<4?sm_scene_current:direction>0?3:0)+direction*(int)n+8)%4;
        if(sm_program_meta[2]&(1u<<slot))return sm_scene_load(slot);
    }return 1;
}
static int sm_setlist_step(int direction)
{
    if(!sm_setlist_count)return 1;
    uint32_t next=(uint32_t)((int)sm_setlist_pos+direction+(int)sm_setlist_count)%sm_setlist_count;
    int rc=sm_program_load(sm_setlist[next]);if(!rc)sm_setlist_pos=(uint8_t)next;return rc;
}
static void sm_mapping_poll(void)
{
    /* At most four foreground actions per UI pass. The ISR never touches flash. */
    for(uint32_t budget=0;budget<4&&sm_map_r!=sm_map_w;budget++){
        uint32_t q=sm_map_q[sm_map_r++%SM_MAP_Q],slot=q&31u,value=(q>>5)&127u;
        if((uint8_t)(q>>12)!=sm_map_generation||slot>=SM_MAP_COUNT)continue;
        const uint8_t *m=sm_maps[slot];if(!m[0])continue;
        uint32_t part=m[6]<4?m[6]:song.sel%4;track_t *t=&trk[part];int rc=0;
        if(m[5]){
            if(m[10]!=255&&m[10]!=t->eng_req)continue;
            const param_desc_t *d=param_desc_of(t->eng_req,m[7]);
            if(!d||param_reserved(m[7]))continue;
            uint32_t guard=motion_guard();t->p[m[7]]=(int16_t)(d->min+(int32_t)(value*(uint32_t)(d->max-d->min)+63)/127);track_fx_normalize(t->p);motion_unguard(guard);
        }else switch(m[7]){
            case 0:case 1:{uint32_t count=ENGINES[t->eng_req]->npresets;if(count){int dir=m[7]? -1:1;load_begin(t,UNDO_SOUND);apply_preset_to(t,(uint32_t)((int)t->preset+dir+(int)count)%count);load_end(t);}break;}
            case 2:rc=sm_program_step(1);break;
            case 3:rc=sm_program_step(-1);break;
            case 4:rc=sm_scene_step(1);break;
            case 5:rc=sm_scene_step(-1);break;
            case 6:sm_layers.enabled^=1;settings_save();break;
            case 7:rc=sm_setlist_step(1);break;
            case 8:rc=sm_setlist_step(-1);break;
            case 9:panic_req|=15u;break;
        }
        if(rc)ui_message(rc==4?"STOP TO LOAD":"MIDI ACTION FAILED");
        sync_reload=1;ui.force=1;
    }
}

static void sm_program_autosave(void)
{
    if(sm_program_current>=8||!sm_program_name[0]||transport_busy()||wire_waiting())return;
    if(!sm_program_save(sm_program_current,sm_program_name,sm_program_meta[0])){sm_config_save();ui_message("PROGRAM SAVED");}
}

static void sm_program_boot(void)
{
    if(sm_program_boot_slot<8)sm_program_load(sm_program_boot_slot);
}
