/* SPDX-License-Identifier: GPL-3.0-only */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
int main(void)
{
    int bad=0;
    reset(); up_boot();
    set_engine_of(TSEL,ENGI_HYBRID); apply_preset_to(TSEL,41);
    hy_patch_t want=*hy_patch_of(0);
    want.osc[0].fine=-37; want.flt[0].cut=91;
    hy_set_patch(0,&want);
    TSEL->p[P_E4]=73; TSEL->p[P_E1]=-12;
    bad+=check("HYBRID accepts its complete native factory layout",hy_patch_valid(&want));
    bad+=check("HYBRID save includes pending patch and macros",!up_store(5,0) && up_used(5) && up_rec(5)->ver==UP_VER_HYBRID);
    up_cache_reset(); native_cache_reset(); up_boot();
    hy_patch_t other=HY_FACTORY[0]; hy_set_patch(0,&other); hy_block(TSEL);
    TSEL->p[P_E4]=0; TSEL->p[P_E1]=0;
    bad+=check("HYBRID native sound loads after bank cache reboot",!up_load(5));
    hy_block(TSEL);
    bad+=check("oscillators, filters, matrix and macro values survive save/load",
        !memcmp(hy_patch_of(0),&want,sizeof want) && TSEL->p[P_E4]==73 && TSEL->p[P_E1]==-12);
    up_rec_t invalid=*up_rec(5);
    ((hy_patch_t *)invalid.packed)->osc[0].type=255;
    uint32_t before=erases;
    bad+=check("invalid native HYBRID patch is rejected before flash erase",up_put(6,&invalid)==1 && erases==before && !up_used(6));
    want.osc[0].fine=24; hy_set_patch(0,&want); fail_after=2;
    bad+=check("interrupted HYBRID save reports failure",up_store(5,0)==2);
    fail_after=-1;up_cache_reset();native_cache_reset();up_boot();
    bad+=check("interrupted save retains previous durable HYBRID sound",!up_load(5) && hy_patch_of(0)->osc[0].fine==-37);
    reset(); up_boot();
    set_engine_of(TSEL,ENGI_HYBRID); apply_preset_to(TSEL,41);
    want=*hy_patch_of(0); want.osc[0].fine=-37; want.osc[1].coarse=-24;
    hy_set_patch(0,&want); TSEL->p[P_E4]=73;
    project_save_as(0,"HYBRID TEST");
    bad+=check("complete HYBRID project saves in the unchanged flash extent",project_used(0) && sizeof(project_store_t)==13212u);
    set_engine_of(TSEL,ENGI_FM6); apply_preset_to(TSEL,0);
    project_load(0); hy_block(TSEL);
    bad+=check("program restore preserves signed HYBRID fields and macro settings",TSEL->eng_req==ENGI_HYBRID && !memcmp(hy_patch_of(0),&want,sizeof want) && TSEL->p[P_E4]==73);
    static project_store_t encoded; static project_t decoded;
    bad+=check("HYBRID project wire payload is validated without 7-bit truncation",proj_import(&decoded,&proj_slot[0],sizeof proj_slot[0]) && proj_pack(&encoded,&decoded) && proj_import(&decoded,&encoded,sizeof encoded) && !memcmp(decoded.fm6[0],&want,sizeof want));
    ((hy_patch_t *)decoded.fm6[0])->osc[0].type=255;
    decoded.sum=proj_sum(&decoded);
    bad+=check("malformed HYBRID project rejected before import or save",!proj_ok(&decoded) && !proj_pack(&encoded,&decoded));
    undo_clear(); apply_preset_to(TSEL,42); undo_step(0);
    bad+=check("HYBRID sound undo restores the edited native patch",!memcmp(hy_patch_of(0),&want,sizeof want));
    undo_step(1);
    bad+=check("HYBRID redo returns to the browsed sound",memcmp(hy_patch_of(0),&want,sizeof want)!=0);
    return bad?1:0;
}
