/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper 0.6 (CLIPPER_SYNTH 1): an INSERT effect per sound, and MULTI (bi-timbral: the layer on its own channel).
 *   INSERT   TYPE OFF, or MIX 0: the sound bit for bit as without; every type changes the sound, bounded and finite;
 *            a TYPE changed under a held note fades (no step); a flanger's tail rings after the note, then stops;
 *            the layer's INSERT its own; kept with the sound: projects (0.5's read: INSERT off), user presets (0.5's
 *            records read, SLOOP 2.6's INSERT read), a SLOOP 2.6 project's track 1
 *   MULTI    a note on the layer's channel plays the layer only, any other channel and the keys the instrument only;
 *            each its own pitch bend and pedal; LAYER as before (every note on both) */
#define main hostsim_main
#include <stddef.h>
#include "hostsim.c"
#undef main
#define PROJ_HOST 1
static const uint8_t TRK_DEF[NPART][2] = {{ENGI_HYBRID, 0}, {1, 0}, {4, 5}};   /* (ui.c, Clipper) */
static uint32_t trk_def_engine(uint32_t i) { return i < NPART ? TRK_DEF[i][0] : 0u; }
#include "../firmware/src/project.c"
#define UP_HOST 1
#include "../firmware/src/upreset.c"

static int fails;
static void ok(int c, const char *what)
{
    printf("synth6: %-92s %s\n", what, c ? "ok" : "FAIL");
    fails += !c;
}

#define NCAP (4096u * 8u)
static int32_t cl[NCAP], cr[NCAP];
static uint32_t ncap;
static void run(uint32_t nb)
{
    static int32_t o[CTL * 2];
    uint32_t b, i;
    ncap = 0;
    for (b = 0; b < nb; b++) {
        mix_block(o, CTL);
        for (i = 0; i < CTL; i++)
            if (ncap < NCAP) {
                cl[ncap] = o[2 * i];
                cr[ncap++] = o[2 * i + 1];
            }
    }
}
static double rms(uint32_t a, uint32_t n)
{
    double e = 0;
    uint32_t i;
    for (i = a; i < a + n && i < ncap; i++)
        e += (double)cl[i] * cl[i];
    return sqrt(e / n);
}
static void quiet(void)
{
    uint32_t i, k;
    for (i = 0; i < NPART; i++)
        trk_all_off(&trk[i]);
    run(400);
    for (i = 0; i < NPART; i++)
        for (k = 0; k < NVOICE; k++)
            trk[i].v[k].active = trk[i].v[k].gate = 0, trk[i].v[k].stage = 0, trk[i].v[k].env = trk[i].v[k].env_out = 0;
    run(300);
}
/* ANALOG, a plain saw, no sends, no modulation */
static void plain(track_t *t, uint32_t eng)
{
    host_preset(t, eng, 0);
    t->p[P_DIST] = t->p[P_CHOR] = t->p[P_DLY] = t->p[P_REV] = 0;
    t->p[P_LD_PIT] = t->p[P_LD_FLT] = t->p[P_LD_SHP] = t->p[P_LD_AMP] = t->p[P_ED_PIT] = 0;
    t->p[P_ATK] = 0, t->p[P_SUS] = 127, t->p[P_REL] = 10, t->p[P_VOICE] = V_POLY, t->p[P_PAN] = 0;
    t->p[P_MWDST] = t->p[P_ATDST] = MD_OFF;
    t->p[P_ITYPE] = 0, t->p[P_IA] = TP[P_IA].def, t->p[P_IB] = TP[P_IB].def, t->p[P_IC] = TP[P_IC].def, t->p[P_IMIX] = 127;
}
static void midi(uint32_t st, uint32_t d1, uint32_t d2)
{
    midi_in_q[mi_w % MQ] = (st >> 4) | st << 8 | d1 << 16 | d2 << 24;
    mi_w++;
}
static uint32_t sounding(const track_t *t)
{
    uint32_t i, n = 0;
    for (i = 0; i < NVOICE; i++)
        n += t->v[i].active && t->v[i].gate;
    return n;
}

static int32_t ref[NCAP];
static void t_insert(void)
{
    track_t *t = &trk[0];
    uint32_t i, ty, same = 1, all = 1;
    char w[200];
    /* OFF / MIX 0: bit for bit (the INSERT on a block of a sound: nothing touched, nothing runs) */
    {
        static int32_t b[CTL], c[CTL];
        quiet();
        plain(t, 0u);
        trk_note_on(t, 57, 100);
        run(50);
        for (i = 0; i < CTL; i++)
            b[i] = c[i] = cl[ncap - CTL + i];
        t->p[P_ITYPE] = IT_FUZZ, t->p[P_IMIX] = 0;
        track_insert(t, b, CTL, 0);
        for (i = 0; i < CTL; i++)
            same &= b[i] == c[i];
        ok(same && !t->ins_run && !ins[0].w, "INSERT: MIX 0 is the sound bit for bit as with TYPE OFF (nothing runs)");
        quiet();
        plain(t, 0u);
        trk_note_on(t, 57, 100);
        run(200);
        memcpy(ref, cl, sizeof ref);                    /* (the dry sound, to compare the types with) */
    }
    /* every type: another sound, bounded */
    for (ty = 1; ty < IT_N; ty++) {
        double d = 0, pk = 0;
        quiet();
        plain(t, 0u);
        t->p[P_ITYPE] = (int16_t)ty, t->p[P_IA] = 90, t->p[P_IB] = 100, t->p[P_IC] = 70;
        trk_note_on(t, 57, 100);
        run(200);
        for (i = 0; i < ncap; i++) {
            double e = (double)cl[i] - ref[i];
            d += e * e;
            pk = fabs((double)cl[i]) > pk ? fabs((double)cl[i]) : pk;
        }
        d = sqrt(d / ncap) / (rms(0, ncap) + 1);
        snprintf(w, sizeof w, "INSERT %s: changes the sound (%.2f of its level), bounded (peak %.0f)", N_ITYPE[ty], d, pk);
        all &= d > 0.02 && pk < 32768;
        if (!(d > 0.02 && pk < 32768))
            ok(0, w);
    }
    ok(all, "INSERT: every type (SOFT HARD FOLD FUZZ CRUSH PHASR FLANG CHOR) changes the sound, bounded");
    /* a TYPE change under a held note: faded, no step */
    {
        double jump = 0, step = 0;
        quiet();
        plain(t, 3u);                                  /* (ANALOG's sine: a smooth wave, any step shows) */
        t->p[P_E0] = 3;
        t->p[P_ITYPE] = IT_SOFT, t->p[P_IA] = 40;
        trk_note_on(t, 45, 100);
        run(100);
        t->p[P_ITYPE] = IT_CHORUS;
        run(100);
        for (i = 1; i < ncap; i++) {
            double d = fabs((double)cl[i] - cl[i - 1]);
            if (i > 3 && i < ncap - 3)
                step = d > step ? d : step;
        }
        quiet();
        plain(t, 3u);
        t->p[P_E0] = 3;
        trk_note_on(t, 45, 100);
        run(200);
        for (i = 1; i < ncap; i++) {
            double d = fabs((double)cl[i] - cl[i - 1]);
            jump = d > jump ? d : jump;
        }
        snprintf(w, sizeof w, "INSERT: a TYPE changed under a held note fades (largest step %.0f, the dry sine's %.0f)", step, jump);
        ok(step < jump * 3.0, w);
    }
    /* a flanger's tail rings after the note, then the INSERT stops */
    {
        uint32_t b, rang = 0, stopped = 0;
        quiet();
        plain(t, 0u);
        t->p[P_REL] = 0;
        t->p[P_ITYPE] = IT_FLANGER, t->p[P_IC] = 127;
        trk_note_on(t, 57, 100);
        run(100);
        trk_note_off(t, 57);
        for (b = 0; b < 2000u; b++) {
            run(1);
            if (b > 40u && b < 60u && rms(0, CTL) > 1)
                rang = 1;
            {
                static uint32_t ph0;                    /* (the sweep stops: the part is not rendered any more) */
                if (b > 60u && ins[0].ph == ph0 && !t->tail) {
                    stopped = 1;
                    break;
                }
                ph0 = ins[0].ph;
            }
        }
        ok(rang && stopped, "INSERT: a flanger's feedback rings after the note, then it stops (the part no longer rendered)");
    }
    /* the layer's INSERT its own */
    {
        track_t *l = &trk[1];
        double a, b;
        quiet();
        plain(t, 0u);
        plain(l, 0u);
        l->p[P_LYMODE] = LY_LAYER;
        l->p[P_ITYPE] = IT_CRUSH, l->p[P_IA] = 8, l->p[P_IB] = 0;   /* 2 bits, 689 Hz: a ruin */
        trk_note_on(t, 57, 100);
        run(200);
        a = rms(4000, 8000);
        quiet();
        l->p[P_ITYPE] = 0;
        trk_note_on(t, 57, 100);
        run(200);
        b = rms(4000, 8000);
        snprintf(w, sizeof w, "INSERT: the layer's own (CRUSH on the layer changes the mix: %.0f against %.0f), the instrument's untouched",
                 a, b);
        ok(fabs(a - b) > 0.05 * b && t->p[P_ITYPE] == 0, w);
        l->p[P_LYMODE] = LY_OFF;
        quiet();
    }
}

static project_t pj;
static void t_storage(void)
{
    track_t *a = &trk[0], *b = &trk[1];
    uint32_t k, good;
    char w[200];
    quiet();
    plain(a, 0u);
    plain(b, 1u);
    a->p[P_ITYPE] = IT_PHASER, a->p[P_IA] = 11, a->p[P_IB] = 22, a->p[P_IC] = 33, a->p[P_IMIX] = 44;
    b->p[P_ITYPE] = IT_CRUSH, b->p[P_IA] = 55, b->p[P_IB] = 66, b->p[P_IC] = 77, b->p[P_IMIX] = 88;
    a->p[P_M3AMT] = -17, b->p[P_E2SUS] = 99;
    proj_capture(&pj);
    for (k = P_ITYPE; k <= P_IMIX; k++)
        a->p[k] = b->p[k] = TP[k].def;
    a->p[P_M3AMT] = b->p[P_E2SUS] = 0;
    proj_apply(&pj, 1);
    good = a->p[P_ITYPE] == IT_PHASER && a->p[P_IA] == 11 && a->p[P_IB] == 22 && a->p[P_IC] == 33 && a->p[P_IMIX] == 44 &&
           b->p[P_ITYPE] == IT_CRUSH && b->p[P_IA] == 55 && b->p[P_IB] == 66 && b->p[P_IC] == 77 && b->p[P_IMIX] == 88 &&
           a->p[P_M3AMT] == -17 && b->p[P_E2SUS] == 99 && sizeof pj == 3840u;
    ok(good, "project: both sounds' INSERT kept (with the matrix, LFO 2, ENV 2), the size unchanged");
    {   /* as 0.5 wrote it: the 0.6 bytes and their mark not there */
        uint8_t raw[3u * NLOCK];
        proj_capture(&pj);
        for (k = 0; k < NLOCK; k++)
            raw[3u * k] = pj.t[2].lock[k].param, raw[3u * k + 1u] = (uint8_t)pj.t[2].lock[k].val,
            raw[3u * k + 2u] = (uint8_t)((uint16_t)pj.t[2].lock[k].val >> 8);
        memset(raw + PROJ_XM1 + 1u, 0, sizeof raw - PROJ_XM1 - 1u);
        for (k = 0; k < NLOCK; k++)
            pj.t[2].lock[k].param = raw[3u * k], pj.t[2].lock[k].val = (int16_t)(raw[3u * k + 1u] | raw[3u * k + 2u] << 8);
        proj_apply(&pj, 1);
        ok(a->p[P_ITYPE] == 0 && b->p[P_ITYPE] == 0 && a->p[P_IMIX] == TP[P_IMIX].def && a->p[P_M3AMT] == -17 &&
           b->p[P_E2SUS] == 99, "a project of 0.5: its matrix, LFO 2, ENV 2 read, the INSERT off");
    }
    {   /* user presets: a round trip; 0.5's record; SLOOP 2.6's */
        up_rec_t r;
        int16_t v[P_COUNT], out[P_COUNT], def[P_COUNT];
        uint32_t i, pass, same = 1;
        for (i = 0; i < P_COUNT; i++)
            def[i] = i >= P_E0 ? 0 : TP[i].def;
        for (pass = 0; pass < 3u; pass++) {
            memset(&r, 0, sizeof r);
            for (i = 0; i < P_COUNT; i++)
                v[i] = (int16_t)(i >= P_E0 ? 40 + i : i < UP_E0V ? i * 3 :
                                 pass == 0 ? TP[i].min : pass == 1 ? TP[i].max : TP[i].min + (int32_t)(i * 7u) % (TP[i].max - TP[i].min + 1));
            r.used = UP_USED, r.ver = UP_VER, r.name[0] = 'X';
            up_rec_set(&r, v);
            up_params(&r, out, def);
            for (i = 0; i < P_COUNT; i++)
                same &= out[i] == v[i];
        }
        ok(same && r.np == 61u && up_valid(&r) && r.p[52] == 156 && r.p[53] == 40 + P_E0,
           "user presets: the INSERT packed with LFO 2 / ENV 2 / matrix in a record SLOOP reads, back exactly");
        /* a 0.5 record: the 25 packed as 0.5 did, p[71] = "CL" */
        memset(&r, 0, sizeof r);
        for (i = 0; i < P_COUNT; i++)
            v[i] = (int16_t)(i >= P_E0 ? 40 + i : i < UP_E0V ? i : i < P_ITYPE ? TP[i].max : TP[i].def);
        r.used = UP_USED, r.ver = UP_VER, r.name[0] = 'Y';
        up_rec_set(&r, v);
        r.p[UP_PMAX - 1u] = UP_XMAGIC;                 /* (0.5: the stream's bits 160.. not there, the mark "CL") */
        for (i = 129u; i < 160u; i++)
            r.p[UP_NP + (i >> 4)] = (int16_t)((uint16_t)r.p[UP_NP + (i >> 4)] & ~(1u << (i & 15u)));
        up_params(&r, out, def);
        for (i = 0, same = 1; i < P_COUNT; i++)
            same &= out[i] == (i >= P_ITYPE && i < P_E0 ? def[i] : v[i]);
        ok(same, "user presets: a 0.5 record: LFO 2 / ENV 2 / matrix read, the INSERT off");
        /* SLOOP 2.6's (np 71): its INSERT at 53..57 */
        memset(&r, 0, sizeof r);
        r.used = UP_USED, r.ver = UP_VER, r.np = 71, r.name[0] = 'S';
        for (i = 0; i < 71u; i++)
            r.p[i] = (int16_t)(i < 53u ? i : i < 58u ? 10 + i : i < 63u ? 1 : 200 + i);
        up_params(&r, out, def);
        snprintf(w, sizeof w, "user presets: a SLOOP 2.6 record keeps its INSERT (TYPE %d, MIX %d)", out[P_ITYPE], out[P_IMIX]);
        ok(out[P_ITYPE] == 63 && out[P_IA] == 64 && out[P_IMIX] == 67 && out[P_E0] == 263 && out[P_L2RATE] == def[P_L2RATE], w);
    }
    {   /* a SLOOP 2.6 project (FUN6): its track 1's INSERT */
        static project_v6_t p6;
        uint32_t i;
        memset(&p6, 0, sizeof p6);
        p6.magic = PROJ_MAGIC_V6, p6.size = sizeof p6;
        for (i = 0; i < PROJ_NG; i++)
            p6.g[i] = GP[i].def;
        for (i = 0; i < NTRK; i++) {
            for (k = 0; k < PROJ_E0; k++)
                p6.t[i].p[k] = TP[k].def;
            for (k = 0; k < NLOCK; k++)
                p6.t[i].lock[k].step = LOCK_FREE;
            p6.t[i].x[0] = (uint8_t)(i ? IT_FUZZ : IT_FLANGER), p6.t[i].x[1] = 12, p6.t[i].x[4] = 100;
        }
        p6.sum = proj_hash(&p6, sizeof p6 - 4u);
        good = proj_import(&proj_slot[1], &p6, (int)sizeof p6);
        proj_import(&proj_slot[2], &pj, (int)sizeof pj);   /* (another slot read after it: its own) */
        proj_apply(&proj_slot[1], 1);
        ok(good && a->p[P_ITYPE] == IT_FLANGER && a->p[P_IA] == 12 && a->p[P_IMIX] == 100,
           "a SLOOP 2.6 project (FUN6): its track 1's INSERT on the instrument");
    }
    for (k = P_ITYPE; k <= P_IMIX; k++)
        a->p[k] = b->p[k] = TP[k].def;
    clipper_proj_neutral();
    trk[1].p[P_LYMODE] = LY_OFF;
}

static void t_multi(void)
{
    track_t *a = &trk[0], *l = &trk[1];
    uint32_t i;
    quiet();
    plain(a, 0u);
    plain(l, 1u);
    l->p[P_LYMODE] = LY_MULTI, l->p[P_LYCH] = 2;
    midi(0x91, 64, 100);                               /* channel 2: the layer */
    run(4);
    ok(sounding(l) == 1 && sounding(a) == 0, "MULTI: a note on the layer's channel (2) plays the layer only");
    midi(0x90, 60, 100);                               /* channel 1: the instrument */
    midi(0x9F, 67, 100);                               /* channel 16 too */
    run(4);
    ok(sounding(a) == 2 && sounding(l) == 1, "MULTI: any other channel plays the instrument only");
    input_on(a, 72, 100);                              /* the keys */
    run(4);
    ok(sounding(a) == 3 && sounding(l) == 1, "MULTI: the keys play the instrument only");
    midi(0xE1, 0, 127);                                /* a bend on channel 2 */
    run(4);
    ok(part_ctl[1].bend > 30000 && part_ctl[0].bend == 0, "MULTI: the layer's channel bends the layer only");
    midi(0xB0, 64, 127);                               /* the pedal on channel 1 */
    midi(0x80, 60, 0);
    midi(0x81, 64, 0);
    run(4);
    {
        uint32_t held = 0;
        for (i = 0; i < NVOICE; i++)
            held += a->v[i].active && a->v[i].note == 60 && a->v[i].sus;
        ok(held == 1 && sounding(l) == 0, "MULTI: the instrument's pedal holds its note, the layer's note lets go");
    }
    midi(0xB0, 64, 0), midi(0xE1, 0, 64);
    midi(0x80, 67, 0), midi(0x8F, 67, 0);
    input_off(a, 72);
    run(4);
    quiet();
    l->p[P_LYMODE] = LY_LAYER;
    midi(0x91, 64, 100);
    run(4);
    ok(sounding(a) == 1 && sounding(l) == 1, "LAYER (as before): a note on any channel plays both sounds");
    midi(0x81, 64, 0);
    run(4);
    ok(sounding(a) == 0 && sounding(l) == 0, "LAYER: its note-off ends both");
    l->p[P_LYMODE] = LY_OFF;
    quiet();
}

int main(void)
{
    uint32_t i;
    host_tracks_init();
    for (i = 0; i < NPART; i++)
        host_preset(&trk[i], TRK_DEF[i][0], TRK_DEF[i][1]);
    clipper_proj_neutral();
    trk[1].p[P_LYMODE] = LY_OFF;
    t_insert();
    t_storage();
    t_multi();
    printf(fails ? "synth6: %d FAILED\n" : "synth6: all passed\n", fails);
    return fails;
}
