/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
static void pedal(uint32_t st,uint32_t n,uint32_t v,uint32_t port,uint32_t ch) {
 midi_in_q[mi_w++%MQ]=(st>>4)|(port<<4)|((st|ch)<<8)|(n<<16)|(v<<24);frames(2);
}
int main(void) {
 uint32_t total,cur,first,last,n,k,count=0;
 outdir="build/host";bank_resolve();panel=PANEL_DEFAULT;layers_init();host_tracks_init();palette_set(4);go_home();frames(2);
 cur=preset_pos(&total);k=kind_at(cur);first=last=cur;
 for(n=0;n<total;n++)if(kind_at(n)==k){if(!count)first=n;last=n;count++;}
 preset_go(last);encs[panel.enc[EN_PRESET]]=1;frames(2);
 check(preset_pos(&total)==first,"HOME preset forward wraps within bank");
 encs[panel.enc[EN_PRESET]]=-1;frames(2);check(preset_pos(&total)==last,"HOME preset backward wraps within bank");
 check(home_preset_next(first,(int32_t)count*3+1)==home_preset_next(first,1),"large encoder delta wraps in bank");
 /* A BLE pedal CC is accepted independently of the TRS keyboard route. */
 sm_patch_port=1;sm_key_port=1;midi_pc_off=0;midi_in_ch=1;
 sm_pedal[0][0]=1;sm_pedal[0][1]=80;sm_pedal[0][2]=127;sm_pedal[0][3]=2;
 preset_go(first);cur=preset_pos(&total);
 pedal(0xB0,80,127,0,1);check(preset_pos(&total)==cur,"pedal ignores wrong source");
 pedal(0xB0,80,127,2,0);check(preset_pos(&total)==cur,"pedal ignores wrong channel");
 pedal(0xB0,80,0,2,1);check(preset_pos(&total)==cur,"CC release does not switch patch");
 pedal(0xB0,80,127,2,1);check(preset_pos(&total)==home_preset_next(cur,1),"BLE CC patch up bypasses keyboard port and channel");
 cur=preset_pos(&total);midi_pc_off=1;pedal(0xB0,80,127,2,1);check(preset_pos(&total)==cur,"PROG OFF blocks mapped action");midi_pc_off=0;
 sm_pedal[1][0]=2;sm_pedal[1][1]=12;sm_pedal[1][3]=2;
 pedal(0xC0,12,0,2,1);check(preset_pos(&total)==first,"PC patch down uses mapped action instead of absolute program");
 sm_pedal[2][0]=1;sm_pedal[2][1]=81;sm_pedal[2][2]=127;sm_pedal[2][3]=2;
 sm_pedal[3][0]=1;sm_pedal[3][1]=82;sm_pedal[3][2]=127;sm_pedal[3][3]=2;
 pedal(0xB0,81,127,2,1);check(kind_at(preset_pos(&total))!=k,"bank up selects next bank");
 pedal(0xB0,82,127,2,1);check(kind_at(preset_pos(&total))==k,"bank down returns to original bank");
 ui.force=1;frame();ppm("synthmaven-four-patches");
 open_family(FAM_GLO);while(strcmp(cur_page()->title,"PATCH UP"))open_family(FAM_GLO);
 {int16_t *v;syn_desc(X_PNUM,&v);*v=88;syn_changed(X_PNUM,v);check(sm_pedal[0][1]==88 && settings_later,"mapping editable from hardware and schedules save");}
 return fails?1:0;
}
