/* SPDX-License-Identifier: GPL-3.0-only */
#define ED_SM_MAPPING 113u
#define ED_SM_SETLIST 114u
#define ED_SM_CONTROLLER 115u
static int ed_sm_mapping_handle(uint32_t cmd,const uint8_t *a,uint32_t n)
{
    if(cmd==ED_SM_CONTROLLER){
        uint32_t rc=1,op=n?a[0]:127;
        if(op==0&&n==1)rc=0;
        else if(op==1&&n==3&&a[1]<=3&&a[2]<=16){
            uint8_t mode=sm_controller_override,port=sm_controller_port;
            sm_controller_override=a[1];sm_controller_port=a[2];rc=(uint32_t)sm_config_save();
            if(rc){sm_controller_override=mode;sm_controller_port=port;}
            else{uint32_t guard=motion_guard();sm_controller_mode(a[1]);panic_req|=15u;motion_unguard(guard);}
        }
        ed_b(op);ed_b(rc);ed_b(ct_connected);ed_b(sm_controller_override);ed_b(sm_controller_port);ed_b(ct_mode);ed_b(ct_daw==255?127:ct_daw);
        ed_str(ct_product,47);ed_str(ct_family,23);return 1;
    }
    if(cmd!=ED_SM_MAPPING&&cmd!=ED_SM_SETLIST)return 0;
    uint32_t op=n?a[0]:127,rc=1,conflict=127;
    if(cmd==ED_SM_SETLIST){
        if(op==0&&n==1)rc=0;
        else if(op==1&&n>=2&&a[1]<=32&&n==2u+a[1]){
            uint8_t old[32],count=sm_setlist_count,pos=sm_setlist_pos;uint32_t valid=1;
            for(uint32_t i=2;i<n;i++)if(a[i]>=8)valid=0;
            if(valid){memcpy(old,sm_setlist,32);memset(sm_setlist,0,32);memcpy(sm_setlist,a+2,a[1]);sm_setlist_count=a[1];sm_setlist_pos=0;rc=(uint32_t)sm_config_save();if(rc){memcpy(sm_setlist,old,32);sm_setlist_count=count;sm_setlist_pos=pos;}}
        }else if(op==2&&n==2&&a[1]<2)rc=(uint32_t)sm_setlist_step(a[1]?1:-1);
        ed_b(op);ed_b(rc);ed_b(sm_setlist_count);ed_b(sm_setlist_pos);for(uint32_t i=0;i<sm_setlist_count;i++)ed_b(sm_setlist[i]);return 1;
    }
    if(op==0&&n==1)rc=0;
    else if(op==1&&n==2&&a[1]<=1){sm_map_learn=a[1];rc=0;}
    else if(op==2&&n>2&&a[1]<SM_MAP_COUNT){
        uint8_t record[SM_MAP_BYTES],old[SM_MAP_BYTES];uint32_t slot=a[1];
        if(ed_unpack7(a+2,n-2,record,sizeof record)==sizeof record&&sm_map_valid(record)){
            int c=sm_map_conflict(slot,record);if(c>=0){conflict=(uint32_t)c;rc=5;}
            else{
                memcpy(old,sm_maps[slot],sizeof old);uint32_t guard=motion_guard();memcpy(sm_maps[slot],record,sizeof record);sm_map_generation++;sm_map_r=sm_map_w;motion_unguard(guard);
                rc=(uint32_t)sm_config_save();if(rc){guard=motion_guard();memcpy(sm_maps[slot],old,sizeof old);sm_map_generation++;motion_unguard(guard);}else{sm_map_learn=0;sm_map_seen=sm_map_released=0;}
            }
        }
    }
    ed_b(op);ed_b(rc);ed_b(conflict);ed_b(sm_map_learn);ed_b(sm_midi_match);
#if MELODEE_USB_HOST
    ed_b(uh.on);ed_b(uh.seen);ed_b(uh.run);
#else
    ed_b(0);ed_b(usb.up);ed_b(usb.up);
#endif
    uint32_t stats[]={sm_midi_last,sm_midi_learned,sm_midi_serial,sm_map_hits,sm_map_drops};
    for(uint32_t j=0;j<5;j++)for(uint32_t i=0;i<5;i++)ed_b(stats[j]>>(i*7));
    ed_bk_pack((const uint8_t *)sm_maps,sizeof sm_maps);return 1;
}
