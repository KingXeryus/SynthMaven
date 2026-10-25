/* SPDX-License-Identifier: GPL-3.0-only */
/* Lossless factory spectra: per-table DEFLATE of signed frame deltas as LEB128.
 * Both buffers are in the WAVE mipmap while wt_have == -1; no permanent RAM. */
int tinf_uncompress(void *dest, unsigned int *dest_len, const void *source, unsigned int source_len);
static int wt_rom_decode(uint32_t table, uint8_t *scratch, int16_t *out)
{
    unsigned int n = 8192u;
    uint32_t i, pos = 0;
    if (table >= WT_NFACTORY || tinf_uncompress(scratch, &n, WT_PACK_DATA + WT_PACK_OFF[table],
        WT_PACK_OFF[table + 1u] - WT_PACK_OFF[table]) || n != WT_PACK_LEN[table])
        return 0;
    for (i = 0; i < WT_NF * WT_NH * 2u; i++) {
        uint32_t v = 0, sh = 0, b;
        int32_t delta, value;
        do {
            if (pos >= n || sh > 14u) return 0;
            b = scratch[pos++]; v |= (b & 127u) << sh; sh += 7u;
        } while (b & 128u);
        delta = (v & 1u) ? -(int32_t)(v >> 1) - 1 : (int32_t)(v >> 1);
        value = delta + (i >= WT_NH * 2u ? out[i - WT_NH * 2u] : 0);
        if (value < -32767 || value > 32767) return 0;
        out[i] = (int16_t)value;
    }
    return pos == n;
}
