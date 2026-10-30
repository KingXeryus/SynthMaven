/* SPDX-License-Identifier: GPL-3.0-only */
/* SynthMaven capability-gated native HYBRID protocol. No flash writes here.
 * 96: op (0 read track, 1 write track, 2 read factory), index, packed bytes.
 * Reply: op, index, rc, then a 7-bit-packed 128-byte patch for reads.
 * rc: 0 success, 1 malformed/range, 2 wrong engine. */
#define ED_HYBRID 96u
static int ed_hybrid_handle(uint32_t cmd,const uint8_t *a,uint32_t n)
{
    if(cmd!=ED_HYBRID)return 0;
    uint32_t op=n?a[0]:127u,index=n>1u?a[1]:127u,rc=0;
    hy_patch_t patch;
    if(n<2u || op>2u || (op==2u?index>=HY_NFACTORY:index>=NPART))rc=1;
    else if(op!=2u && trk[index].eng_req!=ENGI_HYBRID)rc=2;
    else if(op==1u){
        if(ed_unpack7(a+2,n-2,(uint8_t *)&patch,sizeof patch)!=sizeof patch || !hy_patch_valid(&patch))rc=1;
        else {
            momentary_restore();
            uint32_t guard=motion_guard();
            hy_set_patch(index,&patch);
            motion_unguard(guard);
            ui.force=1;
        }
    }else if(n!=2u)rc=1;
    else if(op==2u)patch=HY_FACTORY[index];
    else {uint32_t guard=motion_guard();patch=*hy_patch_of(index);motion_unguard(guard);}
    ed_b(op);ed_b(index);ed_b(rc);
    if(!rc && op!=1u)ed_bk_pack((const uint8_t *)&patch,sizeof patch);
    return 1;
}
