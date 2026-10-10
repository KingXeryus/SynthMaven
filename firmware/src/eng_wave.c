/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* WAVE: wavetables (Clipper 0.2). A table is 16 frames, each a single cycle; POS sweeps through them, the frames
 * either side crossfaded (a smooth morph, PPG / Serum style). The ENV and the LFO move POS through the part's SHP
 * destinations (ENV DEST SHP, LFO DEST SHP), as they move ANALOG's pulse width.
 *
 * No aliasing: the tables are stored as spectra (felucca_wavetables.h: harmonics 1..128, cosine and sine terms;
 * tools/gen_wavetables.py), and the main loop (wt_service) builds band-limited copies of the selected one in RAM,
 * one per octave: 1024 points holding 128 harmonics, 512 / 64, ... 16 / 2 (eight points a harmonic: the linear
 * interpolation's images stay ~50 dB down). A voice reads the copy whose top harmonic stays under 0.45 x 44.1 kHz at its pitch.
 * While a table is being built (a few ms after TABL turns) the voices are silent, their phases running on.
 *
 * EDIT 1  TABL  the table (16 factory ones; USR1..USR4: a wavetable sent to a user slot from the editor)
 *         POS   the position in it (+ the SHP modulation)
 *         DTN   a second oscillator of the table, DTN cents up (0: one oscillator)
 *         SUB   a sine an octave down
 * EDIT 2  CUT RES DRV KTR: ANALOG's filter (FX > FILTER > MODL: SVF or the MOOG ladder) and drive.
 * Voice state: ph[0..2] (osc, the detuned one, the sub), s[0..1] the SVF, s[3..6] the ladder, s[7] the last
 * POS (ramped over a block). In the Clipper synth only (the RAM: the groovebox has its punch-in ring there). */
#include "felucca_wavetables.h"

#define WT_LEVELS 7u
#define WT_PTS (1024u + 512u + 256u + 128u + 64u + 32u + 16u)   /* a frame's band-limited copies, 2032 points */
static const uint16_t WT_OFF[WT_LEVELS] = {0, 1024, 1536, 1792, 1920, 1984, 2016};
static int16_t wt_mip[WT_NF][WT_PTS] __attribute__((section(".pool")));   /* 64 KB */
static volatile int16_t wt_have = -1;            /* the table in wt_mip, complete (-1: none, or being built) */
static int16_t wt_building = -1;                 /* the table being built, frame by frame */
static uint8_t wt_next;                          /* its next frame */
#define WT_NALL (WT_NFACTORY + SMP_USER_SLOTS)   /* + USR1..USR4 */
static const char *const WT_ALL_NAMES[WT_NALL] = {
    "BASIC", "PWM", "HARMON", "ODD", "VOCAL", "RESO", "SYNC", "FM", "FM 2:3", "FOLD", "PD", "DIGITL", "BELL",
    "ORGAN", "CHIP", "COMB", "USR1", "USR2", "USR3", "USR4"};
_Static_assert(WT_NFACTORY == 16, "WT_ALL_NAMES follows gen_wavetables.py");

/* a user slot holding a wavetable (eng_sample.c's slots, sent as a sample is: SMP_BEGIN, SMP_WRITE of the data at
 * +512, SMP_END with this header): the same spectra as the factory tables, 16 x 128 x 2 Q15 at +SMP_USER_DATA */
#define WT_USER_MAGIC 0x42545746u                /* "FWTB" */
#define WT_USER_LEN ((uint32_t)(WT_NF * WT_NH * 2u * sizeof(int16_t)))   /* 8192 B */
typedef struct {
    uint32_t magic;
    uint16_t version;            /* 1 */
    uint8_t nf, nh;              /* WT_NF, WT_NH */
    char name[8];
    uint32_t data_len, crc, rsv2[2];   /* WT_USER_LEN, CRC-32 of the data (as a sample's) */
} wt_user_hdr_t;
typedef struct {
    int16_t spec[WT_NF][WT_NH][2];
} wt_user_t;
static uint8_t wt_user_ok[SMP_USER_SLOTS];       /* the slot holds a wavetable (wt_user_scan: boot, upload) */
static const wt_user_t *wt_user(uint32_t k)
{
    k %= SMP_USER_SLOTS;
    return wt_user_ok[k] ? (const wt_user_t *)(SMP_USER_XIP(k) + SMP_USER_DATA) : 0;
}
static int wt_user_hdr_ok(const wt_user_hdr_t *h)
{
    return h->magic == WT_USER_MAGIC && h->version == 1 && h->nf == WT_NF && h->nh == WT_NH && h->data_len == WT_USER_LEN;
}
/* smp_user_scan (eng_sample.c), the main loop: boot, an upload's end: is slot k a wavetable; a table built from
 * it is built again */
static void wt_user_scan(uint32_t k)
{
    wt_user_ok[k] = (uint8_t)wt_user_hdr_ok((const wt_user_hdr_t *)SMP_USER_XIP(k));
    if (wt_have == (int16_t)(WT_NFACTORY + k) || wt_building == (int16_t)(WT_NFACTORY + k)) {
        wt_have = -1;
        wt_building = -1;
    }
}
static const int16_t (*wt_spec(uint32_t table, uint32_t f))[2]
{
    if (table < WT_NFACTORY)
        return WT_SPEC[table][f];
    {
        const wt_user_t *u = wt_user(table - WT_NFACTORY);
        return u ? u->spec[f] : 0;
    }
}

/* one frame of the table: its seven copies (additive: x[n] = sum a_h cos + b_h sin, the 1024-point sine at exact
 * points: 1024 / N is whole). A missing user table builds silence. ~4 ms a frame on the FM-1 (an estimate) */
static void wt_build_frame(uint32_t table, uint32_t f)
{
    const int16_t (*sp)[2] = wt_spec(table, f);
    uint32_t l;
    for (l = 0; l < WT_LEVELS; l++) {
        uint32_t n, h, N = 1024u >> l, H = 128u >> l, step = 1u << l;
        int16_t *d = &wt_mip[f][WT_OFF[l]];
        int32_t tap[128];                                        /* the taper: 1 up to 0.6 H, then a raised cosine
                                                                  * to 0 at H: the top partials (whose images the
                                                                  * cubic cannot hide) fade, and a copy's switch to
                                                                  * the next octave's is smooth */
        for (h = 0; h < H; h++) {
            uint32_t h0 = (H * 3u) / 5u;
            tap[h] = h + 1u <= h0 ? 32767 : (32767 + SINE[(256u + (h + 1u - h0) * 512u / (H + 1u - h0)) & 1023u]) >> 1;
        }
        for (n = 0; n < N; n++) {
            int64_t acc = 0;                                     /* (64 bits, rounded once: no truncation noise) */
            uint32_t idx = 0, dn = n * step;
            if (sp)
                for (h = 0; h < H; h++) {
                    idx = (idx + dn) & 1023u;                    /* (h + 1) n step */
                    acc += (((int64_t)sp[h][0] * SINE[(idx + 256u) & 1023u] + (int64_t)sp[h][1] * SINE[idx]) * tap[h]) >> 15;
                }
            d[n] = (int16_t)clamp((int32_t)((acc + 16384) >> 15), -32767, 32767);
        }
    }
}

/* the table the instrument plays: track 1's, when it plays WAVE (the synth: one instrument) */
static int32_t wt_wanted(void);
/* main loop: build the wanted table, one frame a call (~2 ms each on the FM-1): the voices are silent meanwhile */
static void wt_service(void)
{
    int32_t want = wt_wanted();
    if (want < 0)
        return;
    if (want != wt_have && want != wt_building) {        /* a new table: start over */
        wt_have = -1;
        wt_building = (int16_t)want;
        wt_next = 0;
    }
    if (wt_building >= 0) {
        wt_build_frame((uint32_t)wt_building, wt_next);
        if (++wt_next >= WT_NF) {
            wt_have = wt_building;
            wt_building = -1;
        }
    }
}

static void wave_note_on(track_t *t, voice_t *v)
{
    (void)t;
    if (!v->env && !v->env_out) {                        /* from silence: the phases apart (two oscillators) */
        v->ph[0] = 0;
        v->ph[1] = 0x55555555u;
        v->ph[2] = 0;
        v->s[0] = v->s[1] = v->s[3] = v->s[4] = v->s[5] = v->s[6] = 0;
        v->s[7] = -1;
    }
}

/* the copy for an increment: the most harmonics under 0.45 FS (128 >> l <= 0.45 2^32 / inc) */
static inline uint32_t wt_level(uint32_t inc)
{
    uint32_t r = inc ? 1932735283u / inc : 1000u, l = 0;
    while (l < WT_LEVELS - 1u && (128u >> l) > r)
        l++;
    return l;
}

/* one oscillator of the table into acc[] (+=, at g Q15): frames fi, fi + 1 crossfaded by fr (per sample, Q8), then
 * read between its points with a 4-point cubic (Hermite / Catmull-Rom): with eight points a harmonic its images stay
 * under the table's own band limit (linear interpolation's were ~-45 dB) */
static void wave_osc(int32_t *acc, uint32_t n, uint32_t *php, uint32_t inc, const int32_t *pos, int32_t g)
{
    uint32_t l = wt_level(inc), N = 1024u >> l, sh = 22u + l, i, ph = *php;   /* index = ph >> (32 - log2 N) */
    uint32_t off = WT_OFF[l], mask = N - 1u;
    for (i = 0; i < n; i++, ph += inc) {
        int32_t pq = pos[i], fi = pq >> 8, fr = pq & 255;
        uint32_t k = ph >> sh;
        int32_t t = (int32_t)((ph >> (sh - 15u)) & 0x7FFFu);       /* Q15 between point k and k + 1 */
        const int16_t *a = &wt_mip[fi][off], *b = &wt_mip[fi < (int32_t)WT_NF - 1 ? fi + 1 : fi][off];
        uint32_t k0 = (k - 1u) & mask, k2 = (k + 1u) & mask, k3 = (k + 2u) & mask;
        int32_t x0 = a[k0] + (((b[k0] - a[k0]) * fr) >> 8), x1 = a[k] + (((b[k] - a[k]) * fr) >> 8);
        int32_t x2 = a[k2] + (((b[k2] - a[k2]) * fr) >> 8), x3 = a[k3] + (((b[k3] - a[k3]) * fr) >> 8);
        int32_t c1 = (x2 - x0) >> 1, c2 = x0 - ((5 * x1) >> 1) + 2 * x2 - (x3 >> 1), c3 = ((x3 - x0) >> 1) + ((3 * (x1 - x2)) >> 1);
        int32_t y = (int32_t)(((((int64_t)c3 * t >> 15) + c2) * t >> 15) + c1);
        y = x1 + (int32_t)(((int64_t)y * t) >> 15);
        acc[i] += mulq15(y, g);
    }
    *php = ph;
}

static void wave_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    static int32_t acc[CTL], pos[CTL];               /* (static: the audio ISR's stack stays small) */
    uint32_t want = (uint32_t)p[P_E0] % WT_NALL, i, inc1 = m->inc, inc2;
    int32_t det = p[P_E2], sub = p[P_E3] * 258, drive = 32768 + p[P_E6] * 768, dw = p[P_E6] * 258, drv = p[P_E6];
    int32_t cut = (p[P_E4] << 8) + m->cutoff + (p[P_E7] * (v->pitch16 - 60 * 16) >> 4);
    int32_t p1 = clamp((p[P_E1] << 8) + m->shape - (64 << 8), 0, 127 << 8) * (int32_t)(WT_NF - 1u) / 127;   /* Q8 */
    int32_t p0 = v->s[7] < 0 ? p1 : v->s[7];
    int32_t ic1 = v->s[0], ic2 = v->s[1], ls[4] = {v->s[3], v->s[4], v->s[5], v->s[6]};
    int moog = t->p[P_FMODEL] == FM_MOOG;
    tsvf_t flt;
    ladder_t lad;
    if (n > CTL)
        return;
    if (wt_have != (int32_t)want) {                  /* not built yet (TABL just turned): silent, phases run on */
        v->ph[0] += inc1 * n;
        v->ph[1] += inc1 * n;
        v->ph[2] += (inc1 >> 1) * n;
        return;
    }
    /* DTN: the second oscillator DTN cents above the first (2^(c / 1200) - 1 ~ 37.9 / 65536 a cent up to 50: its
     * unison spread, TUNE and glide come with m->inc) */
    inc2 = inc1 + (uint32_t)(((uint64_t)inc1 * (uint32_t)(det * 38)) >> 16);
    for (i = 0; i < n; i++) {
        acc[i] = 0;
        pos[i] = p0 + (((p1 - p0) * (int32_t)i) >> CTL_LOG2);
    }
    wave_osc(acc, n, &v->ph[0], inc1, pos, det ? 16384 : 32767);
    if (det)
        wave_osc(acc, n, &v->ph[1], inc2, pos, 16384);
    else
        v->ph[1] += inc1 * n;
    if (sub) {
        uint32_t ph = v->ph[2], si = inc1 >> 1;
        for (i = 0; i < n; i++, ph += si)
            acc[i] += mulq15(osc_sine(ph), sub) >> 1;
        v->ph[2] = ph;
    } else {
        v->ph[2] += (inc1 >> 1) * n;
    }
    if (moog)
        ladder_coef(&lad, cut, p[P_E5]);
    else
        tsvf_coef(&flt, cut, p[P_E5]);
    for (i = 0; i < n; i++) {
        int32_t s = acc[i];
        if (drv)                                      /* ANALOG's drive */
            s += mulq15(softclip(((s >> 2) * (drive >> 2)) >> 11) - s, dw);
        {
            int32_t y = moog ? ladder_tick(&lad, s, ls) >> 1 : tsvf_lp(&flt, s >> 1, &ic1, &ic2), a = y < 0 ? -y : y;
            if (a > 16000) {
                a = 16000 + (softclip((a - 16000) * 2) >> 1);
                y = y < 0 ? -a : a;
            }
            s = y << 1;
        }
        out[i] += mulq15(mulq15(s, amp_at(m, i)), VOICE_FS) << 1;
    }
    v->s[0] = ic1;
    v->s[1] = ic2;
    v->s[3] = ls[0], v->s[4] = ls[1], v->s[5] = ls[2], v->s[6] = ls[3];
    v->s[7] = p1;
}

static const preset_t WAVE_PRESETS[] = {
    /* {TABL, POS, DTN, SUB, CUT, RES, DRV, KTR}, {A D S R}, ENV -> FILTER, mono; extras: SHP modulation (POS) */
    {"WT BASIC", {0, 40, 0, 0, 110, 10, 0, 64}, {0, 80, 110, 40}, 0, 0, FX(0, 10, 10, 20)},
    {"WT SWEEP", {11, 10, 8, 40, 122, 20, 0, 64}, {10, 90, 110, 70}, 10, 0, FX(0, 30, 20, 40),
     XP(P_ED_SHP + 1, 60, P_LRATE + 1, 30, P_LD_SHP + 1, 20)},
    {"PPG PAD", {11, 30, 10, 30, 118, 10, 0, 40}, {80, 100, 115, 95}, 0, 0, FX(0, 40, 20, 60),
     XP(P_LRATE + 1, 22, P_LD_SHP + 1, 40)},
    {"VOCAL WT", {4, 0, 6, 0, 110, 10, 0, 64}, {20, 90, 115, 60}, 0, 0, FX(0, 30, 20, 45),
     XP(P_LRATE + 1, 18, P_LD_SHP + 1, 63)},
    {"SYNC BASS", {6, 20, 0, 60, 80, 30, 30, 64}, {0, 70, 80, 30}, 20, 1, FX(10, 0, 0, 6),
     XP(P_ED_SHP + 1, 50, P_TRANS + 1, -12, P_GLIDE + 1, 24)},
    {"FM GROWL", {7, 30, 0, 40, 85, 40, 40, 64}, {0, 70, 90, 30}, 0, 1, FX(20, 0, 0, 4),
     XP(P_LRATE + 1, 70, P_LD_SHP + 1, 50, P_TRANS + 1, -12)},
    {"DIGI BELL", {12, 60, 0, 0, 120, 0, 0, 64}, {0, 100, 0, 90}, 0, 0, FX(0, 20, 30, 50), XP(P_ED_SHP + 1, -50)},
    {"RESO SWP", {5, 0, 7, 0, 110, 10, 0, 64}, {0, 90, 100, 60}, 0, 0, FX(0, 20, 30, 40), XP(P_ED_SHP + 1, 63)},
    {"FOLD LEAD", {9, 20, 6, 0, 100, 20, 20, 64}, {2, 70, 110, 40}, 0, 1, FX(0, 0, 26, 24),
     XP(P_LRATE + 1, 60, P_LD_SHP + 1, 30, P_GLIDE + 1, 30)},
    {"CZ WAVE", {10, 50, 0, 0, 110, 0, 0, 64}, {0, 90, 60, 50}, 0, 0, FX(0, 20, 10, 30), XP(P_ED_SHP + 1, -40)},
    {"CHIP WT", {14, 0, 0, 0, 127, 0, 0, 64}, {0, 60, 100, 20}, 0, 1, FX(0, 0, 20, 10),
     XP(P_LRATE + 1, 96, P_LD_SHP + 1, 40)},
    {"ORGAN WT", {13, 64, 0, 30, 120, 0, 10, 64}, {0, 30, 127, 20}, 0, 0, FX(0, 40, 0, 30)},
    {"HARMONICS", {2, 0, 5, 0, 120, 0, 0, 64}, {30, 90, 115, 80}, 0, 0, FX(0, 30, 20, 60), XP(P_ED_SHP + 1, 63)},
    {"COMB PAD", {15, 20, 12, 0, 100, 10, 0, 40}, {70, 100, 115, 95}, 0, 0, FX(0, 50, 20, 70),
     XP(P_LRATE + 1, 14, P_LD_SHP + 1, 60)},
    {"PWM STR", {1, 30, 9, 0, 90, 10, 0, 40}, {60, 100, 120, 90}, 0, 0, FX(0, 60, 0, 55),
     XP(P_LRATE + 1, 30, P_LD_SHP + 1, 40)},
    {"USER WT", {16, 0, 0, 0, 110, 10, 0, 64}, {0, 80, 110, 40}, 0, 0, FX(0, 10, 10, 20)},
};

static const engine_t ENG_WAVE = {
    "WAVE", {"WAVE", "FLT"},
    {
        {"TABL", F_ENUM, 0, WT_NALL - 1, 0, WT_ALL_NAMES, 0},
        {"POS", F_PCT, 0, 127, 0, 0, 0},
        {"DTN", F_INT, 0, 50, 0, 0, "ct"},
        {"SUB", F_PCT, 0, 127, 0, 0, 0},
        {"CUT", F_CUTOFF, 0, 127, 110, 0, 0},
        {"RES", F_PCT, 0, 127, 10, 0, 0},
        {"DRV", F_PCT, 0, 127, 0, 0, 0},
        {"KTR", F_PCT, 0, 127, 64, 0, 0},
    },
    WAVE_PRESETS, sizeof(WAVE_PRESETS) / sizeof(WAVE_PRESETS[0]), 1, wave_note_on, wave_render,
    0x5EDD, {P_E1, P_E4, P_E5, P_E0},
};
