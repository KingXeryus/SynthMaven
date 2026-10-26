/* SPDX-License-Identifier: GPL-3.0-only */
/* Native AC791 BLE MIDI adapter. The task/GATT runtime is the separately
 * licensed fm1-base port; this file binds it to SynthMaven's audio/MIDI/store. */
#include "../vendor/fm1-base/shim/shim.h"
extern void btstack_init(void), bt_ble_init(void), lmp_set_sniff_disable(void);
extern void bt_max_pwr_set(int, int, int, int);
extern void lib_make_ble_address(uint8_t *, uint8_t *);
extern int le_controller_set_mac(void *);
extern void ble_midi_enable(int), ble_midi_poll(void), ble_midi_scan(int), ble_midi_disconnect(void);
extern int ble_midi_found(int, char *, int *), ble_midi_connect(int), ble_midi_central_state(void);
extern int ble_midi_peripheral_connected(void), ble_midi_peripheral_linked(void);
extern void ble_midi_disconnect_peripheral(void);
extern int syscfg_read(uint16_t, void *, uint16_t), syscfg_write(uint16_t, void *, uint16_t);
volatile uint32_t shim_isr_blocks; /* upstream shim counts refused waits in IRQ context */
static uint8_t sm_ble_up, sm_ble_started, sm_ble_on, sm_ble_failed, sm_scan_requested;
static uint32_t sm_ble_t0, sm_scan_t0, sm_irq_depth;
static char sm_ble_log[40];
static volatile uint32_t sm_ble_w, sm_ble_r;
static uint32_t sm_ble_q[64];
static const void *sm_cfg_pending;
static uint32_t sm_cfg_len;

uint32_t shim_ms(void) { return fm1_ms; }
uint32_t shim_irq_save(void) { fm1_irq_off(); return sm_irq_depth++; }
void shim_irq_restore(uint32_t f) { (void)f; if (sm_irq_depth && !--sm_irq_depth) fm1_irq_on(); }
int shim_in_irq(void) { return felucca_dbg.in_audio != 0; }
void shim_wdt(void) { fm1_wdt_feed(); }
void shim_log(const char *s) { str_cpy(sm_ble_log, s, sizeof sm_ble_log); }
void shim_reboot(void) { fm1_reboot(); }
#ifdef SM_BLE_HOST_TEST
#define SM_RADIO_RAM
#else
#define SM_RADIO_RAM __attribute__((section(".radio_ram"),noinline,used))
#endif
SM_RADIO_RAM void shim_delay_us(uint32_t us)
{
#ifdef SM_BLE_HOST_TEST
    (void)us;
#else
    fm1_radio_delay_us(us);
#endif
}
void shim_idle(void)
{
    static uint8_t inside;
    fm1_wdt_feed();
    if (!inside) { inside = 1; shim_timers(); usb_poll(); inside = 0; }
}
void shim_request_irq(int index, int priority, void (*handler)(void))
{
    uint32_t f = shim_irq_save();
    if (index >= 0 && index < 128) {
        fm1_guard_unlock_top();
        fm1_irq_attach((uint32_t)index, handler, (uint32_t)(priority + 5 > 7 ? 7 : priority + 5));
        fm1_guard_lock_top();
    }
    shim_irq_restore(f);
}

/* Radio syscfg journal: the previously unused 0xFB000 sector, outside every
 * sample/project/preset. Commit word written last. At rollover, power loss may
 * forget radio bonds, but cannot touch musical data or the boot/update area. */
#define SM_RADIO_BASE 0xFB000u
#define SM_RADIO_MAGIC 0x534D4254u
static int sm_radio_find(uint32_t *end, uint32_t *seq, uint32_t *len)
{
    uint32_t off = 0, best = 0, at = 0, n = 0;
    while (off + 20u <= 4096u) {
        uint32_t h[5];
        if (st_read(SM_RADIO_BASE + off, h, sizeof h) || h[0] != SM_RADIO_MAGIC || h[1] > 1536u || !h[1] ||
            h[4] != 0x434F4D54u || off + 20u + h[1] > 4096u) break;
        if (!st_read(SM_RADIO_BASE + off + 20u, st_buf, h[1]) && st_crc32(st_buf,h[1]) == h[2] && h[3] >= best)
            best = h[3], at = off + 20u, n = h[1];
        off += (20u + h[1] + 3u) & ~3u;
    }
    *end = off; *seq = best; *len = n;
    return n ? (int)at : -1;
}
int shim_store_load(void *out, uint32_t max)
{
    uint32_t end, seq, n; int at = sm_radio_find(&end,&seq,&n);
    if (at < 0 || n > max || st_read(SM_RADIO_BASE + (uint32_t)at,out,n)) return -1;
    return (int)n;
}
int shim_store_save(const void *src, uint32_t n)
{
    if (!n || n > 1536u) return -1;
    sm_cfg_pending = src; sm_cfg_len = n; return (int)n; /* main-loop flush, outside the BT task */
}
static void sm_radio_flush(void)
{
    uint32_t end, seq, n, i, h[5], bytes;
    if (!sm_cfg_pending || !flash_ok || song.playing) return;
    for (i = 0; i < NPART; i++) { uint32_t k; for(k=0;k<NVOICE;k++)if(trk[i].v[k].active)return; }
    sm_radio_find(&end,&seq,&n); bytes = (20u+sm_cfg_len+3u)&~3u;
    /* A failed or interrupted append is never programmed over: erase instead. */
    {uint32_t word=0; if(end+bytes>4096u || st_read(SM_RADIO_BASE+end,&word,4u) || word!=0xFFFFFFFFu) {
        if(st_erase(SM_RADIO_BASE))return; end=0;
    }}
    h[0]=SM_RADIO_MAGIC;h[1]=sm_cfg_len;h[2]=st_crc32(sm_cfg_pending,sm_cfg_len);h[3]=seq+1u;h[4]=0x434F4D54u;
    if(st_prog(SM_RADIO_BASE+end,h,16u) || st_prog(SM_RADIO_BASE+end+20u,sm_cfg_pending,sm_cfg_len) ||
       st_prog(SM_RADIO_BASE+end+16u,h+4,4u))return;
    sm_cfg_pending=0;
}
const uint8_t *bt_get_mac_addr(void)
{
    static uint8_t mac[6]; uint32_t h=2166136261u,i;
    if(syscfg_read(1,mac,6)==6)return mac;
    for(i=0;i<6;i++){h=(h^fm1_ticks())*16777619u;mac[i]=(uint8_t)(h>>8);}
    mac[0]=(uint8_t)((mac[0]&0xFCu)|0xC0u);syscfg_write(1,mac,6);return mac;
}
void ble_midi_receive(uint32_t packet, uint32_t timestamp)
{
    (void)timestamp;
    if(sm_ble_on && sm_ble_w-sm_ble_r<64u) {
        sm_ble_q[sm_ble_w%64u]=(packet&~0xF0u)|0x20u;RING_PUBLISH();sm_ble_w++;
    }
}
static int sm_ble_pending(void) { return sm_ble_w != sm_ble_r; }
static int sm_ble_read(uint32_t *packet)
{
    if(sm_ble_r==sm_ble_w)return 0;
    *packet=sm_ble_q[sm_ble_r%64u];RING_PUBLISH();sm_ble_r++;return 1;
}
struct sm_bt_event {uint8_t event,args[7];uint32_t value;};
void shim_bt_event(int from,const void *event,int len)
{
    const struct sm_bt_event *e=event;
    /* SDK bt_event_notify(from,event) has no length argument. The bridge
     * forwards len=0; event is a typed SDK object, not a received byte buffer. */
    (void)len;
    if(from==3 && e && e->event==3 && !sm_ble_up) {
        bt_ble_init();sm_ble_up=1;sm_ble_failed=0;sm_radio_quiet=0;
    }
}
static uint32_t sm_ble_state(void)
{
    if(!sm_bluetooth)return 0;
    if(sm_ble_failed)return 7;
    if(!sm_ble_up)return 1;
    if(ble_midi_peripheral_connected())return 6;
    if(ble_midi_peripheral_linked() && !ble_midi_central_state())return 5;
    return 2u+(uint32_t)ble_midi_central_state(); /* idle, scan, connect, discover, ready */
}
static uint32_t sm_ble_scan_start(void)
{
    if(!sm_bluetooth || sm_ble_failed)return 4;
    sm_scan_requested=1;return 0;
}
static int sm_ble_device(uint32_t i,char *name,int *rssi)
{
    return sm_ble_up && i<8u ? ble_midi_found((int)i,name,rssi):0;
}
static uint32_t sm_ble_connect_device(uint32_t i)
{
    return sm_ble_up && sm_bluetooth && i<8u && !ble_midi_connect((int)i) ? 0u:4u;
}
static void sm_ble_disconnect_device(void) { if(sm_ble_up){ble_midi_disconnect();ble_midi_disconnect_peripheral();} }
static void sm_ble_poll(void)
{
    if(sm_bluetooth && !sm_ble_started) {
        uint8_t address[6];sm_ble_started=1;sm_radio_quiet=1;sm_ble_t0=shim_ms();
        shim_os_init();fm1_guard_stack_floor(shim_stacks_lo());
        lmp_set_sniff_disable();bt_max_pwr_set(8,6,6,6);
        lib_make_ble_address(address,(uint8_t *)bt_get_mac_addr());le_controller_set_mac(address);btstack_init();
    }
    if(!sm_ble_started)return;
    shim_timers();shim_run();
    if(sm_ble_up) {
        if(sm_ble_on!=sm_bluetooth){ble_midi_enable(sm_bluetooth);sm_ble_on=sm_bluetooth;if(!sm_ble_on){sm_ble_r=sm_ble_w;sm_scan_requested=0;}}
        ble_midi_poll();
        if(sm_ble_on && sm_scan_requested){ble_midi_scan(1);sm_scan_t0=shim_ms();sm_scan_requested=0;}
        if(ble_midi_central_state()==1 && shim_ms()-sm_scan_t0>15000u)ble_midi_scan(0);
    }
    if(!sm_ble_up && shim_ms()-sm_ble_t0>15000u){sm_ble_failed=1;sm_radio_quiet=0;}
    sm_radio_flush();
    {static uint32_t seen;uint32_t state=sm_ble_state();if(state!=seen){seen=state;ui.force=1;if(state==6u)ui_message("BLE MIDI READY");else if(state==7u)ui_message("BT START TIMEOUT");}}
}
