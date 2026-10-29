/* SPDX-License-Identifier: GPL-3.0-only */
static uint8_t sm_controller_override,sm_controller_port;
/* SynthMaven routing. USB=0, TRS=1. Pedal messages are USB-only. */
#ifndef SYNTHMAVEN_MIDI_H
#define SYNTHMAVEN_MIDI_H
#define SM_CAP_USB_DEVICE 1u
#define SM_CAP_TRS 2u
#define SM_CAP_BLE_CENTRAL 4u
#define SM_CAP_USB_HOST 8u
#define SM_CAP_BLE_EDITOR 16u
#ifndef SM_BLE
#define SM_BLE 0
#endif
#ifndef FELUCCA_USB_HOST
#define FELUCCA_USB_HOST 0
#endif
#define SM_CAPS (SM_CAP_USB_DEVICE | SM_CAP_TRS | (SM_BLE ? SM_CAP_BLE_EDITOR : 0u) | (FELUCCA_USB_HOST ? SM_CAP_USB_HOST : 0u))
#if SM_BLE
static int sm_ble_pending(void), sm_ble_read(uint32_t *);
static int sm_ble_editor_busy(void), sm_ble_editor_frame(const uint8_t **,uint32_t *);
static void sm_ble_editor_done(void), sm_ble_editor_send(const uint8_t *,uint32_t);
static uint32_t sm_ble_state(void), sm_ble_scan_start(void), sm_ble_connect_device(uint32_t);
static int sm_ble_device(uint32_t, char *, int *);
static void sm_ble_poll(void), sm_ble_disconnect_device(void);
static void sm_ble_diagnostics(uint32_t *);
#endif
static uint8_t sm_patch_port; /* USB only; old routing values migrate on load. */
static uint8_t sm_usb_host; /* 0 COMPUTER/device, 1 PEDAL/host */
static uint8_t sm_key_port = 2;   /* 0 USB, 1 TRS, 2 BOTH */
static uint8_t sm_bluetooth;
/* Four pedal actions: patch +/-, bank +/-. Type 0 disables, 1 CC, 2 PC.
 * Channel 0 follows IN CH; 1..16 explicitly selects a pedal channel. */
#define SM_ACTIONS 10u
static const char *const SM_ACTION_NAME[SM_ACTIONS]={"PATCH UP","PATCH DOWN","BANK UP","BANK DOWN","PROGRAM NEXT","PROGRAM PREV","SCENE NEXT","SCENE PREV","SETLIST NEXT","SETLIST PREV"};
static uint8_t sm_pedal[SM_ACTIONS][4];
static uint16_t sm_repeat_ms; /* 0 disables suppression; identical accepted events only. */
static uint32_t sm_repeat_at[SM_ACTIONS];static uint8_t sm_repeat_valid[SM_ACTIONS];
static volatile uint8_t sm_learn_state,sm_learn_action,sm_candidate[4],sm_last[4],sm_last_match=127,sm_last_result;
static volatile uint32_t sm_last_at,sm_trigger_at,sm_trigger_count;
static uint8_t sm_usb_device_seen,sm_usb_midi_seen;
static uint8_t sm_setlist[32],sm_setlist_len,sm_setlist_pos,sm_setlist_active;
static uint8_t sm_current_program=255;
static uint8_t sm_learn_arm(uint32_t action){if(action>=SM_ACTIONS)return 1;sm_learn_action=(uint8_t)action;sm_learn_state=1;return 0;}
static int sm_map_conflict(const uint8_t *m,uint32_t action){uint32_t i;if(!m[0])return 0;for(i=0;i<SM_ACTIONS;i++){const uint8_t *o=sm_pedal[i];if(i!=action&&o[0]==m[0]&&o[1]==m[1]&&(m[0]==2||o[2]==m[2])&&(!o[3]||!m[3]||o[3]==m[3]))return 1;}return 0;}
static uint8_t sm_learn_accept(void){if(sm_learn_state!=2)return 1;if(sm_map_conflict((const uint8_t *)sm_candidate,sm_learn_action))return 2;uint32_t i;for(i=0;i<4;i++)sm_pedal[sm_learn_action][i]=sm_candidate[i];sm_learn_state=0;memset(sm_repeat_valid,0,sizeof sm_repeat_valid);return 0;}

static volatile uint32_t sm_action_w, sm_action_r;
static uint8_t sm_actions[16];
static int sm_pedal_receive(uint32_t st,uint32_t ch,uint32_t n,uint32_t v,uint32_t port,int accepted)
{
    uint32_t i;int enabled=accepted>=0; /* -1 observes while PATCH MIDI is disabled. */
    if(port != 0u)return 0;
    sm_last[0]=(uint8_t)st;sm_last[1]=(uint8_t)(ch+1u);sm_last[2]=(uint8_t)n;sm_last[3]=(uint8_t)v;sm_last_at=fm1_ms;sm_last_match=127;sm_last_result=0;
    if(sm_learn_state==1 && (st==0xB0u || st==0xC0u)){
        sm_candidate[0]=st==0xB0u?1:2;sm_candidate[1]=(uint8_t)n;sm_candidate[2]=st==0xB0u?(uint8_t)v:0;sm_candidate[3]=(uint8_t)(ch+1u);sm_learn_state=2;sm_last_result=3;return 1;
    }
    if(sm_learn_state==2 && (st==0xB0u || st==0xC0u))return 1; /* candidate cannot trigger an old assignment */
    for(i=0;i<SM_ACTIONS;i++) {
        const uint8_t *m=sm_pedal[i];
        if(!m[0] || (m[3] ? ch+1u!=m[3] : accepted<=0))continue;
        if(m[0]==1u && st==0xB0u && n==m[1] && v!=m[2]){sm_repeat_valid[i]=0;continue;} /* release re-arms immediately */
        if(((m[0]==1u && st==0xB0u && v==m[2]) || (m[0]==2u && st==0xC0u)) && n==m[1]) {
            sm_last_match=(uint8_t)i;
            if(!enabled){sm_last_result=4;return 0;}
            if(sm_repeat_ms && sm_repeat_valid[i] && (uint32_t)(fm1_ms-sm_repeat_at[i])<sm_repeat_ms){sm_last_result=2;return 1;}
            if(sm_action_w-sm_action_r>=16u){sm_last_result=5;return 1;}
            sm_repeat_at[i]=fm1_ms;sm_repeat_valid[i]=1;sm_last_result=1;sm_trigger_at=fm1_ms;sm_trigger_count++;
            sm_actions[sm_action_w%16u]=(uint8_t)i;__asm__ volatile("" ::: "memory");sm_action_w++;return 1;
        }
    }
    return 0;
}

static uint32_t sm_set(uint32_t patch_port, uint32_t key_port, uint32_t bluetooth)
{
    if (patch_port > 2u || key_port > 2u || bluetooth > 1u) return 1;
    if (patch_port == 1u || (bluetooth && !SM_BLE)) return 4;
    sm_patch_port = 0;
    sm_key_port = (uint8_t)key_port;
    sm_bluetooth = (uint8_t)bluetooth;
    return 0;
}
static int sm_patch_accept(uint32_t port)
{
    return port == 0u;
}
static int sm_key_accept(uint32_t port)
{
    return port < 2u && (sm_key_port == 2u || port == sm_key_port);
}
#endif
