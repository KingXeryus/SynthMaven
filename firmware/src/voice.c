/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Voice allocation, envelopes, LFO and per-track rendering. Runs in the
 * audio ISR.
 *
 * Each synth part has its own NVOICE voices (so MONO / LEGATO / UNISON keep using
 * v[0..]), but only NVOICE of all the parts' voices sound at once: a part that starts
 * a voice while the budget is full takes one from any part (voice_victim): the oldest
 * released voice, else the oldest extra UNISON voice, else the oldest held voice of a
 * POLY part that is not its lowest note. A voice taken from another part fades out
 * over one block (stage 4: the envelope goes to 0, the block's amplitude ramp
 * declicks it); one of the part's own is restarted in place. Extra UNISON
 * voices only start when there is room. The drum track has its own voices (drums.c). */
static uint32_t vage;                                   /* voice ages: one clock for every part */
/* engines that play recorded material (a position, not a phase): no phases kept or spread */
#if FELUCCA_SLICE
static int eng_sampled(const engine_t *e) { return e == &ENG_SAMPLE || e == &ENG_SLICE; }
#else
static int eng_sampled(const engine_t *e) { return e == &ENG_SAMPLE; }
#endif
static int32_t lfo_wave(track_t *t, uint32_t ph)
{
    switch (t->p[P_LWAVE]) {
    case 1:
        return osc_tri(ph);
    case 2:
        return (int32_t)(ph >> 16) - 32768;
    case 3:
        return ph < 0x80000000u ? 32767 : -32767;
    case 4:
        return (int32_t)(t->lfo_rnd >> 16) - 32768;
    default:
        return osc_sine(ph);
    }
}

static void track_lfo_tick(track_t *t)
{
    uint32_t old = t->lfo_ph;
    t->lfo_ph += LFO_INC[t->p[P_LRATE] & 127];
    if (t->lfo_ph < old)
        t->lfo_rnd = rng();
    t->lfo_val = lfo_wave(t, t->lfo_ph);
    if (t->lfo_fade < 32767) {
        int32_t step = (int32_t)(ENV_LIN[t->p[P_LFADE] & 127] >> 9);
        t->lfo_fade = t->p[P_LFADE] ? clamp(t->lfo_fade + (step ? step : 1), 0, 32767) : 32767;
    }
}

/* voices the engine may use (POLY and UNISON): its cap, else all of them */
static uint32_t trk_nvoice(const track_t *t)
{
    uint32_t c = ENGINES[t->engine] == &ENG_PHYS ? phys_poly(t) : ENGINES[t->engine]->poly;   /* (PHYS: by MODEL) */
    return c && c < NPOLY ? c : NPOLY;
}

/* ------------------------------------------------- the shared voice budget --- */
static uint32_t voices_busy(void)                       /* sounding voices of all parts (not the fading ones) */
{
    uint32_t p, i, n = 0;
    for (p = 0; p < NPART; p++)
        for (i = 0; i < NVOICE; i++)
            n += trk[p].v[i].active && trk[p].v[i].stage != 4u;
    return n;
}

static uint32_t lowest_held(const track_t *t)           /* index of the lowest held note (the bass), NVOICE = none */
{
    uint32_t i, low = NVOICE;
    for (i = 0; i < NVOICE; i++)
        if (t->v[i].active && t->v[i].gate && (low == NVOICE || t->v[i].note < t->v[low].note))
            low = i;
    return low;
}

/* the voice to give up for a new one (see the top); soft: only released voices and
 * extra UNISON voices of other parts. Returns its index, *pp its part; NVOICE = none */
static uint32_t voice_victim(const track_t *self, int soft, track_t **pp)
{
    uint32_t p, i, best = NVOICE, cat = 4;
    for (p = 0; p < NPART; p++) {
        track_t *t = &trk[p];
        uint32_t mode = (uint32_t)t->p[P_VOICE], nu = mode == V_UNISON ? trk_nvoice(t) : 1u;
        uint32_t low = voice_poly((int)mode) ? lowest_held(t) : NVOICE;
        if (soft && t == self)
            continue;
        for (i = 0; i < NVOICE; i++) {
            const voice_t *v = &t->v[i];
            uint32_t c;
            if (!v->active || v->stage == 4u)
                continue;
            if (!v->gate)
                c = 1;                                  /* released */
            else if (mode == V_UNISON && i > 0 && i < nu)
                c = 2;                                  /* an extra UNISON voice */
            else if (!soft && voice_poly((int)mode) && i != low)
                c = 3;                                  /* held, not the bass */
            else
                continue;
            if (c < cat || (c == cat && v->age < (*pp)->v[best].age)) {
                cat = c;
                best = i;
                *pp = t;
            }
        }
    }
    return best;
}

static uint32_t voice_kills;                            /* voices given up (budget, overload): console, hostsim */
#define KILL_BLOCKS 2u                                  /* 1.5 ms: no click on a bass, little extra load */
static void voice_kill(voice_t *v)                      /* fade out over the next blocks (env_tick stage 4) */
{
    v->gate = 0;
    if (v->stage != 4u)
        v->kill = KILL_BLOCKS;
    v->stage = 4;
    voice_kills++;
}

/* overload (after Felucca 1.0; audio.c asks for it when two halves in a row took > 85 % of their time):
 * fade one voice out over the next blocks (voice_kill: no click), one more each half while it lasts. A
 * single late half (a USB burst, a flash write) sheds nothing. First the quietest releasing voice; else
 * the oldest held one that is not a part's lowest POLY note nor its MONO / LEGATO / UNISON lead, as the
 * voice budget keeps them: a dense chord on a heavy engine thins out from the top, bass and lead stay. */
static uint32_t shed_count;

static void shed_voice(void)
{
    uint32_t p, i;
    voice_t *best = 0;
    for (p = 0; p < NPART; p++)
        for (i = 0; i < NVOICE; i++) {
            voice_t *v = &trk[p].v[i];
            if (v->active && !v->gate && v->stage != 4u && (!best || v->env < best->env))
                best = v;
        }
    if (best) {
        voice_kill(best);
        shed_count++;
        return;
    }
    for (p = 0; p < NPART; p++) {
        const track_t *t = &trk[p];
        uint32_t mode = (uint32_t)t->p[P_VOICE], low = voice_poly((int)mode) ? lowest_held(t) : 0u;
        for (i = 0; i < NVOICE; i++) {
            voice_t *v = &trk[p].v[i];
            if (v->active && v->gate && v->stage != 4u &&
                ((voice_poly((int)mode) && i != low) || (!voice_poly((int)mode) && i > 0u)) && (!best || v->age < best->age))
                best = v;
        }
    }
    if (best) {
        voice_kill(best);
        shed_count++;
    }
}

/* the budget is full: free a voice for part t. 0 = nothing to take (soft) */
static int voice_room(track_t *t, int soft)
{
    track_t *vp = 0;
    uint32_t k;
    if (voices_busy() < NVOICE)
        return 1;
    k = voice_victim(t, soft, &vp);
    if (k == NVOICE)
        return !soft;                                   /* hard: over the budget (cannot happen with 3 parts) */
    voice_kill(&vp->v[k]);
    return 1;
}

/* POLY allocation (within the engine's voice cap). Same note: reuse its voice. Else a free voice: ROTATE takes
 * the next one round-robin (release tails ring out), REUSE (or any GLIDE) takes
 * the free voice whose last pitch is closest, so poly portamento moves each
 * voice the shortest way. Steal: the oldest released voice, else the oldest
 * held one, never the lowest held note (the bass). A free voice needs room in the
 * shared budget: when the voice to give up is one of this part's, it is restarted
 * in place instead. */
static voice_t *voice_alloc(track_t *t, uint32_t note)
{
    uint32_t i, np = trk_nvoice(t), best = np, low = np, nfree = 0;
    int32_t bd = 0x7FFFFFFF;
    for (i = 0; i < np; i++) {
        if (t->v[i].active && t->v[i].note == note)
            return &t->v[i];
        nfree += !t->v[i].active;
    }
    if (nfree && voices_busy() >= NVOICE) {
        track_t *vp = 0;
        uint32_t k = voice_victim(t, 0, &vp);
        if (k < np && vp == t)
            return &t->v[k];                            /* our own: restart it in place (no click) */
        if (k < NVOICE)
            voice_kill(&vp->v[k]);
    }
    if (t->p[P_ALLOC] || t->p[P_GLIDE]) {
        for (i = 0; i < np; i++) {
            int32_t d = t->v[i].pitch_cur - (int32_t)note * 16;
            if (t->v[i].active)
                continue;
            d = d < 0 ? -d : d;
            if (!t->v[i].pitch_cur)
                d = 0x7FFFFFF0;                          /* never played: last choice */
            if (d < bd) {
                bd = d;
                best = i;
            }
        }
    } else {
        for (i = 0; i < np && best == np; i++) {
            uint32_t k = (t->rr + i) % np;
            if (!t->v[k].active)
                best = k;
        }
        t->rr = (uint8_t)((best == np ? t->rr : best) + 1u) % np;
    }
    if (best < np)
        return &t->v[best];
    for (i = 0; i < np; i++)                             /* the lowest held note is protected */
        if (t->v[i].gate && (low == np || t->v[i].note < t->v[low].note))
            low = i;
    for (i = 0; i < np; i++)                             /* oldest released (not one fading out for another part) */
        if (!t->v[i].gate && t->v[i].stage != 4u && (best == np || t->v[i].age < t->v[best].age))
            best = i;
    if (best == np)
        for (i = 0; i < np; i++)                         /* oldest held, not the bass */
            if (i != low && (best == np || t->v[i].age < t->v[best].age))
                best = i;
    return &t->v[best == np ? 0 : best];
}

/* glide, in 1/4096 semitone a control tick (pitch_cur . pitch_frac): RATE = GLIDE is the time an octave
 * takes (any interval at that speed), TIME = GLIDE is the time of any glide. A sequencer SLIDE with
 * GLIDE 0 glides in SLIDE_T (TIME), as an acid slide */
#define SLIDE_T 54                                       /* TIME_MS index: ~50 ms */
static int32_t glide_ticks(int32_t g)                    /* control ticks of TIME_MS index g */
{
    int32_t k = (int32_t)ENV_LIN[g & 127];
    return k ? (int32_t)((1u << 24) / (uint32_t)k) : 1;
}
static void glide_set(track_t *t, voice_t *v, int glide)
{
    int32_t d, ticks;
    if (!glide || !v->pitch_cur) {
        v->pitch_cur = v->pitch16;
        v->pitch_frac = 0;
        v->gstep = 0;
        return;
    }
    if (t->p[P_GLMODE] || !t->p[P_GLIDE]) {
        d = (v->pitch16 << 8) - ((v->pitch_cur << 8) + v->pitch_frac);
        d = d < 0 ? -d : d;
        ticks = glide_ticks(t->p[P_GLIDE] ? t->p[P_GLIDE] : SLIDE_T);
        v->gstep = d / ticks > 0 ? d / ticks : 1;
    } else {
        v->gstep = 0;
    }
}

static void voice_start(track_t *t, voice_t *v, uint32_t note, uint32_t vel, int glide)
{
    const engine_t *e = ENGINES[t->engine];
    int sounding = v->active && v->stage != 0;
    uint32_t ph0 = v->ph[0], ph1 = v->ph[1], ph2 = v->ph[2];
    int32_t s0 = v->s[0], s1 = v->s[1], s4 = v->s[4], s5 = v->s[5], s6 = v->s[6], s7 = v->s[7];
    v->note = (uint8_t)note;
    v->vel = (uint8_t)vel;
    v->gate = 1;
    v->sus = 0;
    v->pat = 0;
    v->active = 1;
    v->stage = 1;
    v->age = ++vage;
    v->pitch16 = (int32_t)note * 16;
    glide_set(t, v, glide);
    v->penv = 32767;                                    /* the pitch envelope starts over (not on legato) */
    if (!sounding) {
        v->env = 0;
        v->env_out = 0;
    }                                                   /* sounding: the attack starts from the current level */
    e->note_on(t, v);
    if (sounding && !eng_sampled(e)) {                 /* retrigger / steal: keep phases and filter states */
        v->ph[0] = ph0;                                 /* (resetting them clicks) */
        v->ph[1] = ph1;
        v->ph[2] = ph2;
        if (e == &ENG_ANALOG) {
            v->s[0] = s0;
            v->s[1] = s1;
        } else if (e == &ENG_DIGITAL) {
            v->s[5] = s5;
            v->s[6] = s6;
            v->s[7] = s7;                               /* op 4 phase; the modulator envelope restarts */
        } else if (e == &ENG_LOFI) {
            v->s[0] = s0;
            v->s[4] = s4;
        } else if (e == &ENG_TRIO) {
            v->s[0] = s0;                               /* filter */
            v->s[1] = s1;
            v->s[4] = s4;                               /* the sample waiting for its step corrections */
        } else if (e == &ENG_PHASE) {
            v->s[0] = s0;                               /* the WAVE / WAVE2 toggles go with the phases kept */
            v->s[1] = s1;
        } else if (e == &ENG_NOISE) {
            v->s[0] = s0;                               /* the filter and the colour poles (2.5) */
            v->s[1] = s1;
            v->s[4] = s4;
            v->s[5] = s5;
            v->s[6] = s6;
            v->s[7] = s7;
        }
    }
}

/* MONO / LEGATO / UNISON: one note on one voice (eight for UNISON, or the
 * engine's voice cap, spread by DETUNE over the same width) */
static void mono_play(track_t *t, uint32_t note, uint32_t vel, int retrig, int glide)
{
    uint32_t mode = (uint32_t)t->p[P_VOICE], nv = mode == V_UNISON ? trk_nvoice(t) : 1u, i;
    for (i = 0; i < nv; i++) {
        voice_t *v = &t->v[i];
        int32_t k = 2 * (int32_t)i - (int32_t)(nv - 1u);   /* -7 .. 7 */
        if (nv > 1u && nv < NVOICE)
            k = k * (NVOICE - 1) / (int32_t)(nv - 1u);      /* fewer voices: the outer ones as wide */
        v->fine = nv > 1u ? k * t->p[P_DETUNE] * 56 / 889 : 0;   /* up to ~±40 cents */
        if (retrig || !v->active || !v->gate) {
            int was_sounding = v->active && v->stage != 0;
            if ((!v->active || v->stage == 4u) && !voice_room(t, i > 0))
                continue;                                   /* no room for this extra UNISON voice */
            /* level: about the same sum for 8 or 4 voices at random phases */
            voice_start(t, v, note, nv >= NVOICE ? vel * 36u / 100u : nv > 1u ? vel / 2u : vel, glide);
            if (nv > 1u && i && !eng_sampled(ENGINES[t->engine]) && !was_sounding) {   /* random start phases: */
                static uint32_t seed = 0x1234567u;          /* in phase they stack, evenly spread they cancel */
                seed = seed * 1664525u + 1013904223u;
                v->ph[0] += seed;
            }
        } else {                                            /* legato: new pitch, same envelope */
            v->note = (uint8_t)note;
            v->pitch16 = (int32_t)note * 16;
            if (vel > v->vel && nv == 1u)
                v->vel = (uint8_t)vel;
            glide_set(t, v, glide);
        }
    }
    t->mono_note = (uint8_t)note;
}

/* the note the mono stack asks for (PRIO: LAST / LOW / HIGH), 0 = none */
static uint32_t mono_pick(const track_t *t)
{
    uint32_t i, n;
    if (!t->nmono)
        return 0;
    n = t->mono_stack[t->nmono - 1u];
    for (i = 0; i < t->nmono; i++) {
        if (t->p[P_PRIO] == 1 && t->mono_stack[i] < n)
            n = t->mono_stack[i];
        if (t->p[P_PRIO] == 2 && t->mono_stack[i] > n)
            n = t->mono_stack[i];
    }
    return n;
}

static void mono_remove(track_t *t, uint32_t note)
{
    uint32_t j, k = 0;
    for (j = 0; j < t->nmono; j++)
        if (t->mono_stack[j] != note)
            t->mono_stack[k++] = t->mono_stack[j];
    t->nmono = (uint8_t)k;
}

#if CLIPPER_SYNTH
/* Clipper 0.3: poly unison (VOICE UNI2 / UNI4): each note on 2 or 4 voices spread by DETUNE as UNISON's (the
 * outer ones as wide), random start phases. The voice for one more: a free one while the budget has room, else
 * this part's oldest released, else a released or extra voice of another part, else this part's oldest held that is
 * not its lowest note (the bass). The same note again restarts its own voices in place */
static int puni_taken(const uint32_t *idx, uint32_t n, uint32_t i)
{
    uint32_t j;
    for (j = 0; j < n; j++)
        if (idx[j] == i)
            return 1;
    return 0;
}
static uint32_t puni_pick(track_t *t, const uint32_t *idx, uint32_t n, uint32_t np)
{
    uint32_t i, best = np, low = lowest_held(t);
    if (voices_busy() < NVOICE)
        for (i = 0; i < np; i++)
            if (!t->v[i].active && !puni_taken(idx, n, i))
                return i;
    for (i = 0; i < np; i++)
        if (t->v[i].active && !t->v[i].gate && t->v[i].stage != 4u && !puni_taken(idx, n, i) &&
            (best == np || t->v[i].age < t->v[best].age))
            best = i;
    if (best < np)
        return best;
    if (voice_room(t, 1))                                /* another part gives one up (soft) */
        for (i = 0; i < np; i++)
            if (!t->v[i].active && !puni_taken(idx, n, i))
                return i;
    for (i = 0; i < np; i++)
        if (t->v[i].gate && !puni_taken(idx, n, i) && (low >= NVOICE || t->v[i].note != t->v[low].note) &&
            (best == np || t->v[i].age < t->v[best].age))
            best = i;
    return best;
}
static void puni_on(track_t *t, uint32_t note, uint32_t vel)
{
    static uint32_t seed = 0x2468ACEu;
    uint32_t np = trk_nvoice(t), k = t->p[P_VOICE] == V_PUNI2 ? 2u : 4u, idx[4], n = 0, i, j;
    if (k > np)
        k = np;
    for (i = 0; i < np && n < k; i++)                  /* the same note: its voices again */
        if (t->v[i].active && t->v[i].note == note && t->v[i].stage != 4u)
            idx[n++] = i;
    while (n < k) {
        i = puni_pick(t, idx, n, np);
        if (i >= np)
            break;
        idx[n++] = i;
    }
    for (j = 0; j < n; j++) {
        voice_t *v = &t->v[idx[j]];
        int was = v->active && v->stage != 0;
        int32_t s = n > 1u ? (2 * (int32_t)j - (int32_t)(n - 1u)) * 7 : 0;   /* -7 .. 7 (x (n - 1)) */
        v->fine = n > 1u ? s * t->p[P_DETUNE] * 56 / (889 * (int32_t)(n - 1u)) : 0;
        voice_start(t, v, note, n >= 4u ? vel / 2u : n > 1u ? vel * 70u / 100u : vel, t->p[P_GLIDE] != 0);
        if (j && !was && !eng_sampled(ENGINES[t->engine])) {   /* random start phases (in phase they stack) */
            seed = seed * 1664525u + 1013904223u;
            v->ph[0] += seed;
        }
    }
}
#endif

/* the notes each synth part started since the UI last looked (menu NOTES: a short note lights its key
 * for a few frames, as a drum hit does; ui_studio.c pads_tick). Set here (mostly the audio ISR), taken
 * by the UI with the IRQ off */
static volatile uint32_t note_hits[NPART][4];
/* the visualiser (ui_vis.c): the tracks a note started on (bit per track), each one's last note, a kick */
static volatile uint8_t vis_hit, vis_kick_hit, vis_note[NTRK];

static void trk_note_on1(track_t *t, uint32_t note, uint32_t vel)
{
    uint32_t any = 0, i, mode = (uint32_t)t->p[P_VOICE];
    if (trk_silent(t))
        return;                                         /* MUTE, or another track soloed */
    vis_hit |= (uint8_t)(1u << ((uint32_t)(t - trk) & 3u));
    vis_note[(uint32_t)(t - trk) & 3u] = (uint8_t)note;
    if (is_drum(t)) {                                   /* the drum track: GM drums (drums.c) */
        if (note == 35u || note == 36u)
            vis_kick_hit = 1;
        drum_on(note, vel);
        return;
    }
    if ((uint32_t)(t - trk) < NPART && note < 128u)
        note_hits[t - trk][note >> 5] |= 1u << (note & 31u);
    if (t->xf_on || t->eng_req != t->engine) {          /* engine switch under way: after the fade */
        for (i = 0; i < t->xp_n && t->xp_note[i] != note; i++)
            ;
        if (i == t->xp_n && t->xp_n < 4u)
            t->xp_n++;
        i = i < t->xp_n ? i : t->xp_n - 1u;             /* (full: the last one is replaced) */
        t->xp_note[i] = (uint8_t)note;
        t->xp_vel[i] = (uint8_t)vel;
        return;
    }
    for (i = 0; i < NVOICE; i++)
        any |= t->v[i].gate;
    if (!any) {                                        /* fresh phrase: LFO retrigger and fade */
        t->lfo_ph = (uint32_t)t->p[P_LPHASE] << 25;
        t->lfo_fade = 0;
    }
    if (voice_poly((int)mode)) {
        voice_t *v;
        t->nmono = 0;                                   /* no stale mono stack after a mode change */
        t->mono_note = 0;
#if CLIPPER_SYNTH
        if (mode >= V_PUNI2) {                          /* Clipper 0.3: poly unison */
            puni_on(t, note, vel);
            return;
        }
#endif
        v = voice_alloc(t, note);
        v->fine = 0;
        voice_start(t, v, note, vel, t->p[P_GLIDE] != 0);
        return;
    }
    {
        uint32_t sounding = t->mono_note && t->v[0].active && t->v[0].gate, want;
        mono_remove(t, note);
        if (t->nmono >= 8u)
            mono_remove(t, t->mono_stack[0]);
        t->mono_stack[t->nmono++] = (uint8_t)note;
        want = mono_pick(t);
        if (sounding && want == t->mono_note)
            return;                                     /* LOW / HIGH priority: this key does not win */
        if (mode == V_MONO)                             /* MONO: always retrigger, glide when GLIDE is set */
            mono_play(t, want, vel, 1, t->p[P_GLIDE] != 0 || t->slide_glide);
        else                                            /* LEGATO / UNISON: glide only between held notes */
            mono_play(t, want, vel, !sounding, sounding && (t->p[P_GLIDE] != 0 || t->slide_glide));
    }
}

/* STRUM (2.4, P_STRUM): a chord's notes start one after the other, |P_STRUM| ms apart (> 0 from the lowest up, as
 * a guitar's down stroke; < 0 from the highest down). The later ones wait here, in samples; a note-off before its
 * start cancels it, a track's all-off (STOP) clears them */
#define STQ 16u
static struct { uint8_t on, trk, note, vel; uint32_t left; } stq[STQ];
static void strum_cancel(const track_t *t, uint32_t note, int all)
{
    uint32_t i;
    for (i = 0; i < STQ; i++)
        if (stq[i].on && stq[i].trk == (uint8_t)(t - trk) && (all || stq[i].note == note))
            stq[i].on = 0;
}
static void trk_note_off1(track_t *t, uint32_t note)
{
    uint32_t i, k = 0, mode = (uint32_t)t->p[P_VOICE];
    if (is_drum(t))
        return;                                         /* one-shots */
    strum_cancel(t, note, 0);
    for (i = 0; i < t->xp_n; i++)                       /* not sounding yet (engine switch): forget it */
        if (t->xp_note[i] != note) {
            t->xp_note[k] = t->xp_note[i];
            t->xp_vel[k++] = t->xp_vel[i];
        }
    t->xp_n = (uint8_t)k;
    mono_remove(t, note);                               /* in any mode: a key let go after a VOICE change must
                                                         * not stay in the MONO stack (after Felucca 1.0) */
    if (!voice_poly((int)mode)) {
        uint32_t nv = mode == V_UNISON ? trk_nvoice(t) : 1u;
        if (t->mono_note == note) {
            uint32_t next = mono_pick(t);
            if (next) {                                 /* fall back to a held note, legato */
                mono_play(t, next, t->v[0].vel, 0, t->p[P_GLIDE] != 0);
            } else {
                for (i = 0; i < nv; i++)
                    if (t->v[i].gate) {
                        t->v[i].gate = 0;
                        t->v[i].stage = 3;
                    }
                t->mono_note = 0;
            }
        }
    }
    for (i = 0; i < NVOICE; i++)                        /* POLY voices (also ones left from another mode: */
        if (t->v[i].note == note && t->v[i].gate &&     /* VOICE changed while they were held) */
            (voice_poly((int)mode) || i >= (mode == V_UNISON ? trk_nvoice(t) : 1u) || t->mono_note != note)) {
            if (voice_poly((int)mode) && part_ctl[CLIPPER_SYNTH ? 0u : (uint32_t)(t - trk) % NPART].sus) {
                t->v[i].sus = 1;                        /* Clipper: the sustain pedal holds it */
                continue;
            }
            t->v[i].gate = 0;
            t->v[i].stage = 3;
        }
}

/* Clipper: the sustain pedal comes up: the notes it held are released (a key still down keeps its note:
 * pressed again, its voice started over and lost the mark) */
static void trk_sustain_up1(track_t *t)
{
    uint32_t i;
    for (i = 0; i < NVOICE; i++)
        if (t->v[i].sus) {
            t->v[i].sus = 0;
            if (t->v[i].gate) {
                t->v[i].gate = 0;
                t->v[i].stage = 3;
            }
        }
}

static void trk_all_off1(track_t *t)
{
    uint32_t i;
    strum_cancel(t, 0, 1);
    for (i = 0; i < NVOICE; i++) {
        t->v[i].gate = 0;
        t->v[i].sus = 0;
        t->v[i].stage = t->v[i].active ? 3 : 0;
    }
    t->nmono = 0;
    t->mono_note = 0;
    t->xp_n = 0;
}

/* Clipper 0.3: the layer. Track 2's sound with track 1's (LAYER: every note on both; SPLIT: below SPLIT on track
 * 2, from it up on track 1), its octave OCT; what a note-on did is kept per note, so its note-off goes where it
 * went whatever changed meanwhile (the mode, the split, the octave). Track 2 has no other way in: its sequencer and
 * arpeggiator never run, the keys, MIDI and track 1's sequencer and arpeggiator all come through here */
#if CLIPPER_SYNTH
static volatile int16_t syn_morph_req = -1;      /* MIDI CC 20 (seq.c): MORPH, applied by the UI (ui_synth.c) */
static uint8_t ly_route[128], ly_note[128];           /* per note: bit 0 track 1, bit 1 track 2; track 2's note */
static uint32_t layer_mode(void)
{
    int32_t m = trk[1].p[P_LYMODE];
    return m == LY_LAYER || m == LY_SPLIT ? (uint32_t)m : LY_OFF;
}
static void trk_note_on(track_t *t, uint32_t note, uint32_t vel)
{
    uint32_t m, r = 1;
    if (t != &trk[0] || note > 127u) {
        trk_note_on1(t, note, vel);
        return;
    }
    m = layer_mode();
    if (m == LY_LAYER)
        r = 3;
    else if (m == LY_SPLIT)
        r = (int32_t)note >= trk[1].p[P_LYSPLIT] - 4 ? 1u : 2u;
    if (ly_route[note] & 2u)                            /* (the same note again: its layer note first let go) */
        trk_note_off1(&trk[1], ly_note[note]);
    ly_route[note] = (uint8_t)r;
    if (r & 2u) {
        ly_note[note] = (uint8_t)clamp((int32_t)note + (trk[1].p[P_LYOCT] - 2) * 12, 0, 127);
        trk_note_on1(&trk[1], ly_note[note], vel);
    }
    if (r & 1u)
        trk_note_on1(t, note, vel);
}
static void trk_note_off(track_t *t, uint32_t note)
{
    uint32_t r;
    if (t != &trk[0] || note > 127u) {
        trk_note_off1(t, note);
        return;
    }
    r = ly_route[note] ? ly_route[note] : 1u;
    ly_route[note] = 0;
    if (r & 2u)
        trk_note_off1(&trk[1], ly_note[note]);
    if (r & 1u)
        trk_note_off1(t, note);
}
static void trk_sustain_up(track_t *t)
{
    trk_sustain_up1(t);
    if (t == &trk[0])
        trk_sustain_up1(&trk[1]);
}
static void trk_all_off(track_t *t)
{
    trk_all_off1(t);
    if (t == &trk[0]) {
        trk_all_off1(&trk[1]);
        memset(ly_route, 0, sizeof ly_route);
    }
}
#else
#define trk_note_on trk_note_on1
#define trk_note_off trk_note_off1
#define trk_sustain_up trk_sustain_up1
#define trk_all_off trk_all_off1
#endif

/* note i of a chord of n (notes[]): at once, or strummed (rank: its place from the lowest, or the highest) */
static void trk_note_chord(track_t *t, const uint8_t *notes, uint32_t n, uint32_t i, uint32_t vel)
{
    int32_t s = t->p[P_STRUM];
    uint32_t rank = 0, j, q;
    if (!s || n < 2u || is_drum(t)) {
        trk_note_on(t, notes[i], vel);
        return;
    }
    for (j = 0; j < n; j++)                             /* its place: lowest first (down), highest first (up) */
        if (j != i && (s > 0 ? notes[j] < notes[i] || (notes[j] == notes[i] && j < i)
                             : notes[j] > notes[i] || (notes[j] == notes[i] && j < i)))
            rank++;
    if (!rank) {
        trk_note_on(t, notes[i], vel);
        return;
    }
    strum_cancel(t, notes[i], 0);
    for (q = 0; q < STQ && stq[q].on; q++)
        ;
    if (q == STQ) {                                     /* (full: at once) */
        trk_note_on(t, notes[i], vel);
        return;
    }
    stq[q].trk = (uint8_t)(t - trk);
    stq[q].note = (uint8_t)notes[i];
    stq[q].vel = (uint8_t)vel;
    stq[q].left = rank * (uint32_t)(s < 0 ? -s : s) * (uint32_t)FS / 1000u;
    stq[q].on = 1;
}
/* each audio block: the strummed notes due */
static void strum_block(uint32_t n)
{
    uint32_t i;
    for (i = 0; i < STQ; i++)
        if (stq[i].on) {
            if (stq[i].left <= n) {
                stq[i].on = 0;
                trk_note_on(&trk[stq[i].trk % NTRK], stq[i].note, stq[i].vel);
            } else {
                stq[i].left -= n;
            }
        }
}

/* engine switch, at each block start (events_block), before any note of the block. The UI writes
 * eng_req and the new engine's P_E0..P_E7 together (IRQ off). The part's sounding voices (released by
 * the preset change) then fade out over XF_BLOCKS blocks on the old engine, rendered with its own
 * parameters (pe_old: an engine never reads another engine's values, which index its tables); only
 * then the engine switches and the notes that came during the fade start on it. A part with nothing
 * sounding switches at once. */
#if CLIPPER_SYNTH
#define XF_BLOCKS 8u                                    /* Clipper: 8 x 32 samples, 5.8 ms (a bass fades without a thump) */
#else
#define XF_BLOCKS 4u                                    /* 4 x 32 samples: 2.9 ms */
#endif
static void engine_block(track_t *t)
{
    uint32_t i, any = 0;
    if (t->xf_on || t->eng_req != t->engine) {
        if (!t->xf_on) {
            for (i = 0; i < NVOICE; i++)
                any |= t->v[i].active;
            if (any) {
                t->xf_on = 1;
                t->xf = XF_BLOCKS;
                return;
            }
        } else if (t->xf) {
            return;                                     /* still fading (track_render counts down) */
        }
        for (i = 0; i < NVOICE; i++) {                  /* faded to 0: gone */
            voice_t *v = &t->v[i];
            v->active = v->gate = 0;
            v->stage = 0;
            v->env = v->env_out = 0;
        }
        t->engine = t->eng_req % NENGINES;
        t->xf_on = 0;
#if CLIPPER_SYNTH
        t->xf_snd = 0;
#endif
        t->nmono = 0;
        t->mono_note = 0;
        {
            uint32_t n = t->xp_n;
            t->xp_n = 0;
            for (i = 0; i < n; i++)
                trk_note_on1(t, t->xp_note[i], t->xp_vel[i]);   /* (the layer had it then) */
        }
    }
    for (i = 0; i < 8u; i++)                            /* the engine's own values, for a later fade */
        t->pe_old[i] = t->p[P_E0 + i];
}

#if CLIPPER_SYNTH
/* Clipper 0.3: before a sound changes under sounding voices (a preset, an engine, a user preset, SWAP, BACK...):
 * they fade out over XF_BLOCKS with all their own values (p_old: fx.c mix_part renders the part with them; pe_old the
 * engine's), then go; notes played meanwhile wait for the new sound (engine_block). Without it the voices ringing
 * on took the new values at once (a new trim, filter, envelope, table) and clicked. The main loop / UI calls it */
static void sound_fade_begin(track_t *t)
{
    uint32_t i, any = 0;
    if ((uint32_t)(t - trk) >= NPART || t->xf_on)
        return;
    for (i = 0; i < NVOICE; i++)
        any |= t->v[i].active && t->v[i].stage != 0;
    if (!any)
        return;
    memcpy(t->p_old, t->p, sizeof t->p_old);
    t->xf_snd = 1;
    t->xf = XF_BLOCKS;
    RING_PUBLISH();
    t->xf_on = 1;                                       /* (last: the ISR uses p_old from then on) */
}
#else
#define sound_fade_begin(t) ((void)0)
#endif

/* one control tick (CTL samples) of the amplitude envelope; returns Q15 */
static int32_t env_tick(track_t *t, voice_t *v)
{
    const int16_t *p = t->p;
    int32_t sus = (int32_t)p[P_SUS] << 17;              /* Q24 */
    switch (v->stage) {
    case 1:
        v->env += (int32_t)ENV_LIN[p[P_ATK] & 127];
        if (v->env >= (1 << 24)) {
            v->env = 1 << 24;
            v->stage = 2;
        }
        break;
    case 2:
        v->env += mulq16(sus - v->env, ENV_EXP[p[P_DEC] & 127]);
        break;
    case 3:
        v->env -= mulq16(v->env, ENV_EXP[p[P_REL] & 127]);
        if (v->env < (1 << 12)) {
            v->env = 0;
            v->stage = 0;
            v->active = 0;
        }
        break;
    case 4:                                             /* given up for another part: faded out over KILL_BLOCKS */
        if (v->kill > 1u) {
            v->env -= v->env / v->kill;
            v->kill--;
        } else {
            v->env = 0;
            v->stage = 0;
            v->active = 0;
            v->kill = 0;
        }
        break;
    default:
        v->env = 0;
        break;
    }
    return v->env >> 9;
}

/* render one block of a part into out (cleared here); returns the voices rendered */
static uint32_t track_render(track_t *t, int32_t *out, uint32_t n)
{
    const engine_t *e = ENGINES[t->engine];
    const int16_t *p = t->p;
    uint32_t i;
    int32_t lfo = mulq15(t->lfo_val, t->lfo_fade);
    /* Clipper: the pitch bend (MIDI En), in 1/4096 semitone: the HYBRID patch's range, else +-2 semitones */
    uint32_t pc = CLIPPER_SYNTH ? 0u : (uint32_t)(t - trk) % NPART;   /* (Clipper: the layer plays part 1's controls) */
    int32_t bend = (uint32_t)(t - trk) < NPART ?
                   (part_ctl[pc].bend * (e == &ENG_HYBRID ? (int32_t)hy_cur[t - trk].bend : 2)) >> 3 : 0;
#if CLIPPER_SYNTH
    /* Clipper 0.3: the mod wheel's and the aftertouch's destinations (P_MWDST, P_ATDST), velocity to the cutoff
     * (P_VELF); VIB and TREM ride a vibrato LFO of their own (~5.5 Hz sine), WAH the sound's LFO */
    static uint32_t vib_ph[NPART];
    int32_t vib, mw = part_ctl[pc].mw, cat = part_ctl[pc].at;
    if (e == &ENG_HYBRID)                               /* (HYBRID: its matrix takes MW and AT) */
        mw = cat = 0;
    vib_ph[(uint32_t)(t - trk) % NPART] += 535642u * n;
    vib = osc_sine(vib_ph[(uint32_t)(t - trk) % NPART]);
#endif
    /* TUNE in cents: whole 1/16 semitones in the pitch, the rest as a fine factor (no dead zone) */
    int32_t tune = song.g[G_TUNE] >= 0 ? song.g[G_TUNE] * 16 / 100 : -((-song.g[G_TUNE] * 16 + 99) / 100);
    int32_t tune_fine = (song.g[G_TUNE] * 16 - tune * 100) * 2367 / 16000;   /* rest, in 1/4096 (1 ct = 2.367) */
    uint32_t nr = 0, fade = t->xf_on && t->xf;
    int16_t pe_new[8];
    for (i = 0; i < n; i++)
        out[i] = 0;
    if (fade)                                           /* engine switch: the old engine, its own values */
        for (i = 0; i < 8u; i++) {
            pe_new[i] = t->p[P_E0 + i];
            t->p[P_E0 + i] = t->pe_old[i];
        }
    track_lfo_tick(t);
    if (e->block)                                       /* the engine's per-part work (DRAWBAR: bars, rotor) */
        e->block(t);
    for (i = 0; i < NVOICE; i++) {
        voice_t *v = &t->v[i];
        vmod_t m;
        int32_t env, pitch;
        if (!v->active)
            continue;
        {
            env = env_tick(t, v);
            if (e->ownenv && v->active) {               /* the engine's envelopes (FM6): they end the voice */
                if (e->done(t, v)) {
                    v->active = v->gate = 0;
                    v->stage = 0;
                    v->kill = 0;
                    v->env = v->env_out = 0;
                    continue;
                }
                if (v->stage != 4u) {                   /* (the ADSR's release never ends it; a voice given up for
                                                         * another part still fades out: env_tick stage 4) */
                    v->env = 1 << 24;
                    env = 32767;
                }
            }
            if (e->amp)                                 /* the engine's own amplitude curve */
                env = e->amp(t, v, env);
            m.envq15 = e->ownenv ? 0 : env;
            m.amp1 = e->ownenv ? env : mulq15(env, v->vel * 258);
            if (p[P_LD_AMP])
                m.amp1 = mulq15(m.amp1, 32767 - mulq15((lfo + 32768) >> 1, p[P_LD_AMP] * 258));
#if CLIPPER_SYNTH
            {   /* TREM: up to 60 % of the level away, at the vibrato's rate */
                int32_t tr = (p[P_MWDST] == MD_TREM ? mw : 0) + (p[P_ATDST] == MD_TREM ? (e == &ENG_HYBRID ? 0 : v->pat > cat ? v->pat : cat) : 0);
                if (tr)
                    m.amp1 = mulq15(m.amp1, 32767 - mulq15((vib + 32768) >> 1, (tr > 127 ? 127 : tr) * 155));
            }
#endif
            if (fade)                                   /* linear to 0 over the fade */
                m.amp1 = m.amp1 * (int32_t)(t->xf - 1u) / (int32_t)XF_BLOCKS;
            m.amp0 = v->env_out;
            v->env_out = m.amp1;
        }
        if (v->pitch_cur != v->pitch16 || v->pitch_frac) {   /* glide, in 1/4096 semitone */
            int32_t q = (v->pitch_cur << 8) + v->pitch_frac, d = (v->pitch16 << 8) - q;
            int32_t st = v->gstep ? v->gstep : (int32_t)((ENV_LIN[p[P_GLIDE] & 127] * 3u) >> 10);   /* RATE: an octave in GLIDE */
            if (st < 1)
                st = 1;
            q += d > 0 ? (d < st ? d : st) : (-d < st ? d : -st);
            v->pitch_cur = q >> 8;
            v->pitch_frac = (uint8_t)(q & 255);
        }
        if (v->penv)                                    /* the pitch envelope: ~40 ms */
            v->penv = (int16_t)(v->penv - ((v->penv * 590 + 32767) >> 15));
        if (!env && !m.amp0 && v->stage == 2 && !eng_sampled(e))
            continue;                                   /* held at a silent sustain (SUS 0): nothing to render */
        {   /* the pitch in 1/4096 semitone: glide, LFO, the pitch envelope; the fraction goes into the increment */
            int32_t q = (v->pitch_cur << 8) + v->pitch_frac + ((lfo * p[P_LD_PIT] * 3) >> 7) +
                        ((v->penv * p[P_ED_PIT] * 3) >> 7) + bend;
#if CLIPPER_SYNTH
            {   /* VIB: up to +-half a semitone */
                int32_t vd = (p[P_MWDST] == MD_VIB ? mw : 0) + (p[P_ATDST] == MD_VIB ? (e == &ENG_HYBRID ? 0 : v->pat > cat ? v->pat : cat) : 0);
                if (vd)
                    q += (vib * (vd > 127 ? 127 : vd)) >> 11;
            }
#endif
            pitch = (q >> 8) + tune;
            m.pitch16 = clamp(pitch, 0, 2047);
            m.inc = pitch_inc(m.pitch16);
            q = (q & 255) * 3792 >> 16;                 /* the fraction, as fine (1/16 st = 14.8) */
            m.fine = v->fine + tune_fine + q;
            if (m.fine)                                 /* unison detune, fine tune and the fraction */
                m.inc += (uint32_t)((int32_t)(m.inc >> 12) * m.fine);
        }
        m.cutoff = ((lfo * p[P_LD_FLT]) >> 7) + ((m.envq15 * p[P_ED_FLT]) >> 7);
        if (v->vel > 110)                               /* accent opens the filter with the env */
            m.cutoff += (m.envq15 * 24) >> 7;
        m.shape = (64 << 8) + ((lfo * p[P_LD_SHP]) >> 7) + ((m.envq15 * p[P_ED_SHP]) >> 7);
#if CLIPPER_SYNTH
        {   /* VEL>F: around velocity 96, up to ~2 octaves up (127) and ~4.5 down (32); CUT: up to ~4 octaves,
             * WAH: the sound's LFO on the cutoff, up to ~+-2 octaves; SHP: the shape up to its top */
            int32_t a = e == &ENG_HYBRID ? 0 : v->pat > cat ? v->pat : cat;
            if (p[P_VELF])
                m.cutoff += ((int32_t)v->vel - 96) * p[P_VELF] * 4;
            if (p[P_MWDST] == MD_CUT)
                m.cutoff += mw * 120;
            if (p[P_ATDST] == MD_CUT)
                m.cutoff += a * 120;
            if (p[P_MWDST] == MD_WAH)
                m.cutoff += (lfo * mw) >> 9;
            if (p[P_ATDST] == MD_WAH)
                m.cutoff += (lfo * a) >> 9;
            if (p[P_MWDST] == MD_SHP)
                m.shape += mw * 128;
            if (p[P_ATDST] == MD_SHP)
                m.shape += a * 128;
        }
#endif
        e->render(t, v, out, n, &m);
        nr++;
    }
    if (fade) {
        for (i = 0; i < 8u; i++)
            t->p[P_E0 + i] = pe_new[i];
        t->xf--;
    }
    return nr;
}
