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
