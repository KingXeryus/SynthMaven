/* SPDX-License-Identifier: GPL-3.0-only */
/* SynthMaven routing. Cable tags: USB=0, TRS=1, BLE=2. Native USB host
 * is not supplied; BLE central is advertised only when its runtime is compiled. */
#ifndef SYNTHMAVEN_MIDI_H
#define SYNTHMAVEN_MIDI_H
#define SM_CAP_USB_DEVICE 1u
#define SM_CAP_TRS 2u
#define SM_CAP_BLE_CENTRAL 4u
#define SM_CAP_USB_HOST 8u
#ifndef SM_BLE
#define SM_BLE 0
#endif
#define SM_CAPS (SM_CAP_USB_DEVICE | SM_CAP_TRS | (SM_BLE ? SM_CAP_BLE_CENTRAL : 0u))
#if SM_BLE
static int sm_ble_pending(void), sm_ble_read(uint32_t *);
static uint32_t sm_ble_state(void), sm_ble_scan_start(void), sm_ble_connect_device(uint32_t);
static int sm_ble_device(uint32_t, char *, int *);
static void sm_ble_poll(void), sm_ble_disconnect_device(void);
#endif
static uint8_t sm_patch_port = 2; /* 0 USB, 1 BLE, 2 legacy ANY */
static uint8_t sm_key_port = 2;   /* 0 USB, 1 TRS, 2 BOTH */
static uint8_t sm_bluetooth;
/* Four pedal actions: patch +/-, bank +/-. Type 0 disables, 1 CC, 2 PC.
 * Channel 0 follows IN CH; 1..16 explicitly selects a pedal channel. */
static uint8_t sm_pedal[4][4];
static volatile uint32_t sm_action_w, sm_action_r;
static uint8_t sm_actions[16];
static int sm_pedal_receive(uint32_t st,uint32_t ch,uint32_t n,uint32_t v,uint32_t port,int accepted)
{
    uint32_t i;
    if(!(sm_patch_port==2u || port==(sm_patch_port?2u:0u))) return 0;
    for(i=0;i<4u;i++) {
        const uint8_t *m=sm_pedal[i];
        if(!m[0] || (m[3] ? ch+1u!=m[3] : !accepted))continue;
        if(((m[0]==1u && st==0xB0u && v==m[2]) || (m[0]==2u && st==0xC0u)) && n==m[1]) {
            if(sm_action_w-sm_action_r<16u){sm_actions[sm_action_w%16u]=(uint8_t)i;__asm__ volatile("" ::: "memory");sm_action_w++;}
            return 1;
        }
    }
    return 0;
}
static uint32_t sm_set(uint32_t patch_port, uint32_t key_port, uint32_t bluetooth)
{
    if (patch_port > 2u || key_port > 2u || bluetooth > 1u) return 1;
    if ((patch_port == 1u || bluetooth) && !(SM_CAPS & SM_CAP_BLE_CENTRAL)) return 4;
    sm_patch_port = (uint8_t)patch_port;
    sm_key_port = (uint8_t)key_port;
    sm_bluetooth = (uint8_t)bluetooth;
    return 0;
}
static int sm_patch_accept(uint32_t port)
{
    return sm_patch_port == 2u || port == (sm_patch_port ? 2u : 0u);
}
static int sm_key_accept(uint32_t port)
{
    return port < 2u && (sm_key_port == 2u || port == sm_key_port);
}
#endif
