/* SPDX-License-Identifier: GPL-3.0-only */
/* Experimental editor-only BLE peripheral. No task scheduler, central scanning,
 * pedal routing or audio transport. Radio IRQs have lower priority than audio. */
#define BLE_DIAG 0
#define BLE_CONN_TIMEOUT 200u
#define BLE_DEVICE_NAME "SynthMaven FM1"
#define BLE_CONN_MIN 24u
#define BLE_CONN_MAX 36u
#define BLE_CONN_RETRY_MIN 24u
#define BLE_CONN_RETRY_MAX 36u
#include "../vendor/fm1-ble-own/ble/ble_stack.c"
static uint8_t sm_ble_started,sm_ble_on,sm_ble_last_state;
#include "synthmaven_ble_editor.h"
/* Prepare RF before starting CPU1/audio. The AC79 secondary core cannot be
 * reliably cold-started again after runtime RF calibration. Advertising stays
 * disabled until the user explicitly enables BLE EDITOR. */
static void sm_ble_prepare(void){
        fm1_ble_crumb_boot();
        uint32_t record[4];uint8_t *addr=(uint8_t *)(record+1);
        if(st_read(0xFB000u,record,sizeof record)||record[0]!=0x33454C42u||record[3]!=st_crc32(record,12)){
            record[0]=0x33454C42u;record[1]=fm1_ticks()*2654435761u;record[2]=fm1_ticks()*2246822519u;addr[5]|=0xC0u;record[3]=st_crc32(record,12);
            if(!st_erase(0xFB000u))st_prog(0xFB000u,record,sizeof record);
        }
        static const struct ble_rf_trims trims={{0x0B,0x0B},{1,7,4,7,11,1,7},{0},{0}};
    fm1_wdt_feed();ble_init(addr,1);
    ble_hw_wl82_attach();fm1_rf_cal_on=1;ble_hw_wl82_start(&trims);
    fm1_wdt_feed();sm_ble_started=1;
}
static void sm_ble_poll(void){
    if(!sm_ble_started)return;
    uint8_t state=(uint8_t)sm_ble_state();if(state!=sm_ble_last_state){sm_ble_last_state=state;ui.force=1;}
    if(sm_ble_on!=sm_bluetooth){fm1_ble_irqs_hold(1);ble_enable(sm_bluetooth);sm_ble_on=sm_bluetooth;fm1_ble_irqs_hold(0);ui.force=1;}
    if((sm_rx_open && fm1_ms-sm_rx_at>3000u)||(sm_ble_editor_busy() && fm1_ms-sm_tx_at>10000u)){
        fm1_ble_irqs_hold(1);sm_rx_open=sm_rx_ready=0;sm_rx_n=sm_tx_n=sm_tx_pos=0;ble_enable(0);sm_bluetooth=sm_ble_on=0;fm1_ble_irqs_hold(0);ui_message("BLE LINK TIMEOUT");
    }
}

static void sm_ble_diagnostics(uint32_t *d){
 const uint32_t *c=(const uint32_t *)&fm1_crash;for(uint32_t i=0;i<11;i++)d[i]=c[i];
 d[11]=fm1_ble_bc.now;d[12]=fm1_crash.axi_write_id;d[13]=fm1_crash.axi_read_id;d[14]=fm1_ble_bc.addr;
 d[15]=fm1_ble_bc.prev;d[16]=fm1_ble_bc.prev_irqs;d[17]=fm1_ble_bc.prev_stop;d[18]=fm1_ble_bc.prev_addr;d[19]=fm1_boot.p3_rst;d[20]=fm1_boot.rst_src;for(uint32_t i=0;i<16;i++)d[21+i]=fm1_crash.registers[i];for(uint32_t i=0;i<4;i++)d[37+i]=fm1_crash.etm[i];d[41]=cpu1_stack_free();
}
