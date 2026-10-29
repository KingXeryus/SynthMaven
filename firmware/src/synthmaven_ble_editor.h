/* SPDX-License-Identifier: GPL-3.0-only */
/* Shared bounded editor transport; callbacks run only at the radio IRQ priority. */
static volatile uint8_t sm_rx_ready,sm_rx_open;
static volatile uint32_t sm_rx_n,sm_rx_at,sm_tx_n,sm_tx_pos,sm_tx_at;
#ifdef SM_BLE_HOST_TEST
static uint8_t sm_rx[2048];
#else
static uint8_t sm_rx[2048] __attribute__((section(".pool")));
#endif
static const uint8_t *sm_tx;
BLE_API int ble_app_midi_peek(uint32_t *pkt,uint32_t *ms){
    uint32_t pos=sm_tx_pos,n=sm_tx_n;if(pos>=n)return 0;
    uint32_t count=n-pos;if(count>3)count=3;
    uint32_t cin=pos+count==n?4u+count:4u,p=cin;
    for(uint32_t i=0;i<count;i++)p|=(uint32_t)sm_tx[pos+i]<<(8u*(i+1u));
    *pkt=p;*ms=fm1_ms;return 1;
}
BLE_API void ble_app_midi_pop(void){uint32_t left=sm_tx_n-sm_tx_pos;sm_tx_pos+=left>3?3:left;}
BLE_API void ble_app_midi_in(uint32_t pkt,uint16_t ts,uint16_t last){
    (void)ts;(void)last;uint32_t cin=pkt&15u;
    if(cin<4u||cin>7u||sm_rx_ready)return;
    uint32_t count=cin==4u?3u:cin-4u;
    for(uint32_t i=0;i<count;i++){
        uint8_t b=(uint8_t)(pkt>>(8u*(i+1u)));
        if(b==0xF0){sm_rx_n=0;sm_rx_open=1;sm_rx_at=fm1_ms;continue;}
        if(!sm_rx_open)continue;
        if(b==0xF7){sm_rx_open=0;if(sm_rx_n>=4u && sm_rx[0]==125u && sm_rx[1]==70u && sm_rx[2]==76u){RING_PUBLISH();sm_rx_ready=1;}return;}
        if(b&128u || sm_rx_n>=sizeof sm_rx){sm_rx_open=0;sm_rx_n=0;return;}
        sm_rx[sm_rx_n++]=b;
    }
}
BLE_API void ble_app_state(void){
    if(!ble_connected()){sm_rx_ready=sm_rx_open=0;sm_rx_n=sm_tx_n=sm_tx_pos=0;}
}
static int sm_ble_pending(void){return 0;}
static int sm_ble_read(uint32_t *p){(void)p;return 0;}
static uint32_t sm_ble_state(void){return !sm_bluetooth?0u:!sm_ble_started?1u:ble_midi_ready()?6u:ble_connected()?5u:2u;}
static uint32_t sm_ble_scan_start(void){return 4;}
static uint32_t sm_ble_connect_device(uint32_t i){(void)i;return 4;}
static int sm_ble_device(uint32_t i,char *n,int *r){(void)i;(void)n;(void)r;return 0;}
static void sm_ble_disconnect_device(void){sm_bluetooth=0;}
static int sm_ble_editor_busy(void){return sm_tx_pos<sm_tx_n;}
static int sm_ble_editor_frame(const uint8_t **p,uint32_t *n){if(!sm_rx_ready)return 0;*p=sm_rx;*n=sm_rx_n;return 1;}
static void sm_ble_editor_done(void){sm_rx_n=0;RING_PUBLISH();sm_rx_ready=0;}
static void sm_ble_editor_send(const uint8_t *p,uint32_t n){
    if(!sm_ble_on||!ble_midi_ready())return;
    fm1_ble_irqs_hold(1);sm_tx=p;sm_tx_pos=0;sm_tx_at=fm1_ms;RING_PUBLISH();sm_tx_n=n;fm1_ble_irqs_hold(0);
}
