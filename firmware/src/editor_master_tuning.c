/* SPDX-License-Identifier: GPL-3.0-only */
/* 110: op 0 reads; op 1 applies a validated pack7(SM_TUNE_WIRE) record.
 * Reply op, rc, pack7(current record). Settings use the normal deferred
 * persistence path; rc 0 acknowledges application, not a completed erase. */
#define ED_MASTER_TUNING 110u
static int ed_master_tuning_handle(uint32_t cmd,const uint8_t *a,uint32_t n)
{
    if(cmd!=ED_MASTER_TUNING)return 0;
    uint32_t op=n?a[0]:127u,rc=0;
    uint8_t raw[SM_TUNE_WIRE];sm_tuning_t next;
    if(!n || op>1u || (!op && n!=1u))rc=1;
    else if(op){
        if(ed_unpack7(a+1,n-1,raw,sizeof raw)!=sizeof raw || !sm_tuning_decode(&next,raw))rc=1;
        else {
            uint32_t guard=motion_guard();sm_tuning=next;motion_unguard(guard);
            settings_save();ui.force=1;
        }
    }
    ed_b(op);ed_b(rc);
    sm_tuning_encode(raw,&sm_tuning);ed_bk_pack(raw,sizeof raw);
    return 1;
}
