/* SPDX-License-Identifier: GPL-3.0-only */
#define ED_SM_PROGRAM 112u
/* op 0 status; 1 load; 2 save(name, optional tuning); 3 store scene;
 * 4 recall scene. Named saves require printable ASCII, 1..12 bytes. */
static int ed_sm_program_handle(uint32_t cmd,const uint8_t *a,uint32_t n)
{
    if(cmd!=ED_SM_PROGRAM)return 0;
    uint32_t op=n?a[0]:127,slot=n>1?a[1]:127,rc=1;char name[13]={0};
    if(op==0&&n==1)rc=0;
    else if(op==1&&n==2)rc=(uint32_t)sm_program_load(slot);
    else if(op==4&&n==2)rc=(uint32_t)sm_scene_load(slot);
    else if((op==2&&n>=4&&n<=15&&a[2]<=1)||(op==3&&n>=3&&n<=14)){
        uint32_t start=op==2?3:2,len=n-start,valid=1;
        for(uint32_t i=0;i<len;i++){if(a[start+i]<32||a[start+i]>126)valid=0;name[i]=(char)a[start+i];}
        if(valid)rc=op==2?(uint32_t)sm_program_save(slot,name,a[2]):(uint32_t)sm_scene_store(slot,name);
    }
    ed_b(op);ed_b(rc);ed_b(sm_program_current<8?sm_program_current:127);ed_b(sm_scene_current<4?sm_scene_current:127);ed_b(sm_program_meta[0]);ed_str(sm_program_name,12);
    for(uint32_t i=0;i<8;i++){int status=sm_program_name_at(i,name);ed_b((uint32_t)status);ed_str(name,12);}
    ed_b(sm_program_meta[2]);for(uint32_t i=0;i<4;i++)ed_str((const char *)sm_program_meta+48+i*SM_SCENE_BYTES,12);
    return 1;
}
