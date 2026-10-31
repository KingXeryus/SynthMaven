/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Kerem Kilic (Ellic Studio)
 * Portions Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Chord keys (SCL > CHORD: P_CHRD, P_VOIC): one key, MIDI note or arp input plays a chord. Included by seq.c
 * after kb_map; runs in the audio ISR with the rest of the note input.
 *   CHRD OFF; DIA3 / DIA7: the triad / seventh of the track's scale (ROOT, SCALE) built on the key, every tone
 *        in key (a key outside the scale takes the scale note below it, as QNT SNAP does; CHR builds on the
 *        major scale of ROOT); MAJ MIN DOM7 MAJ7 MIN7 SUS4 POW: that shape on the key, whatever the scale.
 *   VOIC CLOSE (root position); OPEN (the second tone an octave up: 1-5-3); INV1 / INV2 (the lowest tone, then
 *        the next, an octave up); +OCT (the root an octave down added; a seventh drops its fifth). At most
 *        CHORD_MAX notes, notes outside 0..127 left out.
 * The key is mapped first (kb_map: QNT SNAP / WHITE, TRN, the octave), the chord is built on the note it gives.
 * MONO / LEGATO / UNISON parts play the chord's root only; a kit (the DRUM engine, SAMPLE PERC, slices: an
 * engine that maps the keys itself) ignores CHRD. Every source keeps the notes it started (kb_chord for a
 * key, mchord for a MIDI note) and its release ends exactly those, so CHRD / VOIC changed while it is held
 * leave nothing hanging. A note several sources hold sounds once and ends with the last of them. */
/* Append modes: these values are stored in songs, presets and editor files. */
enum { CH_OFF, CH_DIA3, CH_DIA7, CH_MAJ, CH_MIN, CH_DOM7, CH_MAJ7, CH_MIN7, CH_SUS4, CH_POW, CH_AUTO, CH_MELODY, CH_OPEN5, CH_NEUTRAL, CH_LAST=CH_NEUTRAL };
enum { VC_CLOSE, VC_OPEN, VC_INV1, VC_INV2, VC_BASS };
#define CHORD_MAX 4u
#define MCHORD_N 32u                     /* MIDI notes held as chords at once (more: their root alone); a layered note takes one
                                          * per layer, so four stacked layers hold 8. At most 32: midi_strum_pending's bits */
_Static_assert(MCHORD_N <= 32u, "midi_strum_pending");

static const int8_t CHORD_SHAPE[CH_POW - CH_MAJ + 1][CHORD_MAX] = {
    {0, 4, 7, -1}, {0, 3, 7, -1}, {0, 4, 7, 10}, {0, 4, 7, 11}, {0, 3, 7, 10}, {0, 5, 7, -1}, {0, 7, 12, -1},
};

static uint8_t kb_chord[27][CHORD_MAX], kb_chn[27];   /* the notes key k started (kb_chn 0: none) */
/* Pending tones are separate from started ownership: early release never emits
 * an unmatched note-off or lets an unplayed tone keep another source alive. */
typedef struct { uint32_t pos, beat; uint8_t total, rate, vel; } chord_strum_t;
static chord_strum_t kb_strum[27];
static uint32_t kb_strum_pending, midi_strum_pending;
static uint8_t chord_alternate[NTRK];
typedef struct { uint8_t id, ch, src, n, note[CHORD_MAX]; chord_strum_t strum; } mchord_t;   /* id: track + 1, 0 = free */
static mchord_t mchord[MCHORD_N];
/* the last chord played per track (the CHORD page shows it): its root, notes, and the shape's tones above the
 * root (bit i = i semitones, 0..11) */
static struct { uint8_t root, n, note[CHORD_MAX]; uint16_t mask; uint8_t gen; } chord_last[NTRK];

/* 1 = the track's keys are a kit: no chords */
static int chord_kit(const track_t *t)
{
    const engine_t *e = ENGINES[eng_idx(t->eng_req)];
    return e->oneshot || (e->keys && e->keys(t, 0) >= 0);
}

#include "chord_auto.c"
#include "chord_melody.c"

/* Adapted from SynthMaven's scale-safe OPEN5 / NEUTRAL modes. Select only
 * existing keys by their actual tuned pitches; do not create equal-tempered
 * thirds over a maqam. All searches are bounded and allocation-free. */
static int sm_chord_micro(const track_t *t)
{
    if(sm_tuning.enabled){
        uint32_t part=trk_index(t);if(part>=4||!(sm_tuning.apply&(1u<<part)))return 0;
        for(uint32_t note=108;note<120;note++)if(sm_tuning_cents(&sm_tuning,part,note))return 1;
        return 0;
    }
    /* Legacy degree-addressed scales keep Melodee chord semantics. The
     * master chromatic tuning layer alone uses the neutral-only policy. */
    return 0;
}
static uint32_t sm_chord_neutral(const track_t *t,uint32_t root,uint32_t mode,uint8_t *out,int32_t *rp,uint16_t *maskp)
{
    uint32_t mask=scale_mask(t),tonic=scale_root(t),part=trk_index(t);
    while(root && !(mask&(1u<<((root+12-tonic)%12))))root--;
    out[0]=(uint8_t)root;*rp=(int32_t)root;*maskp=1;uint32_t n=1;
    if(trk_vmode(t)!=V_POLY)return n;
    const int32_t target[3]={35000,70000,120000};
    int32_t base=micro_active(t)?micro_pitch(t,root):(int32_t)root*10000+100*sm_tuning_cents(&sm_tuning,part,root);
    for(uint32_t j=mode==CH_NEUTRAL?0u:1u;j<(mode==CH_NEUTRAL?2u:3u);j++){
        int32_t best=0x7fffffff;uint32_t chosen=root;
        for(uint32_t key=root+1;key<128;key++)if(mask&(1u<<((key+12-tonic)%12))){
            int32_t pitch=micro_active(t)?micro_pitch(t,key):(int32_t)key*10000+100*sm_tuning_cents(&sm_tuning,part,key);
            int32_t delta=pitch-base,cost=delta-target[j];if(cost<0)cost=-cost;
            if(delta>0 && cost<best){best=cost;chosen=key;}
        }
        if(chosen>out[n-1]){out[n++]=(uint8_t)chosen;*maskp|=(uint16_t)(1u<<((chosen-root)%12));}
    }
    return n;
}

/* the tones of the chord (semitones above *root, ascending, iv[0] = 0) -> their number; DIA may move *root
 * down onto the scale */
static uint32_t chord_tones(const track_t *t, uint32_t mode, int32_t *root, int32_t *iv)
{
    uint32_t n = 0, i;
    if (micro_active(t)) {
        if (mode < CH_MAJ) {
            n = mode == CH_DIA7 ? 4u : 3u;
            for (i = 0; i < n; i++) iv[i] = 2 * (int32_t)i;
        } else {
            for (i = 0; i < CHORD_MAX && CHORD_SHAPE[mode - CH_MAJ][i] >= 0; i++) {
                int32_t target = micro_pitch(t, *root) + CHORD_SHAPE[mode - CH_MAJ][i] * 10000;
                int32_t lo = 0, hi = 127;
                while (lo < hi) {
                    int32_t mid = (lo + hi) / 2;
                    if (micro_pitch(t, mid) < target) lo = mid + 1; else hi = mid;
                }
                if (lo && target - micro_pitch(t, lo - 1) < micro_pitch(t, lo) - target) lo--;
                iv[n++] = lo - *root;
            }
        }
        return n;
    }
    if (mode >= CH_MAJ) {
        for (i = 0; i < CHORD_MAX && CHORD_SHAPE[mode - CH_MAJ][i] >= 0; i++)
            iv[n++] = CHORD_SHAPE[mode - CH_MAJ][i];
        return n;
    }
    {
        uint32_t mask = scale_mask(t), deg[12], c = 0, at = 0, guard = 12;
        int32_t r = scale_root(t);
        if (mask == 0xFFFu)
            mask = SCALE_MASK[1];                       /* CHR: no key of its own, the major scale of ROOT */
        while (guard-- && !((mask >> (uint32_t)((*root - r + 120) % 12)) & 1u))
            (*root)--;                                  /* outside the scale: the scale note below */
        for (i = 0; i < 12u; i++)
            if ((mask >> i) & 1u) {
                if (i == (uint32_t)((*root - r + 120) % 12))
                    at = c;
                deg[c++] = i;
            }
        for (i = 0; i < (mode == CH_DIA7 ? 4u : 3u); i++) {   /* every other note of the scale */
            uint32_t k = at + 2u * i;
            iv[n++] = (int32_t)deg[k % c] + 12 * (int32_t)(k / c) - (int32_t)deg[at];
        }
        return n;
    }
}

/* the chord on note `root` as track t plays it now -> out (ascending), the number of notes (1..CHORD_MAX);
 * *rp its root, *maskp its tones above the root (bit i: i semitones; 0 = no chord). The root alone when CHRD
 * is OFF, on a kit, or for MONO / LEGATO / UNISON (their chord's root) */
static uint32_t chord_make(const track_t *t, uint32_t root, uint8_t *out, int32_t *rp, uint16_t *maskp)
{
    int32_t iv[CHORD_MAX + 1u], r = (int32_t)root, x;
    uint32_t mode = (uint32_t)t->p[P_CHRD], n, i, j, m = 0;
    int32_t period = (int32_t)scale_note_period(t);
    uint16_t mask = 0;
    *rp = r;
    *maskp = 0;
    if(mode && mode<CH_OPEN5 && sm_chord_micro(t) &&
       (!sm_tuning.enabled || sm_tuning_applies(&sm_tuning,trk_index(t),root)))mode=CH_OPEN5;
    if(mode>=CH_OPEN5 && mode<=CH_LAST && !chord_kit(t))return sm_chord_neutral(t,root,mode,out,rp,maskp);
    int automatic = mode == CH_AUTO || mode == CH_MELODY;
    if (automatic && micro_scale(t)) *maskp = 1;
    if (!mode || mode > CH_MELODY || chord_kit(t) || (automatic && micro_scale(t))) {
        out[0] = (uint8_t)root;
        return 1;
    }
    if (mode == CH_AUTO) return chord_auto_make(t, root, out, rp, maskp);
    if (mode == CH_MELODY) return chord_melody_make(t, root, out, rp, maskp);
    n = chord_tones(t, mode, &r, iv);
    for (i = 0; i < n; i++)
        mask |= (uint16_t)(1u << (micro_active(t) ?
            ((micro_pitch(t, r + iv[i]) - micro_pitch(t, r) + 5000) / 10000 % 12) : iv[i] % 12));
    *rp = r;
    *maskp = mask;
    if (trk_vmode(t) != V_POLY) {                       /* one voice: the root */
        out[0] = (uint8_t)clamp(r, 0, 127);
        return 1;
    }
    switch (t->p[P_VOIC]) {
    case VC_OPEN:                                       /* 1-5-3(-7): the second tone an octave up */
        if (n >= 3u)
            iv[1] += period;
        break;
    case VC_INV2:
    case VC_INV1:
        for (j = 0; j < (t->p[P_VOIC] == VC_INV2 ? 2u : 1u); j++) {   /* the lowest tone an octave up */
            x = iv[0] + period;
            for (i = 1; i < n; i++)
                iv[i - 1u] = iv[i];
            iv[n - 1u] = x;
        }
        break;
    case VC_BASS:                                       /* the root an octave down; a seventh drops its fifth */
        if (n == CHORD_MAX) {
            iv[2] = iv[3];
            n--;
        }
        for (i = n; i > 0; i--)
            iv[i] = iv[i - 1u];
        iv[0] = -period;
        n++;
        break;
    default:
        break;
    }
    for (i = 1; i < n; i++)                             /* ascending */
        for (j = i; j > 0 && iv[j - 1u] > iv[j]; j--) {
            x = iv[j];
            iv[j] = iv[j - 1u];
            iv[j - 1u] = x;
        }
    for (i = 0; i < n && m < CHORD_MAX; i++) {          /* inside 0..127, each note once */
        x = r + iv[i];
        if (x < 0 || x > 127 || (m && out[m - 1u] == (uint8_t)x))
            continue;
        if (micro_active(t) && (micro_pitch(t, x) < 0 || micro_pitch(t, x) > 1270000)) continue;
        out[m++] = (uint8_t)x;
    }
    if (!m) {
        out[0] = (uint8_t)clamp(r, 0, 127);
        return 1;
    }
    return m;
}

/* chord_make for a note played now: a chord is kept in chord_last (the CHORD page shows it) */
static uint32_t chord_build(track_t *t, uint32_t root, uint8_t *out)
{
    int32_t r;
    uint16_t mask;
    uint32_t m = chord_make(t, root, out, &r, &mask), i, k = trk_index(t);
    if (t->p[P_CHRD] == CH_AUTO && !micro_scale(t) && !chord_kit(t) && trk_vmode(t) == V_POLY)
        chord_auto_remember(t, r, mask, out, m);
    if (m > 1u || t->p[P_CHRD] == CH_MELODY) {
        chord_last[k].root = (uint8_t)clamp(r, 0, 127);
        chord_last[k].mask = mask;
        chord_last[k].n = (uint8_t)m;
        for (i = 0; i < CHORD_MAX; i++)
            chord_last[k].note[i] = i < m ? out[i] : 0u;
        chord_last[k].gen++;
    }
    return m;
}

/* a chord's name from its root and tones (bit i: i semitones above the root): "C", "Cm7", "F#m7b5", "Gsus4",
 * "A5" -> b (12 bytes) */
static void chord_name(char *b, uint32_t root, uint32_t mask)
{
    uint32_t mi = (mask >> 3) & 1u, ma = (mask >> 4) & 1u, b5 = (mask >> 6) & 1u, p5 = (mask >> 7) & 1u;
    uint32_t s5 = (mask >> 8) & 1u, d7 = (mask >> 9) & 1u, m7 = (mask >> 10) & 1u, M7 = (mask >> 11) & 1u;
    const char *q, *x = "";
    str_cpy(b, N_NOTE[root % 12u], 12);
    if (mi && !ma && b5 && !p5)
        q = d7 ? "dim7" : m7 ? "m7b5" : "dim";
    else if (ma && !mi && s5 && !p5)
        q = m7 ? "+7" : M7 ? "+M7" : "aug";
    else if (ma && !mi)
        q = m7 ? "7" : M7 ? "maj7" : d7 ? "6" : "";
    else if (mi)
        q = m7 ? "m7" : M7 ? "mM7" : d7 ? "m6" : "m";
    else if ((mask >> 5) & 1u) {                        /* no third: suspended */
        q = m7 ? "7" : "";
        x = "sus4";
    } else if ((mask >> 2) & 1u) {
        q = m7 ? "7" : "";
        x = "sus2";
    } else
        q = "5";
    str_cpy(b + str_len(b), q, 12 - str_len(b));
    str_cpy(b + str_len(b), x, 12 - str_len(b));
}

/* the MIDI note (ch, src) of track id - 1 that plays a chord, 0 = none */
static mchord_t *mchord_of(uint32_t ch, uint32_t src, uint32_t id)
{
    uint32_t i;
    for (i = 0; i < MCHORD_N; i++)
        if (mchord[i].id == id && mchord[i].ch == ch && mchord[i].src == src)
            return &mchord[i];
    return 0;
}

/* a MIDI-held chord of track id - 1 sounds note */
static int mchord_held(uint32_t id, uint32_t note)
{
    uint32_t i, j;
    for (i = 0; i < MCHORD_N; i++)
        if (mchord[i].id == id)
            for (j = 0; j < mchord[i].n; j++)
                if (mchord[i].note[j] == note)
                    return 1;
    return 0;
}

static void mchord_forget(uint32_t track)               /* panic: the track's MIDI chords are gone */
{
    chord_auto_forget(track);
    uint32_t i;
    for (i = 0; i < MCHORD_N; i++)
        if (mchord[i].id == track + 1u)
            mchord[i].id = 0;
}

/* A division describes the complete first-to-last sweep, not each string gap.
 * Freeze its rate and pitch order at the press; tempo may still change. ARP owns
 * timing while enabled, so it receives the complete chord immediately. */
static uint32_t chord_strum_begin(track_t *t, uint8_t *notes, uint32_t n, chord_strum_t *s, uint32_t vel)
{
    memset(s, 0, sizeof *s);
    uint32_t rate = (uint32_t)t->p[P_STRUM];
    if (!rate || rate >= NELEM(N_STRUM) || n < 2u || t->p[P_AMODE]) return n;
    uint32_t up = t->p[P_STROKE] == 1;
    if (t->p[P_STROKE] == 2) up = chord_alternate[trk_index(t)]++ & 1u;
    if (up) for (uint32_t i = 0; i < n / 2u; i++) {
        uint8_t x = notes[i]; notes[i] = notes[n - 1u - i]; notes[n - 1u - i] = x;
    }
    if (!up && t->p[P_CHRD] == CH_MELODY && !micro_scale(t)) {
        /* The melody sounds on the press; DOWN/UP/ALT order its accompaniment.
         * Freeze that order here so later mode changes cannot delay the tune. */
        uint8_t melody = notes[n - 1u];
        for (uint32_t i = n - 1u; i; i--) notes[i] = notes[i - 1u];
        notes[0] = melody;
    }
    s->total = (uint8_t)n; s->rate = (uint8_t)rate; s->vel = (uint8_t)vel;
    s->beat = seq_beat_samples();
    return 1u;
}

/* Keep elapsed beat fraction through tempo changes and INT/MIDI clock switches. */
static uint32_t chord_strum_due(chord_strum_t *s, uint32_t started, uint32_t elapsed)
{
    static const uint8_t DEN[] = {1, 2, 4, 8, 16, 32, 6, 12};
    uint32_t beat = seq_beat_samples();
    if (s->beat != beat) {
        s->pos = (uint32_t)(((uint64_t)s->pos * beat + s->beat / 2u) / s->beat);
        s->beat = beat;
    }
    s->pos += elapsed;
    uint32_t target = (beat / DEN[s->rate]) * started / (s->total - 1u);
    return s->pos < target ? target - s->pos : 0u;
}
