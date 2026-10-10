/* SPDX-License-Identifier: GPL-3.0-only */
/* The screen font (gfx.c cv_text, tools/gen_font.py): every glyph of FONT_S (1x, Latin-1) and FONT_L (2x) drawn in
 * several colours, at several offsets (clipped at every edge of the canvas, and with a graph y offset), hashed.
 * The firmware stores the Terminus bitmaps at 1 bit a pixel (5376 bytes instead of 21504): the expected hashes are
 * those of the 4-bit renderer before, so the screen shows exactly the same pixels. Also checked: the advance and
 * bitmap widths (text_w, the layout), and the data size. Exit status: the number of failed checks. */
#include <stdint.h>
#define FONT_S_HASH 0xca4fe04eu         /* the renderer before (4-bit data) */
#define FONT_L_HASH 0xd0230e39u
#define FONT_W_HASH 0x6840d800u
#include <stdio.h>
#include <string.h>
static void lcd_sync(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p) { (void)x; (void)y; (void)w; (void)h; (void)p; }
#include "../firmware/src/gfx.c"
static int bad;
static void check(int ok, const char *what) { printf("font: %-70s %s\n", what, ok ? "ok" : "FAIL"); bad += !ok; }
static uint32_t hash_cv(uint32_t h)
{
    uint32_t i;
    for (i = 0; i < cv_w * cv_h; i++)
        h = (h ^ cv_px[i]) * 16777619u;
    return h;
}
static uint32_t render_all(const felucca_font_t *f, uint32_t ch_hi)
{
    static const uint16_t COL[4] = {0xFFFFu, 0x07E0u, 0xF81Fu, 0x39E7u};
    static const int32_t XY[6][3] = {{0, 0, 0}, {5, 3, 0}, {-3, 0, 0}, {50, -5, 0}, {52, 30, 0}, {10, 4, -9}};
    uint32_t h = 0x811C9DC5u, c, k, i;
    char s[2] = {0, 0};
    for (c = f->first; c <= ch_hi; c++)
        for (k = 0; k < 4u; k++)
            for (i = 0; i < 6u; i++) {
                cv_begin(64, 48, 0);
                cv_oy = XY[i][2];
                s[0] = (char)c;
                cv_text(XY[i][0], XY[i][1], f, s, COL[k]);
                cv_oy = 0;
                h = hash_cv(h);
            }
    cv_begin(240, 40, 0);                                   /* a line of text, as the pages draw it */
    cv_text(2, 2, &FONT_S, "SLOOP 2.5 HUGELTON BPM 96 -12.5 dB", 0xFFFFu);
    cv_text(2, 18, &FONT_S, "\xdc\xe9\xb0\xb1 kick snare", 0x07E0u);
    return hash_cv(h);
}
int main(int argc, char **argv)
{
    uint32_t hs = render_all(&FONT_S, 255u), hl = render_all(&FONT_L, 95u), wsum = 0, c;
    for (c = 0; c < 224u; c++) wsum = wsum * 31u + FONT_S.adv[c] * 7u + FONT_S.bw[c];
    for (c = 0; c < 64u; c++) wsum = wsum * 31u + FONT_L.adv[c] * 7u + FONT_L.bw[c];
    if (argc > 1) {
        printf("%08x %08x %08x %u\n", hs, hl, wsum, (unsigned)sizeof FONT_S_DATA);
        return 0;
    }
    check(hs == FONT_S_HASH, "FONT_S: every glyph, 4 colours, 6 places (clipped): the same pixels as before");
    check(hl == FONT_L_HASH, "FONT_L (2x): every glyph, 4 colours, 6 places (clipped): the same pixels as before");
    check(wsum == FONT_W_HASH, "advance and bitmap widths unchanged (the layout)");
    {   /* the 4-bit path (anti-aliased fonts) still draws: FONT_S unpacked to 2 pixels a byte, the same pixels */
        static uint8_t d4[21504];
        static uint16_t off4[224];
        felucca_font_t s4 = FONT_S, l4 = FONT_L;
        uint32_t g, n = 0, y, xx;
        for (g = 0; g < 224u; g++) {
            uint32_t w = FONT_S.bw[g], bpr = (w + 1u) / 2u;
            off4[g] = (uint16_t)n;
            for (y = 0; y < FONT_S.h; y++)
                for (xx = 0; xx < w; xx++) {
                    uint32_t i = y * w + xx, on = (FONT_S_DATA[FONT_S_OFF[g] + i / 8u] >> (7u - i % 8u)) & 1u;
                    if (on)
                        d4[n + y * bpr + xx / 2u] |= (uint8_t)(xx & 1u ? 15u : 15u << 4);
                }
            n += FONT_S.h * bpr;
        }
        s4.data = l4.data = d4; s4.off = l4.off = off4; s4.bits = l4.bits = 0;
        check(n == 21504u && render_all(&s4, 255u) == FONT_S_HASH && render_all(&l4, 95u) == FONT_L_HASH,
              "the 4-bit format (anti-aliased fonts) still draws the same pixels");
    }
    check(sizeof FONT_S_DATA == 5376u && FONT_S.bits == 1u, "the Terminus bitmaps at 1 bit a pixel: 5376 bytes (21504 at 4 bits)");
    printf("%s\n", bad ? "FONT TEST FAILED" : "font test passed");
    return bad;
}
