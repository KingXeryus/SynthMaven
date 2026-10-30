/* SPDX-License-Identifier: GPL-3.0-only */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
#include <assert.h>
int main(void)
{
 reset();memset(sm_maps,0,sizeof sm_maps);sm_map_w=sm_map_r=sm_map_seen=sm_map_released=0;
 uint8_t rule[12]={1,0,0,80,127,0,4,6,100,0,255,0};
 assert(sm_map_valid(rule));memcpy(sm_maps[0],rule,12);assert(sm_map_conflict(1,rule)==0);
 fm1_ms=1000;assert(sm_map_receive(0xb0,0,80,127,1));assert(sm_map_w==1);
 fm1_ms=1001;assert(sm_map_receive(0xb0,0,80,127,1));assert(sm_map_w==1);
 sm_layers.enabled=0;sm_mapping_poll();assert(sm_layers.enabled==1);
 fm1_ms=1002;assert(sm_map_receive(0xb0,0,80,0,1));
 fm1_ms=1003;assert(sm_map_receive(0xb0,0,80,127,1));sm_mapping_poll();assert(!sm_layers.enabled);
 sm_map_learn=1;sm_map_receive(0xb0,2,43,92,1);uint32_t captured=sm_midi_learned;
 sm_map_receive(0xb0,2,43,0,1);assert(sm_map_learn==2&&sm_midi_learned==captured);sm_map_learn=0;
 rule[3]=43;rule[4]=128;rule[5]=1;rule[6]=2;rule[7]=P_LEVEL;memcpy(sm_maps[1],rule,12);
 assert(sm_map_receive(0xb0,0,43,75,1));sm_mapping_poll();assert(trk[2].p[P_LEVEL]==75);
 assert(!sm_map_receive(0xb0,1,43,75,1));
 sm_controller_override=CT_HUI;sm_controller_port=3;assert(!sm_config_save());sm_controller_override=sm_controller_port=0;sm_config_load();assert(sm_controller_override==CT_HUI&&sm_controller_port==3);
 assert(!sm_config_save());memset(sm_maps,0,sizeof sm_maps);sm_config_load();assert(sm_maps[1][7]==P_LEVEL&&sm_maps[0][3]==80);
 assert(!sm_program_save(0,"MAPPED",0));memset(sm_maps,0,sizeof sm_maps);sm_config_load();assert(sm_maps[1][3]==43);
 sm_setlist_count=3;sm_setlist[0]=0;sm_setlist[1]=3;sm_setlist[2]=0;assert(!sm_config_save());sm_setlist_count=0;sm_config_load();assert(sm_setlist_count==3&&sm_setlist[1]==3);assert(!sm_program_load(0));
 for(int cut=0;cut<100;cut+=9){sm_maps[0][3]=81;fail_after=cut;int rc=sm_config_save();fail_after=-1;sm_config_load();assert(sm_maps[0][3]==(rc?80:81));assert(!sm_program_load(0));sm_maps[0][3]=80;assert(!sm_config_save());}
 rule[5]=0;rule[7]=0;rule[4]=127;rule[11]=2;rule[3]=82;
 assert(sm_map_valid(rule));memcpy(sm_maps[2],rule,12);
 assert(!sm_map_receive(0xb0,0,82,127,1));
 assert(sm_map_receive(0xb0,0,82,127,2));
 rule[11]=1;assert(sm_map_conflict(3,rule)==-1);
 rule[11]=0;assert(sm_map_conflict(3,rule)==2);
 sm_map_r=sm_map_w;assert(!sm_config_save());memset(sm_maps,0,sizeof sm_maps);sm_config_load();assert(sm_maps[2][11]==2);
 puts("MIDI maps: bounded deferred actions, duplicate suppression, deliberate taps, learn latch, parameter routing, conflicts, durable config and interrupted saves passed");return 0;
}
