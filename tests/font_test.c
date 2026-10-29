/* SPDX-License-Identifier: GPL-3.0-only */
/* Verify Rubik glyph bounds and lossless run-length rendering against the
 * uncompressed alpha path, including offscreen clipping. */
#include <stdint.h>
#define FONT_S_HASH 0xca4fe04eu         /* the renderer before (4-bit data) */
#define FONT_L_HASH 0xd0230e39u
#define FONT_W_HASH 0x6840d800u
#include <stdio.h>
#include <string.h>
static void lcd_sync(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p) { (void)x; (void)y; (void)w; (void)h; (void)p; }
#ifdef __APPLE__
#define __attribute__(x)
#endif
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
    check(FONT_S.bits==2 && FONT_L.bits==2, "Rubik uses lossless run-length alpha storage");
    check(FONT_S.h<=20 && FONT_L.h<=36, "font metrics fit the existing header and value regions");
    check(sizeof FONT_S_DATA+sizeof FONT_L_DATA<26000, "both anti-aliased faces fit the firmware font budget");
    const felucca_font_t *faces[]={&FONT_S,&FONT_L};
    for(uint32_t f=0;f<2;f++) {
        const felucca_font_t *face=faces[f];uint32_t count=face->last-face->first+1;
        for(uint32_t g=0;g<count;g++) {
            static uint8_t raw[4096];static uint16_t off[224];uint32_t total=face->bw[g]*face->h,pos=0,at=face->off[g],limit=f?sizeof FONT_L_DATA:sizeof FONT_S_DATA;
            /* Shared glyphs need not have monotonic offsets. The next distinct
             * stored glyph bounds the stream, not the next character. */
            for(uint32_t k=0;k<count;k++)if(face->off[k]>at && face->off[k]<limit)limit=face->off[k];
            while(pos<total && at<limit){uint32_t code=face->data[at++],run=(code>>4)+1;for(uint32_t j=0;j<run && pos<total;j++)raw[pos++]=code&15;}
            if(pos!=total || at!=limit){printf("decode f%u g%u pos%u/%u at%u/%u\n",f,g,pos,total,at,limit);bad++;}
            /* Decode independently into the legacy nibble renderer, then compare
             * actual colored pixels including clipping and the graph offset. */
            static uint8_t unpacked[4096];memset(unpacked,0,sizeof unpacked);uint32_t w=face->bw[g],bpr=(w+1)/2;
            for(uint32_t y=0;y<face->h;y++)for(uint32_t x=0;x<w;x++)unpacked[y*bpr+x/2]|=raw[y*w+x]<<(x&1?0:4);
            if(glyph(face,face->first+g)!=g)continue;
            off[g]=0;felucca_font_t plain=*face;plain.bits=0;plain.data=unpacked;plain.off=off;char text[2]={(char)(face->first+g),0};
            cv_begin(64,48,0);cv_oy=-3;cv_text(-2,1,face,text,0xF81F);uint32_t expected=hash_cv(0);
            cv_begin(64,48,0);cv_oy=-3;cv_text(-2,1,&plain,text,0xF81F);if(hash_cv(0)!=expected){printf("raster f%u g%u\n",f,g);bad++;}
        }
    }
    check(!bad, "every compressed glyph matches legacy alpha rendering pixel for pixel");
    return bad?1:0;
}
