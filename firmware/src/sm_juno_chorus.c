/* SPDX-License-Identifier: GPL-3.0-only
 * JUNO chorus retained from SynthMaven/Clipper fx.c; fixed per-part state.
 * No allocation or shared render state on the second core. */
static uint32_t smj_tri(uint32_t ph){uint32_t t=ph>>16;return t<32768?t*2:(65535-t)*2;}
static int32_t smj_lpk(int32_t c){int32_t g=SVF_G[clamp(c,0,127)];return (g<<16)/(4096+g);}
#define JC_LEN 256u                     /* the line: 5.8 ms (a power of 2: masked), at a quarter (headroom) */
typedef struct {
    int32_t w;                          /* the wet now, Q15 */
    uint32_t ph, nz;                    /* the LFO's phase; the hiss */
    int32_t l1, l2;                     /* each tap's low-pass */
    uint16_t wp;                        /* the line's next write */
    uint8_t mode;                       /* running: 1 I, 2 II, 3 I+II */
    uint16_t fill;                      /* samples left before it fades in (the line full of the sound first) */
    uint8_t idle;                       /* the part was silent (mix_part1 did not run it): a new mode at once */
    int16_t line[JC_LEN];
} jc_t;
static jc_t jc[NPART];
static int32_t jc_side[CTL];
static const uint32_t JC_INC[3] = {49962u, 84048u, 949577u};   /* 0.513, 0.863, 9.75 Hz at 44.1 kHz */
static int32_t jc_mode(const track_t *t,int32_t *wet,int32_t *width,int32_t *tone,int32_t *hiss){
 *wet=32768;*width=127;*tone=100;*hiss=0;return t->engine==ENGI_SM_JUNO?clamp(t->p[P_E7],0,3):0;
}
/* the part silent: a chorus whose mode is no longer asked for stops at once (nothing to fade: the next note dry) */
static void jc_idle(const track_t *t)
{
    jc_t *c = &jc[(uint32_t)(t - trk) % NPART];
    int32_t a, b, d, e;
    c->idle = 1;
    if (c->mode && jc_mode(t, &a, &b, &d, &e) != c->mode)
        c->w = 0, c->mode = 0;
}
/* the chorus on the part's block b (mono in, its mid back in b, its side in jc_side): 1 when it ran */
static __attribute__((noinline)) int juno_chorus(track_t *t, int32_t *b, uint32_t n)
{
    jc_t *c = &jc[(uint32_t)(t - trk) % NPART];
    int32_t wet, width, tone, hiss, m = jc_mode(t, &wet, &width, &tone, &hiss), tw, w, l1, l2, k;
    uint32_t i, ph, wp, inc, base, span;
    int16_t *line = c->line;
    if (c->mode != m && (!c->w || c->idle)) {           /* faded out (or silent): the new mode from rest */
        memset(c, 0, sizeof *c);
        c->mode = (uint8_t)m;
        c->nz = 0x2545F491u;
        c->fill = JC_LEN;                               /* (an empty line read back: a step when the sound reaches it) */
    }
    c->idle = 0;
    if (!c->mode)
        return 0;
    tw = c->mode == m ? wet : 0;
    k = smj_lpk(tone) >> 1;                             /* Q15 */
    inc = JC_INC[c->mode - 1u];
    if (c->mode == 3u)
        base = 9535488u, span = 18u;                    /* 145.5 + 0 .. 17.7 samples (Q16) */
    else
        base = 4797235u, span = 163u;                   /* 73.2 + 0 .. 163 samples */
    w = c->w, ph = c->ph, wp = c->wp, l1 = c->l1, l2 = c->l2;
    width = width * 258;
    hiss = hiss * hiss / 96;                          /* (a gentle curve: 96 a faint hiss, 127 the BBD's) */
    for (i = 0; i < n; i++, ph += inc) {
        int32_t x = b[i], tr = (int32_t)smj_tri(ph), v1, v2, r0, fr, g, md, sd;
        uint32_t d1 = base + span * (uint32_t)tr, d2 = base + span * (uint32_t)(65535 - tr);
        if (c->fill)
            c->fill--;
        else
            w += clamp(tw - w, -256, 256);
        r0 = (int32_t)(wp - (d1 >> 16)) & (int32_t)(JC_LEN - 1u), fr = (int32_t)((d1 >> 1) & 0x7FFFu);
        v1 = line[r0] + (((line[(r0 - 1) & (int32_t)(JC_LEN - 1u)] - line[r0]) * fr) >> 15);
        r0 = (int32_t)(wp - (d2 >> 16)) & (int32_t)(JC_LEN - 1u), fr = (int32_t)((d2 >> 1) & 0x7FFFu);
        v2 = line[r0] + (((line[(r0 - 1) & (int32_t)(JC_LEN - 1u)] - line[r0]) * fr) >> 15);
        l1 += ((v1 - l1) * k) >> 15;                    /* the BBD's dark */
        l2 += ((v2 - l2) * k) >> 15;
        {
            int32_t in = clamp(x >> 3, -32767, 32767);  /* (1.0: an eighth, was a quarter: 8 loud voices clipped) */
            if (hiss) {
                c->nz = c->nz * 1664525u + 1013904223u;
                in += (((int32_t)(c->nz >> 16) - 32768) * hiss) >> 16;
            }
            line[wp] = (int16_t)clamp(in, -32767, 32767);
        }
        wp = (wp + 1u) & (JC_LEN - 1u);
        g = 32768 - ((w * 9830) >> 15);                 /* 1 dry .. 0.7 at full wet */
        /* (1.0: 64-bit products: a loud chord's x passes 2^16, and x * g wrapped round: a crack at a chord change) */
        md = x + (int32_t)(((int64_t)((l1 + l2) << 2) * w) >> 15);   /* dry + (tap 1 + tap 2) / 2 (the taps at an eighth) */
        sd = (int32_t)(((int64_t)(((int64_t)((l1 - l2) << 2) * w) >> 15) * width) >> 15);   /* (tap 1 - tap 2) / 2 */
        b[i] = (int32_t)(((int64_t)md * g) >> 15);
        jc_side[i] = (int32_t)(((int64_t)sd * g) >> 15);
    }
    c->w = w, c->ph = ph, c->wp = (uint16_t)wp, c->l1 = l1, c->l2 = l2;
    if (!w && c->mode != m)
        c->mode = 0;                                    /* (faded out: the next block takes the new one) */
    return 1;
}
