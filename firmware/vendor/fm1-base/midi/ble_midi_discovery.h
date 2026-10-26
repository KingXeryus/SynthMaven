/* SPDX-License-Identifier: MPL-2.0 AND Apache-2.0
 * Copyright (C) 2026 Charles Vestal (from fm1-ble and fm1-ctl)
 * SynthMaven adaptation: named Chocolate candidates with GATT MIDI validation.
 * Kept separate so the real advertisement parser runs in host regression tests. */
#define MC_MAX 8
typedef struct { char name[21]; u8 addr[6], addr_type; s8 rssi; } mc_dev_t;
static mc_dev_t mc_dev[MC_MAX];
static volatile u8 mc_n;
static int uuid_is(const u8 *u, const u8 *le)    /* a 128-bit uuid as the stack reports it, either order */
{
    int i, fwd = 1, rev = 1;
    for (i = 0; i < 16; i++) {
        fwd &= u[i] == le[i];
        rev &= u[i] == le[15 - i];
    }
    return fwd || rev;
}

/* FootCtrlPlus was observed advertising only HID 0x1812, so MIDI support must be
 * checked over GATT. Admit known pedal names as candidates, then retain the
 * existing characteristic check before declaring a MIDI-ready link. */
static int mc_pedal_name(const u8 *name, u8 len)
{
    static const char *const prefixes[]={"footctrl","chocolate"};
    u8 p,i;
    if(!name)return 0;
    for(p=0;p<2;p++) {
        for(i=0;prefixes[p][i] && i<len;i++)
            if((name[i]|32u)!=(u8)prefixes[p][i])break;
        if(!prefixes[p][i])return 1;
    }
    return 0;
}

/* an advertising report: a device advertising the MIDI service goes in the list (its name from this report or
 * its scan response; the strongest reading kept) */
static void mc_adv(const adv_report_t *r)
{
    const u8 *d = r->data;
    u8 len = r->length, i = 0, midi = 0, nl = 0, k;
    const u8 *name = 0;
    while (i + 1u < len) {
        u8 l = d[i], t = d[i + 1];
        if (!l || i + 1u + l > len)
            break;
        if ((t == 0x06 || t == 0x07) && l >= 17)                 /* 128-bit service uuids */
            for (k = 0; k + 16u <= (u8)(l - 1); k += 16)
                midi |= (u8)uuid_is(&d[i + 2 + k], midi_svc_le);
        if (t == 0x08 || t == 0x09) {                            /* the name */
            name = &d[i + 2];
            nl = (u8)(l - 1);
        }
        i = (u8)(i + 1 + l);
    }
    for (k = 0; k < mc_n; k++)
        if (mc_dev[k].addr_type == r->address_type && !memcmp(mc_dev[k].addr, r->address, 6))
            break;
    if (k == mc_n) {
        if ((!midi && !mc_pedal_name(name,nl)) || mc_n >= MC_MAX)
            return;                                              /* unknown HID devices stay out */
        memset(&mc_dev[k], 0, sizeof mc_dev[k]);
        memcpy(mc_dev[k].addr, r->address, 6);
        mc_dev[k].addr_type = r->address_type;
        mc_dev[k].rssi = r->rssi;
        mc_n++;
    }
    if (name) {
        if (nl > 20)
            nl = 20;
        memcpy(mc_dev[k].name, name, nl);
        mc_dev[k].name[nl] = 0;
    }
    if (r->rssi > mc_dev[k].rssi)
        mc_dev[k].rssi = r->rssi;
}

