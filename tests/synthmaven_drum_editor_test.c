/* SPDX-License-Identifier: GPL-3.0-only */
#define BACKUP_TEST_NO_MAIN
#include "backup_test.c"
#include "../firmware/src/editor_drums.c"
#include <assert.h>
int main(void)
{
    reset();sm_drums.enabled=0;
    uint8_t a[64]={0};rep_n=0;assert(ed_sm_drums_handle(116,a,1));assert(rep[1]==0&&rep[2]==0);
    sm_drum_config_t saved=sm_drums;
    a[0]=1;a[1]=127;rep_n=0;ed_sm_drums_handle(116,a,2);
    assert(rep[1]==1&&!memcmp(&saved,&sm_drums,sizeof saved));
    a[0]=2;a[1]=1;rep_n=0;ed_sm_drums_handle(116,a,2);assert(rep[1]==0&&sm_drums.enabled);
    a[0]=4;rep_n=0;ed_sm_drums_handle(116,a,1);assert(rep[1]==4); /* no erase while playing */
    a[0]=2;a[1]=0;rep_n=0;ed_sm_drums_handle(116,a,2);assert(rep[1]==0&&!sm_drums.enabled);
    a[0]=4;rep_n=0;ed_sm_drums_handle(116,a,1);assert(rep[1]==0);
    a[0]=3;a[1]=0;a[2]=100;rep_n=0;ed_sm_drums_handle(116,a,3);assert(rep[1]==0&&sm_drum_audio.lane[0].on);
    a[0]=1;rep_n=0;ed_sm_drums_handle(117,a,1);assert(rep[1]==0&&transport_req==1);
    a[0]=2;rep_n=0;ed_sm_drums_handle(117,a,1);assert(rep[1]==0&&transport_req==2);
    a[0]=7;rep_n=0;ed_sm_drums_handle(117,a,1);assert(rep[1]==1&&transport_req==2);
    puts("Drum editor: read, invalid edits, start/stop, audition and stopped-only save passed");return 0;
}
