/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper 0.3: the synth's own pages (scope SC_SYN, params.c): the LAYER, the A/B MORPH and the RANDOMIZER.
 *   LAYER    MODE (OFF, LAYER, SPLIT, MULTI), SOUND (any factory or user sound for track 2), SPLIT (track 1 from that
 *            note up, track 2 below; MULTI: CH, the MIDI channel track 2 answers on its own, every other channel and
 *            the keys playing track 1: bi-timbral, 0.6), LEVEL; LAYER 2: OCT, PAN, FILT (track 2's), SWAP (the two sounds trade places: the
 *            way to edit the layer's sound with every page, then SWAP back). The routing: voice.c trk_note_on.
 *   MORPH    SET A, SET B: the sound now as A, as B; MORPH 0..127 between them (every continuous value on the line
 *            between, the switches at half way; A and B on two engines: the engine changes at half way). MIDI CC 20.
 *   RANDOM   AMT (how far from the sound now), WHAT (EDIT: the engine's 8 values; +ENV: and the envelopes; ALL: and
 *            the LFO, its destinations and the sends), RAND, BACK (the sound before the last RAND).
 * Track 2's settings are its own parameters (core.h P_LYMODE ..); the others live here (not in the project). */
enum { X_LYMODE, X_LYSOUND, X_LYSPLIT, X_LYLEVEL, X_LYOCT, X_LYPAN, X_LYFILT, X_LYSWAP,
       X_MORPH, X_SETA, X_SETB, X_RNDAMT, X_RNDWHAT, X_RNDGO, X_RNDBACK,
       X_MSLOT, X_MSRC, X_MDST, X_MAMT,                /* (X_MSRC..X_MAMT: the slot's P_MnSRC..P_MnAMT) */
       /* 0.6: FMX (eng_fmx.c): the selected operator's values, the routing, the patch, STORE (the part's patch) */
       X_FOP, X_FWAVE, X_FMODE, X_FRATIO, X_FFINE, X_FLEVEL, X_FVEL, X_FKSC, X_FATK, X_FDEC, X_FSUS, X_FREL,
       X_FTO, X_FFROM, X_FAMT, X_FOUT, X_FDTN, X_FFIX, X_FPEG, X_FPTIM, X_FSLOT, X_FSTORE, X_COUNT };
static const char *const N_RNDWHAT[] = {"EDIT", "+ENV", "ALL"};
static const char *const N_OCTS[] = {"-2", "-1", "0", "+1", "+2"};
static int16_t xv[X_COUNT] = {[X_RNDAMT] = 40, [X_RNDWHAT] = 1, [X_MSLOT] = 1, [X_FOP] = 1, [X_FTO] = 1, [X_FFROM] = 2,
                              [X_FSLOT] = 1};
static const char *const N_FRATIO[32] = {"0.5", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13",
                                         "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26",
                                         "27", "28", "29", "30", "31"};
static const char *const N_FFIXC[4] = {"1Hz", "10Hz", "100Hz", "1kHz"};
_Static_assert(X_FOP == 19 && X_FSTORE == 40, "params.c PAGES: FMX's ids");
static param_desc_t xd_sound = {"SOUND", F_INT, 0, 0, 0, 0, 0};   /* (its range: the list's length, syn_desc) */
static const param_desc_t XD[X_COUNT] = {
    [X_LYMODE] = PE("MODE", N_LYMODE, LY_OFF),
    [X_LYSPLIT] = {"SPLIT", F_SPLIT, 16, 127, 64, 0, 0},
    [X_LYLEVEL] = PD("LEVEL", F_DB, 0, 127, 104),
    [X_LYOCT] = PE("OCT", N_OCTS, 2),
    [X_LYPAN] = PD("PAN", F_BIPCT, -64, 63, 0),
    [X_LYFILT] = PD("FILT", F_FILT, -64, 63, 0),
    [X_LYSWAP] = PE("SWAP", N_GO, 0),
    [X_MORPH] = PD("MORPH", F_PCT, 0, 127, 0),
    [X_SETA] = PE("SET A", N_GO, 0),
    [X_SETB] = PE("SET B", N_GO, 0),
    [X_RNDAMT] = PD("AMT", F_PCT, 0, 127, 40),
    [X_RNDWHAT] = PE("WHAT", N_RNDWHAT, 1),
    [X_RNDGO] = PE("RAND", N_GO, 0),
    [X_RNDBACK] = PE("BACK", N_GO, 0),
    [X_MSLOT] = PD("SLOT", F_INT, 1, 6, 1),
    [X_FOP] = PD("OP", F_INT, 1, 6, 1),
    [X_FWAVE] = PE("WAVE", N_FMX_WAVE, 0),
    [X_FMODE] = PE("MODE", N_FMX_MODE, 0),
    [X_FRATIO] = PE("RATIO", N_FRATIO, 1),
    [X_FFINE] = PD("FINE", F_INT, 0, 99, 0),
    [X_FLEVEL] = PD("LEVEL", F_INT, 0, 127, 127),
    [X_FVEL] = PD("VEL", F_PCT, 0, 127, 0),
    [X_FKSC] = PD("KSC", F_PCT, 0, 127, 0),
    [X_FATK] = PD("ATK", F_TIME, 0, 127, 0),
    [X_FDEC] = PD("DEC", F_TIME, 0, 127, 70),
    [X_FSUS] = PD("SUS", F_PCT, 0, 127, 127),
    [X_FREL] = PD("REL", F_TIME, 0, 127, 40),
    [X_FTO] = PD("TO", F_INT, 1, 6, 1),
    [X_FFROM] = PD("FROM", F_INT, 1, 6, 2),
    [X_FAMT] = PD("AMT", F_INT, 0, 127, 0),
    [X_FOUT] = PD("OUT", F_INT, 0, 127, 0),
    [X_FDTN] = PD("DTN", F_INT, -7, 7, 0),
    [X_FFIX] = PE("FREQ", N_FMX_FIX, 0),
    [X_FPEG] = PD("PEG", F_BIPCT, -64, 63, 0),
    [X_FPTIM] = PD("PTIM", F_TIME, 0, 127, 40),
    [X_FSLOT] = PD("SLOT", F_INT, 1, FM6_BANK_N, 1),
    [X_FSTORE] = PE("STORE", N_GO, 0),
};

/* ---- a sound (engine + every parameter) as a snapshot */
typedef struct {
    uint8_t ok, eng, preset, user;
    int16_t p[P_COUNT];
} syn_snap_t;
static syn_snap_t syn_a, syn_b, syn_back;

static void snap_take(syn_snap_t *s, const track_t *t)
{
    s->ok = 1;
    s->eng = t->eng_req;
    s->preset = t->preset;
    s->user = t->user;
    memcpy(s->p, t->p, sizeof s->p);
}
/* a snapshot's sound into t (its mix and pattern parameters stay: param_kept) */
static void snap_put(track_t *t, const syn_snap_t *s)
{
    uint32_t i;
    if (!s->ok)
        return;
    sound_fade_begin(t);
    fm1_irq_off();
    t->eng_req = s->eng;
    for (i = 0; i < P_COUNT; i++)
        if (i >= P_E0 || !param_kept(i))
            t->p[i] = s->p[i];
    t->preset = s->preset;
    t->user = s->user;
    fm1_irq_on();
    fm6_track_loaded(t);
    ui.force = 1;
}

/* a value's kind for MORPH and RANDOM: 1 a switch (the names), 0 a continuous one */
static const param_desc_t *syn_pdesc(uint32_t eng, uint32_t i)
{
    return i >= P_E0 ? &ENGINES[eng % NENGINES]->edit[i - P_E0] : &TP[i];
}
static int syn_switch(const param_desc_t *d) { return d->fmt == F_ENUM || d->fmt == F_ONOFF || d->names; }

/* the values MORPH and RANDOM move: the sound, not its mix, pattern, key or voice mode */
static int syn_sound_param(uint32_t i)
{
    if (i >= P_E0)
        return i <= P_E7;
    return !param_kept(i) && i != P_ED_FX && i != P_VOICE && i != P_PRIO && i != P_ALLOC && i != P_GLMODE &&
           !(i >= P_AMODE && i <= P_AORDER);
}

static void syn_morph_apply(int32_t m)
{
    track_t *t = &trk[0];
    uint32_t i;
    if (!syn_a.ok || !syn_b.ok)
        return;
    if (syn_a.eng != syn_b.eng) {                     /* two engines: the one of the nearer end */
        const syn_snap_t *s = m < 64 ? &syn_a : &syn_b;
        if (t->eng_req != s->eng)
            snap_put(t, s);
        return;
    }
    if (t->eng_req != syn_a.eng)
        snap_put(t, &syn_a);
    for (i = 0; i < P_COUNT; i++) {
        const param_desc_t *d = syn_pdesc(syn_a.eng, i);
        int32_t a = syn_a.p[i], b = syn_b.p[i];
        if (!syn_sound_param(i) || a == b)
            continue;
        t->p[i] = (int16_t)(syn_switch(d) ? (m < 64 ? a : b) : a + ((b - a) * m + (b > a ? 63 : -63)) / 127);
    }
    ui.force = 1;
}

/* ---- the randomizer */
static uint32_t syn_seed = 0x9E3779B9u;
static uint32_t syn_rnd(void)
{
    syn_seed ^= syn_seed << 13;
    syn_seed ^= syn_seed >> 17;
    syn_seed ^= syn_seed << 5;
    return syn_seed;
}
/* the range a random value may take: kept musical (no endless attack, no siren of pitch modulation) */
static void syn_rnd_range(uint32_t i, const param_desc_t *d, int32_t *lo, int32_t *hi)
{
    *lo = d->min;
    *hi = d->max;
    switch (i) {
    case P_ATK: *hi = 90; break;
    case P_REL: *hi = 105; break;
    case P_ED_PIT: *lo = -8, *hi = 8; break;
    case P_LD_PIT: *lo = -6, *hi = 6; break;
    case P_LD_AMP: *hi = 60; break;
    case P_DIST: *hi = 70; break;
    default: break;
    }
    if (i >= P_M1SRC && i <= P_M6AMT) {               /* a matrix slot's amount: moderate, the pitch finer */
        int32_t r = trk[0].p[i - 1u] == MT_PITCH ? 6 : 40;
        *lo = -r;
        *hi = r;
    }
    if (i >= P_E0 && d->names) {                      /* no empty user slot (WAVE's USR1..4 and the like) */
        while (*hi > *lo && d->names[*hi - d->min] && d->names[*hi - d->min][0] == 'U' && d->names[*hi - d->min][1] == 'S')
            (*hi)--;
    }
}
static int syn_rnd_in(uint32_t i, uint32_t what)
{
    if (i >= P_E0 && i <= P_E7)
        return 1;
    if (what >= 1u && ((i >= P_ATK && i <= P_REL) || (i >= P_ED_FLT && i <= P_ED_SHP)))
        return 1;
    if (what >= 2u && ((i >= P_LRATE && i <= P_LD_AMP) || (i >= P_DIST && i <= P_REV) || i == P_VELF))
        return 1;
    if (what >= 2u && ((i >= P_L2RATE && i <= P_L2MODE) || (i >= P_E2ATK && i <= P_E2REL)))
        return 1;                                     /* (0.4: LFO 2, ENV 2, the amounts of the slots in use) */
    if (what >= 2u && i >= P_M1SRC && i <= P_M6AMT && (i - P_M1SRC) % 3u == 2u)
        return trk[0].p[i - 2u] != MS_OFF && trk[0].p[i - 1u] != MT_OFF;
    return 0;
}
static void syn_randomize(void)
{
    track_t *t = &trk[0];
    int32_t amt = xv[X_RNDAMT];
    uint32_t i, what = (uint32_t)xv[X_RNDWHAT];
    snap_take(&syn_back, t);
    sound_fade_begin(t);
    syn_seed ^= fm1_ms * 2654435761u;
    for (i = 0; i < P_COUNT; i++) {
        const param_desc_t *d = syn_pdesc(t->eng_req, i);
        int32_t lo, hi, v = t->p[i], u;
        if (!syn_rnd_in(i, what))
            continue;
        syn_rnd_range(i, d, &lo, &hi);
        if (hi <= lo)
            continue;
        u = lo + (int32_t)(syn_rnd() % (uint32_t)(hi - lo + 1));
        if (syn_switch(d)) {
            if ((int32_t)(syn_rnd() % 128u) < amt)
                v = u;
        } else {
            v += (u - v) * amt / 127;
        }
        t->p[i] = (int16_t)clamp(v, d->min, d->max);
    }
    t->user = 0;
    ui.force = 1;
}

/* ---- the layer: its sound by the place in the preset list (as PRESETS: factory by kind, then the user's) */
static uint32_t layer_pos(uint32_t *total)
{
    const track_t *t = &trk[1];
    uint32_t i, cur = 0;
    if (!bank_ready)
        bank_resolve();
    for (i = 0; i < NBANK; i++)
        if (BANK[i].e == t->eng_req && bank_pi[i] == t->preset)
            cur = i;
    if (t->user && t->user <= UP_SLOTS && up_used(t->user - 1u))
        cur = NBANK + up_rank(t->user - 1u);
    *total = NBANK + up_count();
    return cur;
}
static void layer_load(uint32_t n)
{
    uint32_t k, e = preset_at(n, &k);
    track_t *t = &trk[1];
    if (e == NENGINES) {
        up_load_to(t, k);
        return;
    }
    if (e != t->eng_req)
        set_engine_of(t, e);
    apply_preset_to(t, k);
    ui.force = 1;
}
/* track 1 and track 2 trade their sounds (each keeps its mix, its pattern and the layer's settings) */
static void layer_swap(void)
{
    track_t *a = &trk[0], *b = &trk[1];
    syn_snap_t sa, sb;
    snap_take(&sa, a);
    snap_take(&sb, b);
    sound_fade_begin(a);
    sound_fade_begin(b);
    trk_all_off(a);
    fm1_irq_off();
    a->eng_req = sb.eng, b->eng_req = sa.eng;
    {
        uint32_t i;
        for (i = 0; i < P_COUNT; i++)
            if (i >= P_E0 || !param_kept(i)) {
                a->p[i] = sb.p[i];
                b->p[i] = sa.p[i];
            }
    }
    a->preset = sb.preset, b->preset = sa.preset;
    a->user = sb.user, b->user = sa.user;
    hy_slot_seen[0] = hy_slot_seen[1] = 0;            /* HYBRID: each part its patch again */
    fm1_irq_on();
    fm6_track_loaded(a);
    fm6_track_loaded(b);
    {   /* FMX (0.6): the two parts' patches trade places too, edits and all */
        fmx_patch_t x = fmx_cur[0];
        int16_t s = fmx_slot_seen[0];
        fm1_irq_off();
        fmx_cur[0] = fmx_cur[1], fmx_cur[1] = x;
        fmx_slot_seen[0] = fmx_slot_seen[1], fmx_slot_seen[1] = s;
        fmx_reload &= (uint8_t)~3u;
        fmx_dirty[0] = fmx_dirty[1] = 1;
        fm1_irq_on();
    }
    sync_reload = 1;
    ui.force = 1;
}

/* params.c page_desc: a value of the synth's pages */
static const param_desc_t *syn_desc(uint32_t id, int16_t **valp)
{
    track_t *l = &trk[1];
    switch (id) {
    case X_LYMODE: *valp = &l->p[P_LYMODE]; break;
    case X_LYSPLIT:
        if (layer_multi()) {                          /* 0.6, MULTI: no split; the knob is the layer's channel */
            static const param_desc_t ch = {"CH", F_INT, 1, 16, 2, 0, 0};
            *valp = &l->p[P_LYCH];
            return &ch;
        }
        *valp = &l->p[P_LYSPLIT];
        break;
    case X_LYLEVEL: *valp = &l->p[P_LEVEL]; break;
    case X_LYOCT: *valp = &l->p[P_LYOCT]; break;
    case X_LYPAN: *valp = &l->p[P_PAN]; break;
    case X_LYFILT: *valp = &l->p[P_TFLT]; break;
    case X_MSRC:
    case X_MDST:
    case X_MAMT: {                                    /* the slot's own values, the selected sound's */
        uint32_t pid = P_M1SRC + 3u * (uint32_t)(clamp(xv[X_MSLOT], 1, 6) - 1) + (id - X_MSRC);
        *valp = &TSEL->p[pid];
        return track_desc(TSEL, pid);
    }
    case X_FWAVE: case X_FMODE: case X_FRATIO: case X_FFINE: case X_FLEVEL: case X_FVEL: case X_FKSC: case X_FATK:
    case X_FDEC: case X_FSUS: case X_FREL: case X_FAMT: case X_FOUT: case X_FDTN: case X_FFIX: case X_FPEG:
    case X_FPTIM: {                                   /* FMX: the part's patch, the selected operator / connection */
        const fmx_patch_t *p = &fmx_cur[song.sel % NPART];
        const fmx_op_t *o = &p->op[clamp(xv[X_FOP], 1, 6) - 1];
        uint32_t to = (uint32_t)clamp(xv[X_FTO], 1, 6) - 1u, fr = (uint32_t)clamp(xv[X_FFROM], 1, 6) - 1u;
        static param_desc_t rd;
        int16_t v;
        switch (id) {
        case X_FWAVE: v = o->wave; break;
        case X_FMODE: v = (int16_t)(o->mode & 3u); break;
        case X_FRATIO: v = o->coarse; break;
        case X_FFINE: v = o->fine; break;
        case X_FLEVEL: v = o->level; break;
        case X_FVEL: v = o->vel; break;
        case X_FKSC: v = o->ksc; break;
        case X_FATK: case X_FDEC: case X_FSUS: case X_FREL: v = o->adsr[id - X_FATK]; break;
        case X_FAMT: v = p->mod[to][fr]; break;
        case X_FOUT: v = p->out[to]; break;
        case X_FDTN: v = (int16_t)((int32_t)o->dtn - 7); break;
        case X_FFIX: v = (int16_t)((o->mode & FXM_FIX) != 0u); break;
        case X_FPEG: v = (int16_t)((int32_t)p->peg - 64); break;
        default: v = p->ptime; break;
        }
        xv[id] = v;
        *valp = &xv[id];
        if (id == X_FRATIO && (o->mode & FXM_FIX)) {  /* FIX: the decade */
            rd = XD[X_FRATIO];
            rd.max = 3, rd.names = N_FFIXC;
            xv[id] = (int16_t)(o->coarse & 3u);
            return &rd;
        }
        if (id == X_FAMT && to == fr) {               /* an operator into itself: feedback */
            rd = XD[X_FAMT];
            rd.label = "FB";
            return &rd;
        }
        return &XD[id];
    }
    case X_LYSOUND: {
        uint32_t total;
        xv[X_LYSOUND] = (int16_t)layer_pos(&total);
        xd_sound.max = (int16_t)(total ? total - 1u : 0u);
        *valp = &xv[X_LYSOUND];
        return &xd_sound;
    }
    default:
        if (id >= X_COUNT) {
            *valp = 0;
            return 0;
        }
        *valp = &xv[id];
        break;
    }
    return &XD[id];
}

/* ui_input.c: a value of the synth's pages was turned to *vp */
static void syn_changed(uint32_t id, int16_t *vp)
{
    switch (id) {
    case X_LYSOUND:
        layer_load((uint32_t)*vp);
        break;
    case X_LYMODE:
        trk_all_off1(&trk[1]);                        /* (its notes came by another way: let them go) */
        ui_say("MODE ", N_LYMODE[clamp(*vp, 0, LY_MULTI)]);
        break;
    case X_LYSWAP:
        if (*vp) {
            *vp = 0;
            layer_swap();
            ui_message("SOUNDS SWAPPED");
        }
        break;
    case X_MORPH:
        if (!syn_a.ok || !syn_b.ok)
            ui_message("SET A AND B FIRST");
        else if (syn_a.eng != syn_b.eng && (*vp < 64) != (trk[0].eng_req == syn_a.eng))
            ui_message("A / B: TWO ENGINES");
        syn_morph_apply(*vp);
        break;
    case X_SETA:
    case X_SETB:
        if (*vp) {
            *vp = 0;
            snap_take(id == X_SETA ? &syn_a : &syn_b, &trk[0]);
            xv[X_MORPH] = id == X_SETA ? 0 : 127;
            ui_message(id == X_SETA ? "A = THIS SOUND" : "B = THIS SOUND");
        }
        break;
    case X_MSLOT:
    case X_FOP:
    case X_FTO:
    case X_FFROM:
        ui.force = 1;
        break;
    case X_FWAVE: case X_FMODE: case X_FRATIO: case X_FFINE: case X_FLEVEL: case X_FVEL: case X_FKSC: case X_FATK:
    case X_FDEC: case X_FSUS: case X_FREL: case X_FAMT: case X_FOUT: case X_FDTN: case X_FFIX: case X_FPEG:
    case X_FPTIM: {                                   /* FMX: into the part's patch (fmx_block rebuilds its own) */
        uint32_t tr = song.sel % NPART;
        fmx_patch_t *p = &fmx_cur[tr];
        fmx_op_t *o = &p->op[clamp(xv[X_FOP], 1, 6) - 1];
        uint32_t to = (uint32_t)clamp(xv[X_FTO], 1, 6) - 1u, fr = (uint32_t)clamp(xv[X_FFROM], 1, 6) - 1u;
        uint8_t u = (uint8_t)clamp(*vp, 0, 127);
        switch (id) {
        case X_FWAVE: o->wave = u; break;
        case X_FMODE: o->mode = (uint8_t)((o->mode & FXM_FIX) | (u % FXM_N)); break;
        case X_FRATIO: o->coarse = u; break;
        case X_FFINE: o->fine = u; break;
        case X_FLEVEL: o->level = u; break;
        case X_FVEL: o->vel = u; break;
        case X_FKSC: o->ksc = u; break;
        case X_FATK: case X_FDEC: case X_FSUS: case X_FREL: o->adsr[id - X_FATK] = u; break;
        case X_FAMT: p->mod[to][fr] = u; p->alg = 0; break;
        case X_FOUT: p->out[to] = u; p->alg = 0; break;
        case X_FDTN: o->dtn = (uint8_t)clamp(*vp + 7, 0, 14); break;
        case X_FFIX: o->mode = (uint8_t)((o->mode & 3u) | (*vp ? FXM_FIX : 0u)); o->coarse &= *vp ? 3u : 31u; break;
        case X_FPEG: p->peg = (uint8_t)clamp(*vp + 64, 0, 127); break;
        default: p->ptime = u; break;
        }
        if ((id == X_FAMT || id == X_FOUT) && TSEL->p[P_E1]) {   /* the routing changed: the template's off (ALG
                                                               * PAT), what it filled kept in the patch */
            uint32_t a = (uint32_t)TSEL->p[P_E1] - 1u, d, s;
            for (d = 0; d < FMX_NOP; d++) {
                for (s = 0; s < FMX_NOP; s++)
                    if (!((FMX_ALG[a][d] >> s) & 1u))
                        p->mod[d][s] = d == to && s == fr ? p->mod[d][s] : 0u;
                    else if (!p->mod[d][s] && !(d == to && s == fr))
                        p->mod[d][s] = s == d ? 60u : 100u;
                if (!(d == to && id == X_FOUT))
                    p->out[d] = (FMX_ALG[a][6] >> d) & 1u ? (p->out[d] ? p->out[d] : 100u) : 0u;
            }
            TSEL->p[P_E1] = 0;
        }
        fmx_dirty[tr] = 1;
        break;
    }
    case X_FSTORE:
        if (*vp) {
            *vp = 0;
            if (song.playing)
                ui_message("STOP THE SONG FIRST");
            else if (!fmx_bank_store)
                ui_message("NO FLASH");
            else if (fmx_bank_store((uint32_t)clamp(xv[X_FSLOT], 1, FM6_BANK_N) - 1u,
                                    (const uint8_t *)&fmx_cur[song.sel % NPART]))
                ui_message("STORE ERROR");
            else {
                char m[16];
                TSEL->p[P_E0] = (int16_t)(FMX_NFACTORY + (uint32_t)clamp(xv[X_FSLOT], 1, FM6_BANK_N) - 1u);
                fmx_slot_seen[song.sel % NPART] = (int16_t)(TSEL->p[P_E0] + 1);   /* (its patch: this one) */
                str_cpy(m, "STORED B", sizeof m);
                fmx_reload &= (uint8_t)~(1u << (song.sel % NPART));
                fmt_int(m + str_len(m), xv[X_FSLOT]);
                ui_message(m);
            }
        }
        break;
    case X_RNDGO:
        if (*vp) {
            *vp = 0;
            syn_randomize();
            ui_message("RANDOM");
        }
        break;
    case X_RNDBACK:
        if (*vp) {
            *vp = 0;
            if (syn_back.ok) {
                snap_put(&trk[0], &syn_back);
                ui_message("BACK");
            }
        }
        break;
    default:
        break;
    }
}

/* once a UI frame: MIDI CC 20 moves MORPH */
static void syn_tick(void)
{
    int16_t r = syn_morph_req;
    if (r >= 0) {
        syn_morph_req = -1;
        xv[X_MORPH] = r;
        syn_morph_apply(r);
    }
}

/* the graph of the LAYER pages: the layer's sound and the keyboard (the split) */
static void graph_layer(void)
{
    const track_t *l = &trk[1];
    const engine_t *e = ENGINES[l->eng_req % NENGINES];
    char nm[16];
    uint32_t m = layer_mode(), i;
    int32_t sp = l->p[P_LYSPLIT] - 4, x;
    if (l->user && l->user <= UP_SLOTS && up_used(l->user - 1u))
        up_name(l->user - 1u, nm);
    else
        str_cpy(nm, e->npresets ? e->presets[l->preset % e->npresets].name : e->name, sizeof nm);
    if (m == LY_MULTI) {                              /* 0.6: two parts, the layer on its channel */
        char c[12];
        str_cpy(c, "MULTI  CH ", sizeof c);
        fmt_int(c + str_len(c), (int32_t)layer_ch());
        cv_text(8, 4, &FONT_S, c, C_GRAY);
    } else {
        cv_text(8, 4, &FONT_S, m ? "LAYER" : "LAYER OFF", m ? C_GRAY : C_DIM);
    }
    cv_text(8, 22, &FONT_S, nm, m ? C_WHITE : C_GRAY);
    cv_text(8, 40, &FONT_S, e->name, C_DIM);
    for (i = 0; i < 61u; i++) {                       /* the keys C2..C7 (36..96): where each sound plays */
        uint32_t note = 36u + i;
        int on2 = m == LY_LAYER || m == LY_MULTI || (m == LY_SPLIT && (int32_t)note < sp);
        int on1 = m != LY_SPLIT || (int32_t)note >= sp;
        x = 40 + (int32_t)i * 3;
        cv_rect(x, 62, 2, 7, on1 ? C_HI : C_LINE);
        cv_rect(x, 73, 2, 7, on2 ? TE_COL[1] : C_LINE);
        if (note % 12u == 0u)
            cv_rect(x, 82, 1, 3, C_DIM);
    }
    cv_text(8, 58, &FONT_S, "1", C_HI);
    cv_text(24, 69, &FONT_S, "2", m ? TE_COL[1] : C_DIM);
}
static void graph_morph(void)
{
    static const char *const AB[2] = {"A", "B"};
    uint32_t k;
    int32_t w = xv[X_MORPH] * 200 / 127;
    for (k = 0; k < 2u; k++) {
        const syn_snap_t *s = k ? &syn_b : &syn_a;
        char nm[16];
        const engine_t *e = ENGINES[s->eng % NENGINES];
        int32_t y = 6 + (int32_t)k * 22;
        cv_text(8, y, &FONT_S, AB[k], s->ok ? C_WHITE : C_DIM);
        if (s->ok)
            str_cpy(nm, e->npresets ? e->presets[s->preset % e->npresets].name : e->name, sizeof nm);
        else
            str_cpy(nm, "--", sizeof nm);
        cv_text(28, y, &FONT_S, nm, s->ok ? C_GRAY : C_DIM);
    }
    cv_rect(20, 62, 200, 1, C_LINE);
    cv_rect(20 + w - 1, 56, 3, 13, C_WHITE);
    cv_text(8, 56, &FONT_S, "A", C_DIM);
    cv_text(226, 56, &FONT_S, "B", C_DIM);
}
/* the graph of the MATRIX page: the six slots, source > destination and amount, the one turned lit */
static void graph_matrix(void)
{
    const track_t *t = TSEL;
    uint32_t k, sel = (uint32_t)clamp(xv[X_MSLOT], 1, 6) - 1u;
    for (k = 0; k < MX_SLOTS; k++) {
        uint32_t id = P_M1SRC + 3u * k;
        int32_t sr = clamp(t->p[id], 0, MS_NSRC - 1), ds = clamp(t->p[id + 1u], 0, MT_COUNT - 1), a = t->p[id + 2u];
        const param_desc_t *dd = track_desc(t, id + 1u);
        int on = sr && ds && a;
        uint16_t c = k == sel ? C_WHITE : on ? C_GRAY : C_DIM;
        char nb[2] = {(char)('1' + k), 0}, val[12];
        const char *unit = 0;
        int32_t y = 2 + (int32_t)k * 14;
        cv_text(8, y, &FONT_S, nb, k == sel ? C_HI : C_DIM);
        cv_text(24, y, &FONT_S, N_MSRC[sr], c);
        cv_text(84, y, &FONT_S, ">", C_DIM);
        cv_text(98, y, &FONT_S, dd->names[ds], c);
        param_format(&TP[id + 2u], a, val, &unit);
        cv_text(176, y, &FONT_S, val, c);
        if (on) {                                     /* the amount as a bar from the middle */
            int32_t w = a * 24 / 64;
            cv_rect(w < 0 ? 228 + w : 228, y + 4, w < 0 ? -w : (w ? w : 1), 3, k == sel ? C_HI : C_GRAY);
        }
        cv_rect(228, y + 2, 1, 7, C_LINE);
    }
}
/* FMX's pages: the routing as a grid (rows: TO, columns: FROM; a square's size its amount; the diagonal feedback),
 * the output column, the operators' waves and ratios; the operator / connection being edited white */
static void graph_fmx(void)
{
    const fmx_patch_t *p = &fmx_cur[song.sel % NPART];
    const fmx_eff_t *f = &fmx_eff[song.sel % NPART];
    uint32_t d, s, op = (uint32_t)clamp(xv[X_FOP], 1, 6) - 1u;
    uint32_t to = (uint32_t)clamp(xv[X_FTO], 1, 6) - 1u, fr = (uint32_t)clamp(xv[X_FFROM], 1, 6) - 1u;
    int route = cur_page()->id[0] == X_FTO;
    char b[8];
    for (s = 0; s < FMX_NOP; s++) {
        b[0] = (char)('1' + s), b[1] = 0;
        cv_text(26 + (int32_t)s * 12, 0, &FONT_S, b, route && s == fr ? C_WHITE : C_DIM);
    }
    cv_text(102, 0, &FONT_S, "OUT", C_DIM);
    for (d = 0; d < FMX_NOP; d++) {
        int32_t y = 13 + (int32_t)d * 12, a;
        const fmx_op_t *o = &p->op[d];
        uint16_t rc = (route ? d == to : d == op) ? C_WHITE : C_GRAY;
        b[0] = (char)('1' + d), b[1] = 0;
        cv_text(8, y - 1, &FONT_S, b, rc);
        for (s = 0; s < FMX_NOP; s++) {
            int32_t x = 26 + (int32_t)s * 12;
            uint32_t j, used = 0;
            a = 0;
            for (j = 0; j < f->n[d]; j++)             /* (what plays: the template if ALG is on) */
                if (f->src[d][j] == s)
                    used = 1, a = f->amt[d][j];
            cv_rect(x + 4, y + 4, 1, 1, C_LINE);
            if (used) {
                int32_t w = 2 + clamp(a, 0, 127) * 7 / 127;
                cv_rect(x + 5 - w / 2, y + 5 - w / 2, w, w, s == d ? TE_COL[1] : C_HI);
            }
            if (route && d == to && s == fr)
                cv_rect(x, y, 11, 1, C_WHITE), cv_rect(x, y + 10, 11, 1, C_WHITE), cv_rect(x, y, 1, 11, C_WHITE),
                    cv_rect(x + 10, y, 1, 11, C_WHITE);
        }
        a = (f->carriers >> d) & 1u ? f->outg[d] / 258 : 0;
        if (a)
            cv_rect(102, y + 3, 1 + a * 24 / 127, 5, route && d == to ? C_WHITE : C_HI);
        cv_text(136, y - 1, &FONT_S, N_FMX_WAVE[o->wave % FMX_NWAVE], rc);
        str_cpy(b, o->mode & FXM_FIX ? "F" : N_FRATIO[o->coarse & 31u], sizeof b);
        cv_text(186, y - 1, &FONT_S, b, rc);
        cv_text(214, y - 1, &FONT_S, N_FMX_MODE[(o->mode & 3u) % FXM_N], (o->mode & 3u) ? C_HI : C_DIM);
    }
}
static uint32_t fmx_sig(void)
{
    const uint8_t *b = (const uint8_t *)&fmx_cur[song.sel % NPART];
    uint32_t i, h = 2166136261u;
    for (i = 0; i < sizeof(fmx_patch_t); i++)
        h = (h ^ b[i]) * 16777619u;
    return h ^ (uint32_t)xv[X_FOP] * 31u ^ (uint32_t)xv[X_FTO] * 37u ^ (uint32_t)xv[X_FFROM] * 41u;
}
static uint32_t syn_sig(void)
{
    const track_t *l = &trk[1];
    return fmx_sig() + (uint32_t)l->eng_req * 131u + l->preset * 7u + l->user * 3u + (uint32_t)l->p[P_LYMODE] * 17u +
           (uint32_t)l->p[P_LYSPLIT] * 257u + (uint32_t)xv[X_MORPH] * 65537u + syn_a.ok * 11u + syn_b.ok * 13u +
           syn_a.eng * 101u + syn_b.eng * 103u + syn_a.preset * 107u + syn_b.preset * 109u + (uint32_t)xv[X_MSLOT] * 6007u;
}
