/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* v12 (Clipper 0.6): the FMX engine's patches (eng_fmx.c), EDITOR_PROTOCOL.md "FMX patches".
 *   FMX_GET    target, index        -> target, index, rc, [pack7 of the 128-byte patch]
 *              target 0: the part `index` plays now (0..2), 1: factory patch `index`, 2: bank slot `index` (0..26:
 *              an FMX record as it is, a DX7 voice converted, an empty slot rc 2)
 *   FMX_PUT    0, part, pack7 (128) -> 0, part, rc     the part's patch from its next block (fmx_sanitize), until a
 *              sound loads or PTCH turns (or STORE)
 *   FMX_STORE  slot, pack7 (128)    -> slot, rc        into bank slot `slot` (0..26), marked FMX's (fm6_bank.c); the
 *              song stopped (rc 3 while it plays: the flash write silences the audio a moment)
 * rc: 0 ok, 1 bad request, 2 empty slot / flash error, 3 the song plays. Every byte of the patch 0..127. */
enum { ED_FMX_GET = 90, ED_FMX_PUT, ED_FMX_STORE };
enum { ED_FMX_TRACK, ED_FMX_FACTORY, ED_FMX_BANK };

static int ed_fmx_handle(uint32_t cmd, const uint8_t *a, uint32_t n)
{
    fmx_patch_t p;
    uint32_t rc = 0;
    switch (cmd) {
    case ED_FMX_GET:
        if (n != 2u)
            rc = 1;
        else if (a[0] == ED_FMX_TRACK && a[1] < NPART)
            p = fmx_cur[a[1]];
        else if (a[0] == ED_FMX_FACTORY && a[1] < FMX_NFACTORY)
            p = FMX_FACTORY[a[1]];
        else if (a[0] == ED_FMX_BANK && a[1] < FM6_BANK_N) {
            uint8_t pk[FM6_PACKED];
            if (!fm6_bank_read || fm6_bank_read(a[1], pk))
                rc = 2;
            else
                fmx_slot_patch(FMX_NFACTORY + a[1], &p);
        } else
            rc = 1;
        ed_b(n ? a[0] : 127u);
        ed_b(n > 1u ? a[1] : 127u);
        ed_b(rc);
        if (!rc)
            ed_pack7((const uint8_t *)&p, sizeof p);
        return 1;
    case ED_FMX_PUT:
        rc = n < 2u || a[0] != ED_FMX_TRACK || a[1] >= NPART ||
             ed_unpack7(a + 2, n - 2u, (uint8_t *)&p, sizeof p) != sizeof p;
        if (!rc) {
            fmx_sanitize(&p);
            fm1_irq_off();
            fmx_cur[a[1]] = p;
            fmx_dirty[a[1]] = 1;
            fm1_irq_on();
            ui.force = 1;
        }
        ed_b(n ? a[0] : 127u);
        ed_b(n > 1u ? a[1] : 127u);
        ed_b(rc);
        return 1;
    case ED_FMX_STORE:
        rc = n < 1u || a[0] >= FM6_BANK_N || ed_unpack7(a + 1, n - 1u, (uint8_t *)&p, sizeof p) != sizeof p;
        if (!rc && (song.playing || transport_req))
            rc = 3;
        if (!rc) {
            fmx_sanitize(&p);
            rc = !fmx_bank_store || fmx_bank_store(a[0], (const uint8_t *)&p) ? 2u : 0u;
        }
        ed_b(n ? a[0] : 127u);
        ed_b(rc);
        return 1;
    }
    return 0;
}
