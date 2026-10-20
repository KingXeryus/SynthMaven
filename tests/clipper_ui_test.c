/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper's synth interface (CLIPPER_SYNTH 1), on the harness of tests/ui_pages_test.c (the real UI on a
 * framebuffer, the audio running between frames):
 *   power-on: the instrument plays CLIPPER; HOME is the synth screen (no tracks; 0.3: no visualiser)
 *   PRESETS browses the sounds, ALGORITHM jumps from kind to kind (and back); not on a page that edits the sound
 *   one instrument: an old project / the editor selecting another track comes back to track 1; tracks 2..4 and the
 *   drums are silent even with patterns and notes; MIDI plays the instrument on every channel (omni, 10 too)
 *   FX and SAVE held, the keys still play; tapped (or held long), their pages; SAVE opens the SAVE pages (no song);
 *   SEQ and SEL keep their layers (steps; the key); 1.0 (the sequencer of SLOOP 2.7.2): EDIT (erase, SHIFT, LENGTH,
 *   TRANSPOSE, undo), ARP (note repeat) and GLO (fill, fill bar, tap, the level) are layers again; SEQ > NOTES, the note
 *   editor and its sheet; MENU > SEQ > STEP ADVANCE; step entry from a MIDI keyboard
 *   the pages: no SLICER, no DRUMS, no SONG page; MASTER without DUCK and ROLL; VOICE 2 without MUTE
 *   the REC screen: the instrument alone; REC held: "PATTERN CLEARED"
 *   screens to DIR/clipper-*.ppm for review. */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"

static uint32_t voices_part(uint32_t p)
{
    uint32_t i, n = 0;
    for (i = 0; i < NVOICE; i++)
        n += trk[p].v[i].active && trk[p].v[i].stage != 4u;
    return n;
}
static const char *sound_name(void)
{
    const engine_t *e = ENGINES[TSEL->eng_req % NENGINES];
    return e->presets[TSEL->preset % e->npresets].name;
}
static int page_named(const char *t)
{
    uint32_t i;
    for (i = 0; i < NPAGES; i++)
        if (!strcmp(PAGES[i].title, t) && PAGES[i].fam != FAM_TRK)
            return 1;
    return 0;
}
static void midi_in(uint32_t st, uint32_t d1, uint32_t d2)    /* a USB-MIDI packet, as usb.c queues it */
{
    midi_in_q[mi_w % MQ] = (st >> 4) | st << 8 | d1 << 16 | d2 << 24;
    mi_w++;
}

int main(int argc, char **argv)
{
    uint32_t i;
    outdir = argc > 1 ? argv[1] : "build/host";
    check(CLIPPER_SYNTH == 1, "built as the synth (CLIPPER_SYNTH 1)");
    bank_resolve();
    panel = PANEL_DEFAULT;
    layers_init();
    settings.palette = 4;
    palette_set(4);
    host_tracks_init();
    for (i = 0; i < NPART; i++) { set_engine_of(&trk[i], TRK_DEF[i][0]); apply_preset_to(&trk[i], TRK_DEF[i][1]); trk[i].engine = trk[i].eng_req; }
    TDRUM->p[P_E0] = DRUM_DEFAULT_KIT;
    song.g[G_BPM] = 120;
    sloop_splash(); ppm("clipper-splash");
    go_home(); ui.force = 1; frames(3);
    check(ui.home && song.sel == 0 && trk[0].eng_req == ENGI_HYBRID && !strcmp(sound_name(), "CLIPPER"),
          "power-on: HOME, the instrument plays CLIPPER");
    key(7); frames(2);                                    /* (a note, for the scope) */
    fm1_in.notes |= 1u << 7; frames(6); ui.force = 1; frame(); ppm("clipper-home"); fm1_in.notes &= ~(1u << 7); frames(30);
    ui.menu = 2; ui.force = 1; frame(); ppm("clipper-about"); ui.menu = 0; ui.force = 1; frame();

    /* PRESETS and ALGORITHM */
    {
        uint32_t total, p0 = preset_pos(&total), p1, k0, k1;
        encs[panel.enc[EN_PRESET]] = 1; frames(2);
        p1 = preset_pos(&total);
        check(p1 == (p0 + 1u) % total, "PRESETS on HOME: the next sound");
        k0 = kind_at(preset_pos(&total));
        encs[panel.enc[EN_ALGO]] = 1; frames(2);
        p1 = preset_pos(&total);
        k1 = kind_at(p1);
        check(k1 != k0 && (p1 == 0 || kind_at(p1 - 1u) != k1), "ALGORITHM +1: the first sound of the next kind");
        encs[panel.enc[EN_ALGO]] = -1; frames(2);
        check(kind_at(preset_pos(&total)) == k0 && (preset_pos(&total) == 0 || kind_at(preset_pos(&total) - 1u) != k0),
              "ALGORITHM -1: back to the first sound of the kind before");
        for (i = 0; i < 12u; i++) { encs[panel.enc[EN_ALGO]] = 1; frames(1); }
        check(preset_pos(&total) < total, "ALGORITHM round the list: still a sound");
        open_family(FAM_ENV); frames(2);
        p0 = preset_pos(&total);
        encs[panel.enc[EN_PRESET]] = 1; encs[panel.enc[EN_ALGO]] = 1; frames(2);
        check(preset_pos(&total) == p0, "on the ENV page PRESETS and ALGORITHM leave the sound alone");
        go_home(); frames(2);
    }
    {   /* the preset list: every name found; every preset of Clipper's own engines (WAVE .. MACRO) there once */
        uint32_t e, k, n, hits, ok = 1;
        for (n = 0; n < NBANK; n++)
            if (bank_pi[n] == 0xFF) {
                printf("clipper: BANK %s: no such preset in engine %u\n", BANK[n].name, BANK[n].e);
                ok = 0;
            }
        for (e = 0; e < NENGINES; e++)
            for (k = 0; k < ENGINES[e]->npresets; k++) {
                for (hits = 0, n = 0; n < NBANK; n++)
                    hits += BANK[n].e == e && bank_pi[n] == k;
                if (hits != 1) {
                    printf("clipper: preset %s (engine %u) is %u times in BANK\n", ENGINES[e]->presets[k].name, e, hits);
                    ok = 0;
                }
            }
        check(ok, "the preset list: every name found, every factory preset of every engine once");
    }
    /* back to CLIPPER for the rest */
    set_engine_of(TSEL, ENGI_HYBRID); apply_preset_to(TSEL, 0); frames(3);

    /* one instrument */
    track_select(2); frames(1);
    check(song.sel == 0, "track_select (an editor, a stray call): still track 1");
    song.sel = 3; frames(1);
    check(song.sel == 0, "an old project's selected track 4: back to track 1 at the next frame");
    {
        uint32_t j;
        uint8_t n[1] = {60};
        for (j = 0; j < 16u; j += 4u) {                  /* patterns on tracks 2 and 4 (an old SLOOP project) */
            put_step(&trk[1], j, 1, n, ST_NOTE, 0);
            trk[3].dstep[j].on[0] = 1;
        }
        trk[1].p[P_SLEN] = trk[3].p[P_SLEN] = 16;
        transport_req = 1; frames(40);
        check(voices_part(1) == 0 && voices_part(2) == 0, "playing: tracks 2 and 3 sound nothing (their patterns stopped)");
        {
            uint32_t d, busy = 0;
            for (d = 0; d < NDRUM; d++)
                busy += drums.v[d].active;
            check(busy == 0, "playing: the drum machine sounds nothing");
        }
        transport_req = 2; frames(30);
    }
    {   /* MIDI: every channel plays the instrument; 10 too */
        uint32_t ch;
        for (ch = 0; ch < 16u; ch++) {
            midi_in(0x90u | ch, 60u + ch, 100); frames(1);
        }
        check(voices_part(0) >= 8u || voices_part(0) == NVOICE, "MIDI notes on channels 1..16: the instrument plays them");
        check(voices_part(1) == 0 && voices_part(2) == 0, "...and nothing else does");
        for (ch = 0; ch < 16u; ch++)
            midi_in(0x80u | ch, 60u + ch, 0);
        frames(200);
        for (i = 0, ch = 0; i < NVOICE; i++)
            ch += trk[0].v[i].gate;
        check(ch == 0, "the note-offs on every channel let them all go");
    }

    /* 1.0 (a comment from a KeyStep Pro user): GLO > MIDI, IN CH (OMNI or one channel) and OUT CH */
    {
        uint32_t pg, ch, i2, n1, n3, ok;
        for (pg = 0; pg < NPAGES && strcmp(PAGES[pg].title, "MIDI"); pg++)
            ;
        check(pg < NPAGES && PAGES[pg].fam == FAM_GLO && midi_in_ch == 0u, "GLO > MIDI: a page; IN CH OMNI by default");
        go_home(); frames(2);
        while (strcmp(cur_page()->title, "MIDI")) { open_family(FAM_GLO); frame(); }
        encs[panel.enc[EN_K1]] = 1; frame(); encs[panel.enc[EN_K1]] = 1; frame(); encs[panel.enc[EN_K1]] = 1; frame();
        ui.force = 1; frame(); ppm("clipper-midi");
        ok = midi_in_ch == 3u;
        midi_in(0x90u, 60, 100); frames(2);
        n1 = voices_part(0);
        midi_in(0x80u, 60, 0); midi_in(0xB0u, 7, 0); frames(2);       /* (a CC on channel 1: not ours either) */
        ok &= trk[0].p[P_LEVEL] != 0;
        midi_in(0x92u, 62, 100); frames(2);
        n3 = voices_part(0);
        midi_in(0x82u, 62, 0); frames(60);
        check(ok && n1 == 0u && n3 >= 1u, "GLO > MIDI > IN CH 3: channel 3 plays the instrument, channel 1 (notes, CCs) does not");
        for (i2 = 0, ch = 0; i2 < NVOICE; i2++) ch += trk[0].v[i2].gate;
        check(ch == 0u, "IN CH 3: its note-offs let the notes go");
        usb.config = 1;                               /* (MIDI out goes to a configured USB host) */
        mo_r = mo_w;
        encs[panel.enc[EN_K2]] = 1; frame(); encs[panel.enc[EN_K2]] = 1; frame();
        ok = midi_out_ch == 2u;
        key(9); frames(2);
        for (ch = 16; mo_r != mo_w; mo_r++)
            if (((midi_out_q[mo_r % MQ] >> 8) & 0xF0u) == 0x90u)
                ch = (midi_out_q[mo_r % MQ] >> 8) & 15u;
        usb.config = 0;
        check(ok && ch == 2u, "GLO > MIDI > OUT CH 3: the keys go out on channel 3");
        encs[panel.enc[EN_K1]] = -5; frame(); encs[panel.enc[EN_K2]] = -5; frame();
        frames(4);
        midi_in(0x95u, 64, 100); frames(2);
        check(midi_in_ch == 0u && midi_out_ch == 0u && voices_part(0) >= 1u, "IN CH back to OMNI, OUT CH 1: every channel again");
        midi_in(0x85u, 64, 0); frames(60);
        go_home(); frames(2);
    }
    /* the buttons with no layer: held, the keys play; their pages on release */
    {
        static const struct { uint32_t b, fam; const char *n; } B[] = {{B_FX, FAM_FX, "FX"}, {B_SAVE, FAM_SAVE, "SAVE"}};
        char what[96];
        for (i = 0; i < NELEM(B); i++) {
            go_home(); frames(2);
            press(B[i].b); frames(20);                   /* held a while (longer than a tap) */
            fm1_in.notes |= 1u << 9; frames(3);
            snprintf(what, sizeof what, "%s held + a key: the key plays (no layer)", B[i].n);
            check(ui.layer == LY_PLAY && voices_part(0) >= 1u, what);
            fm1_in.notes &= ~(1u << 9); frames(2);
            release(B[i].b); frames(2);
            snprintf(what, sizeof what, "%s held with a key, let go: no page (a key was used)", B[i].n);
            check(ui.home, what);
            press(B[i].b); frames(40); release(B[i].b); frames(2);
            snprintf(what, sizeof what, "%s held 0.6 s, nothing else: its pages", B[i].n);
            check(!ui.home && cur_page()->fam == B[i].fam, what);
            frames(60);
        }
        go_home(); frames(2);
        tap(B_SAVE); frames(2);
        check(!ui.home && cur_page()->fam == FAM_SAVE && cur_page()->scope != SC_SONG, "SAVE tapped on HOME: the SAVE pages (no song screen)");
        ui.force = 1; frame(); ppm("clipper-save");
    }
    /* SEQ and SEL keep their layers */
    {
        go_home(); frames(2);
        press(B_SEQ); frames(12);
        check(ui.layer == LY_STEP, "SEQ held: the steps layer");
        ui.force = 1; frame(); ppm("clipper-layer-steps");
        key(0); frames(2);
        check(trk_step_on(TSEL, 0), "SEQ held + key 1: step 1 set");
        release(B_SEQ); frames(2);
        press(B_SCL); frames(12);
        check(ui.layer == LY_SCALE, "SEL held: the key layer");
        release(B_SCL); frames(2);
    }
    /* 1.0, the sequencer of SLOOP 2.7.2: EDIT, ARP and GLO are layers again */
    {
        track_t *t = TSEL;
        uint32_t j;
        int ok;
        uint8_t n60[1] = {60}, n64[1] = {64};
        transport_req = 2; frames(4);
        go_home(); frames(2);
        t->p[P_SLEN] = 16; t->p[P_CHORD] = 0; song.octave = 0;
        for (j = 0; j < NSTEP; j++) step_clear(&t->step[j]);
        for (j = 0; j < 16u; j += 4u) put_step(t, j, 1, j % 8u ? n64 : n60, ST_NOTE, 0);
        press(B_EDIT); frames(12);
        ok = ui.layer == LY_ERASE;
        ui.force = 1; frame(); ppm("clipper-layer-edit");
        {                                             /* EDIT + a key, stopped: that note gone from the whole pattern */
            uint32_t k;
            for (k = 0; k < 27u && kb_map(t, k) != 64u; k++)
                ;
            ok &= k < 27u;
            key(k); frames(2);
        }
        ok &= step_on(&t->step[0]) && !step_on(&t->step[4]) && step_on(&t->step[8]) && !step_on(&t->step[12]) && voices_part(0) == 0u;
        check(ok, "EDIT held: the erase layer; EDIT + E4 (stopped) erases every E4 (the C4s stay), nothing plays");
        press(B_OCTDN); release(B_OCTDN); frames(2);
        check(step_on(&t->step[4]) && step_on(&t->step[12]), "EDIT + OCT-: undo (the E4s back)");
        encs[panel.enc[EN_K1]] = 1; frames(2);       /* SHIFT: every step one later */
        ok = step_on(&t->step[1]) && !step_on(&t->step[0]) && t->step[1].note[0] == 60u;
        encs[panel.enc[EN_K3]] = 1; frames(2); encs[panel.enc[EN_K3]] = 1; frames(2);   /* TRANSPOSE +2 (a semitone a turn) */
        ok &= t->step[1].note[0] == 62u && t->step[5].note[0] == 66u;
        encs[panel.enc[EN_K2]] = 1; frames(2);       /* LENGTH x2 */
        ok &= t->p[P_SLEN] == 32 && t->step[17].note[0] == 62u;
        release(B_EDIT); frames(4);
        check(ok && ui.home, "EDIT + KNOB 1 SHIFT, KNOB 3 TRANSPOSE, KNOB 2 LENGTH x2; let go: HOME again (no page)");
        press(B_EDIT); press(B_OCTDN); release(B_OCTDN); release(B_EDIT); frames(4);
        check(t->p[P_SLEN] == 16 && t->step[0].note[0] == 60u && ui.home, "EDIT + OCT-: the whole hold's turns undone at once");
        tap(B_EDIT); frames(2);
        check(!ui.home && cur_page()->fam == FAM_EDIT, "EDIT tapped: its pages");
        go_home(); frames(2);
        press(B_GLO); frames(12);
        ok = ui.layer == LY_MIX;
        ui.force = 1; frame(); ppm("clipper-layer-glo");
        fm1_in.notes |= 1u << key_of_white(8); frames(2);
        ok &= fill_now != 0;
        fm1_in.notes &= ~(1u << key_of_white(8)); frames(2);
        ok &= !fill_now;
        key(key_of_white(0)); frames(2);
        ok &= !t->p[P_MUTE] && !trk[1].p[P_MUTE] && voices_part(0) == 0u;   /* (no mute keys: nothing, nothing plays) */
        t->p[P_LEVEL] = 100;
        encs[panel.enc[EN_K1]] = -10; encs[panel.enc[EN_K2]] = -10; frames(2);
        ok &= t->p[P_LEVEL] < 100 && trk[1].p[P_LEVEL] == trk[1].p[P_LEVEL];
        {
            int16_t l1 = trk[1].p[P_LEVEL];
            encs[panel.enc[EN_K2]] = -10; frames(2);
            ok &= trk[1].p[P_LEVEL] == l1;
        }
        release(B_GLO); frames(4);
        t->p[P_LEVEL] = 100;
        check(ok, "GLO held: key 9 a fill while held; keys 1..8 nothing (one instrument); KNOB 1 the sound's level, KNOB 2 nothing");
        press(B_ARP); frames(12);
        ok = ui.layer == LY_ROLL;
        transport_req = 1; frames(4);
        for (j = 0; j < NSTEP; j++) step_clear(&t->step[j]);
        {
            uint32_t k, on = 0, prev = 0, f;
            for (k = 0; k < 27u && kb_map(t, k) != 67u; k++)
                ;
            fm1_in.notes |= 1u << k;
            for (f = 0; f < 120u; f++) {             /* about a second at 120 BPM: a roll of 1/16s (8 a second) */
                uint32_t g = 0, i;
                frame();
                for (i = 0; i < NVOICE; i++)
                    g += t->v[i].active && t->v[i].gate && t->v[i].note == 67;
                on += g && !prev;
                prev = g;
            }
            fm1_in.notes &= ~(1u << k); frames(4);
            ok &= on >= 4u;
            printf("clipper:   ARP roll: %u hits in 120 frames\n", on);
        }
        transport_req = 2; frames(4);
        release(B_ARP); frames(30);
        check(ok, "ARP held: the roll layer; a key repeats on the grid (note repeat)");
        go_home(); frames(2);
    }
    /* 1.0, after SLOOP 2.7.2: SEQ > NOTES, the note editor and its sheet */
    {
        track_t *t = TSEL;
        uint32_t j;
        int ok;
        transport_req = 2; frames(2);
        song.rec = 0; rec_wait = 0; song.octave = 0; layer_unlock();
        t->p[P_VOICE] = V_POLY; t->p[P_CHORD] = 0;
        go_home(); frame();
        t->p[P_SLEN] = 16;
        for (j = 0; j < NSTEP; j++) step_clear(&t->step[j]);
        memset(t->micro, 0, sizeof t->micro); memset(t->fill, 0, sizeof t->fill);
        t->step[0].n = 1; t->step[0].note[0] = 48; t->step[0].time = ST_NOTE;          /* C3, 2 steps */
        t->step[1].time = ST_TIE;
        t->step[4].n = 3; t->step[4].note[0] = 60; t->step[4].note[1] = 64; t->step[4].note[2] = 67;
        t->step[4].time = ST_NOTE;                                                  /* a chord on step 5 */
        t->step[8].n = 1; t->step[8].note[0] = 55; t->step[8].time = ST_NOTE;
        open_family(FAM_SEQ); frame();
        check(!strcmp(PAGES[ui.page].title, "NOTES"), "SEQ tapped: the NOTES page (STEP before 1.0)");
        cursor_set(0); ui.force = 1; frame();
        encs[panel.enc[EN_K1]] = 1; frame();
        ok = ui.cursor == 2u && ui.note_k == 0u;                                    /* (step 2 is the note's TIE: passed) */
        encs[panel.enc[EN_K1]] = 2; frame();
        ok &= ui.cursor == 4u && ui.note_k == 0u;
        encs[panel.enc[EN_K1]] = 2; frame();
        ok &= ui.cursor == 4u && ui.note_k == 2u;                                   /* the chord's notes one by one */
        encs[panel.enc[EN_K1]] = -1; frame();
        ok &= ui.cursor == 4u && ui.note_k == 1u;
        check(ok, "NOTES: KNOB 1 walks the notes, a chord's one by one, past a note's TIEs");
        encs[panel.enc[EN_K2]] = 1; frame();
        check(t->step[4].note[0] == 60u && t->step[4].note[1] == 65u && t->step[4].note[2] == 67u,
              "NOTES: KNOB 2 changes that note's pitch only (E4 -> F4, the chord's others stay)");
        encs[panel.enc[EN_K3]] = 2; frame();
        check(note_len(t, 4) == 3u && t->step[5].time == ST_TIE && t->step[6].time == ST_TIE, "NOTES: KNOB 3 its length (3 steps: two TIEs)");
        encs[panel.enc[EN_K4]] = 1; frame();
        ok = note_force(&t->step[4], 1) == 3u && ((t->step[4].lvl >> 2) & 3u) == LV_HARD && !(t->step[4].flags & SF_ACCENT);
        encs[panel.enc[EN_K4]] = 1; frame();
        ok &= note_force(&t->step[4], 1) == 4u && (t->step[4].flags & SF_ACCENT);
        encs[panel.enc[EN_K4]] = -1; frame(); encs[panel.enc[EN_K4]] = -1; frame();
        encs[panel.enc[EN_K4]] = -1; frame(); encs[panel.enc[EN_K4]] = -1; frame();
        ok &= note_force(&t->step[4], 1) == 0u && ((t->step[4].lvl >> 2) & 3u) == LV_GHOST && !(t->step[4].flags & SF_ACCENT);
        check(ok, "NOTES: KNOB 4 its FORCE: HARD, ACC (the step's accent), down to GHOST");
        {
            uint32_t total, p0 = preset_pos(&total);
            encs[panel.enc[EN_PRESET]] = 1; frame();
            ok = (int8_t)ui.zoom == 1;
            encs[panel.enc[EN_PRESET]] = -1; frame(); encs[panel.enc[EN_PRESET]] = -1; frame();
            ok &= (int8_t)ui.zoom == -1;
            encs[panel.enc[EN_PRESET]] = 1; frame();
            check(ok && (int8_t)ui.zoom == 0 && preset_pos(&total) == p0, "NOTES: PRESETS zooms (16 -> 32, then 8; the sound stays)");
        }
        ui.force = 1; frame(); ppm("clipper-notes-chord");
        tap(B_OCTUP); frame();
        check(song.octave == 1 && !ui.sheet, "NOTES: OCT+ tapped: the octave up, as before (on release)");
        song.octave = 0;
        press(B_OCTUP); frames(30);
        ok = ui.sheet == 1u && song.octave == 0;
        release(B_OCTUP); frame();
        check(ok && ui.sheet == 1u && song.octave == 0, "NOTES: OCT+ held 0.4 s: the note's sheet (the octave stays)");
        ui.force = 1; frame(); ppm("clipper-notes-sheet");
        tap(B_OCTUP); frame();
        ok = (t->step[4].flags & SF_SLIDE) != 0;
        encs[panel.enc[EN_K1]] = 1; frame();
        encs[panel.enc[EN_K2]] = 3; frame();
        ok &= ui.sheet == 2u && t->micro[4] == 3;
        encs[panel.enc[EN_K1]] = 1; frame();
        encs[panel.enc[EN_K2]] = 1; frame();
        ok &= step_fill(t, 4) == FC_FILL;
        check(ok, "NOTES sheet: SLIDE (OCT+), NUDGE +3, FILL (KNOB 1 the row, KNOB 2 its value)");
        encs[panel.enc[EN_K1]] = 1; frame();
        encs[panel.enc[EN_K2]] = -1; frame();
        ok = ui.cursor == 3u && t->step[3].n == 3u && t->step[3].note[1] == 65u && note_len(t, 3) == 3u && !step_on(&t->step[4]) &&
             t->step[6].time == ST_REST && t->micro[3] == 3 && t->micro[4] == 0 && step_fill(t, 3) == FC_FILL;
        encs[panel.enc[EN_K2]] = -1; frame();
        ok &= ui.cursor == 2u && t->step[2].n == 3u;
        encs[panel.enc[EN_K2]] = -1; frame();
        ok &= ui.cursor == 2u && !strcmp(ui.msg, "NO ROOM");                        /* (step 1: the C3's TIE) */
        check(ok, "NOTES sheet: MOVE, KNOB 2 a step a detent (with its TIEs, nudge, fill); not over another note");
        encs[panel.enc[EN_K1]] = 1; frame();
        tap(B_OCTUP); frame();
        ok = step_on(&t->step[2]) && step_on(&t->step[5]) && t->step[5].note[1] == 65u && ui.cursor == 5u && ui.sheet == 4u;
        check(ok, "NOTES sheet: COPY (OCT+): just after it, the copy picked, MOVE next");
        encs[panel.enc[EN_K1]] = 2; frame();
        tap(B_OCTUP); frame();
        ok = step_on(&t->step[5]) && t->step[5].n == 2u && ui.sheet == 0u;
        check(ok, "NOTES sheet: DELETE (OCT+): that note (the chord keeps its others), the sheet closes");
        press(B_OCTUP); frames(30); release(B_OCTUP); frame();
        tap(B_OCTDN); frame();
        check(!ui.sheet && song.octave == 0, "NOTES: OCT- closes the sheet (the octave stays)");
        press(B_EDIT); release(B_EDIT); frames(2);
        check(t->step[5].n == 1u && step_on(&t->step[5]), "NOTES: EDIT tapped deletes the note under edit only");
        press(B_EDIT); release(B_EDIT); frames(2);
        check(!step_on(&t->step[5]) && t->step[6].time == ST_REST, "NOTES: the chord's last note: the step and its TIEs go");
        press(B_EDIT); press(B_OCTDN); release(B_OCTDN); release(B_EDIT); frames(2);
        check(step_on(&t->step[5]), "NOTES: EDIT + OCT- brings it back (undo)");
        cursor_set(0); ui.force = 1; frame(); ppm("clipper-notes");
        frames(20);                                   /* (EDIT let go: KNOB 1..4 quiet 250 ms, #39) */
        /* the keys write the step under edit; STEP ADVANCE (MENU > SEQ) */
        {
            uint32_t kc, ke, kg;
            for (kc = 0; kc < 27u && kb_map(t, kc) != 60u; kc++) ;
            for (ke = 0; ke < 27u && kb_map(t, ke) != 64u; ke++) ;
            for (kg = 0; kg < 27u && kb_map(t, kg) != 67u; kg++) ;
            cursor_set(10); frame();
            fm1_in.notes |= 1u << kc | 1u << ke; frame(); fm1_in.notes = 0; frame();
            ok = t->step[10].n == 2u && t->step[10].note[1] == 64u && ui.cursor == 11u;
            step_adv_off = 1;
            cursor_set(12); frame();
            fm1_in.notes |= 1u << kc | 1u << ke; frame(); fm1_in.notes = 0; frame();   /* a wrong chord */
            ok &= ui.cursor == 12u && t->step[12].n == 2u;
            fm1_in.notes |= 1u << kc | 1u << ke | 1u << kg; frame(); fm1_in.notes = 0; frame();
            ok &= ui.cursor == 12u && t->step[12].n == 3u && t->step[12].note[2] == 67u;   /* the chord again: the step anew */
            encs[panel.enc[EN_K1]] = 1; frame(); encs[panel.enc[EN_K1]] = 1; frame();
            encs[panel.enc[EN_K1]] = 1; frame();
            ok &= ui.cursor == 13u && ui.note_k == 0u;
            step_adv_off = 0;
            check(ok, "NOTES: the keys write the step under edit (and move on); STEP ADVANCE OFF: the cursor stays, a chord again rewrites it");
            /* a MIDI keyboard on the NOTES page (1.0, after SLOOP 2.6) */
            cursor_set(13); frame();
            midi_in(0x90u, 62, 100); midi_in(0x90u, 65, 100); frames(2);
            midi_in(0x80u, 62, 0); midi_in(0x80u, 65, 0); frames(3);
            ok = t->step[13].n == 2u && t->step[13].note[0] == 62u && t->step[13].note[1] == 65u && ui.cursor == 14u;
            midi_in(0x95u, 69, 90); frames(2); midi_in(0x85u, 69, 0); frames(3);   /* (omni: any channel) */
            ok &= t->step[14].n == 1u && t->step[14].note[0] == 69u && ui.cursor == 15u;
            check(ok, "NOTES: a MIDI keyboard writes the step too (a chord: the notes held together; let go: the next step)");
            go_home(); frames(2);
            midi_in(0x90u, 71, 100); frames(2); midi_in(0x80u, 71, 0); frames(3);
            check(t->step[15].time != ST_NOTE || t->step[15].note[0] != 71u, "off the NOTES page, MIDI writes no step");
            frames(100);
        }
        /* (SLOOP 2.7.2) a legato step (SF_LEG) on the instrument: the note before is held into it, the voice takes the
         * new note with no new attack; the same step without SF_LEG: the note before ends, a new attack */
        {
            uint32_t pass, att[2] = {0, 0}, gated_all[2] = {1, 1}, seen55[2] = {0, 0};
            int16_t atk = t->p[P_ATK], sus = t->p[P_SUS];
            go_home(); frames(2);
            for (pass = 0; pass < 2u; pass++) {
                uint32_t f, i, v0 = NVOICE;
                for (j = 0; j < NSTEP; j++) step_clear(&t->step[j]);
                memset(t->micro, 0, sizeof t->micro); memset(t->fill, 0, sizeof t->fill);
                t->p[P_SLEN] = 16; t->p[P_VOICE] = V_LEGATO; t->p[P_GLIDE] = 0; t->p[P_ATK] = 70; t->p[P_SUS] = 100;
                t->step[0].n = 1, t->step[0].note[0] = 48, t->step[0].time = ST_NOTE;
                t->step[1].time = t->step[2].time = t->step[3].time = ST_TIE;
                t->step[4].n = 1, t->step[4].note[0] = 55, t->step[4].time = ST_NOTE;
                t->step[4].flags = (uint8_t)(pass ? 0u : SF_LEG);
                t->step[5].time = ST_TIE;
                transport_req = 1;
                for (f = 0; f < 60u; f++) {               /* (8 steps at 120 BPM: 1 s) */
                    uint32_t g = 0, idx;
                    frame();
                    idx = t->seq_idx;
                    for (i = 0; i < NVOICE; i++)
                        if (t->v[i].active && t->v[i].gate) {
                            g++;
                            if (v0 == NVOICE && t->v[i].note == 48)
                                v0 = i;
                            if (t->v[i].note == 55 && !seen55[pass]) {
                                seen55[pass] = 1;
                                att[pass] = i != v0 || t->v[i].stage == 1u || t->v[i].env < 1000;
                            }
                        }
                    if (song.playing && idx >= 1u && idx <= 3u && f > 2u && !g)
                        gated_all[pass] = 0;
                    if (idx >= 6u)
                        break;
                }
                transport_req = 2; frames(10);
            }
            t->p[P_VOICE] = V_POLY; t->p[P_ATK] = atk; t->p[P_SUS] = sus;
            check(seen55[0] && !att[0] && gated_all[0] && seen55[1] && att[1],
                  "a LEGATO step (SF_LEG, 1.0): the note before held into it, the same voice takes it, no new attack (without: a new one)");
        }
        /* the menu: MENU > SEQ > STEP ADVANCE */
        {
            ui.menu = 1; ui.menu_sel = MI_ADV; ui.force = 1; frame(); ppm("clipper-menu-seq");
            check(mi_sec(MI_ADV) == MS_SEQ && !strcmp(MS_NAME[MS_SEQ], "SEQ") && !strcmp(MI_NAME[MI_ADV], "STEP ADVANCE"),
                  "MENU: a SEQ section (STEP ADVANCE), between AUDIO and MIDI");
            encs[panel.enc[EN_K1]] = -1; frame();
            ok = step_adv_off == 1u;
            encs[panel.enc[EN_K1]] = 1; frame();
            ok &= step_adv_off == 0u;
            ui.menu = 0; ui.force = 1; frame();
            check(ok, "MENU > SEQ: KNOB 1 STEP ADVANCE OFF / ON (kept: synth9_test)");
        }
        go_home(); frames(2);
    }
    /* the pages a synth keeps */
    check(!page_named("SLICER") && !page_named("DRUMS") && !page_named("SONG") && page_named("MASTER") && page_named("VOICE 2"),
          "pages: no SLICER, DRUMS or SONG; MASTER and VOICE 2 stay");
    {
        uint32_t k, duck = 0, mute = 0;
        for (i = 0; i < NPAGES; i++)
            for (k = 0; k < 4u; k++) {
                if (PAGES[i].scope == SC_GLOBAL && PAGES[i].id[k] != 0xFF &&
                    (!strcmp(GP[PAGES[i].id[k]].label, "DUCK") || !strcmp(GP[PAGES[i].id[k]].label, "ROLL")))
                    duck++;                          /* (their slots are the reverb's TYPE and PRE: G_RTYPE, G_RPRE) */
                if (PAGES[i].scope == SC_TRACK && PAGES[i].id[k] == P_MUTE)
                    mute++;
            }
        check(!duck && !mute, "no DUCK, ROLL or MUTE on any page");
        check(page_named("REVERB") && page_named("CHORUS") && !strcmp(GP[G_RTYPE].label, "TYPE") &&
              !strcmp(GP[G_RPRE].label, "PRE"), "REVERB (TYPE SIZE DAMP PRE) and CHORUS pages");
        check(!strcmp(TP[P_FMODEL].label, "MODL") && page_named("FILTER"), "FILTER page: the filter MODEL (ANALOG's ladder)");
    }
    {   /* every page of every button, walked: no SONG / DRUMS / TRACKS screen on the way */
        static const uint32_t FAMS[] = {FAM_ENV, FAM_LFO, FAM_FX, FAM_SCL, FAM_EDIT, FAM_GLO, FAM_SAVE, FAM_ARP, FAM_SEQ};
        uint32_t f, n, bad = 0;
        char nm[32];
        for (f = 0; f < NELEM(FAMS); f++)
            for (n = 0; n < 8u; n++) {
                open_family(FAMS[f]); ui.force = 1; frame();
                bad += cur_page()->scope == SC_SONG || cur_page()->scope == SC_DRUM || cur_page()->scope == SC_TRK;
                if (n < 4u) {
                    snprintf(nm, sizeof nm, "clipper-page-%u-%u", f, n);
                    ppm(nm);
                }
            }
        check(!bad, "the pages of every button: never a SONG, DRUMS or TRACKS screen");
    }
    /* HOME again: Clipper 0.3 has no full-screen visualiser: still the synth screen */
    go_home(); frames(2);
    tap(B_HOME); frames(3);
    check(!vis_shown() && ui.home, "HOME on HOME: still the synth screen (no visualiser)");
    tap(B_HOME); frames(3);
    check(!vis_shown() && ui.home, "HOME again: the synth screen");
    /* REC: armed, the screen shows the instrument alone; held: the pattern cleared */
    press(B_REC); release(B_REC); frames(4);
    ui.force = 1; frame(); ppm("clipper-rec");
    check(rec_wait, "REC: armed");
    press(B_REC); release(B_REC); frames(4);
    {
        uint8_t n[1] = {64};
        put_step(TSEL, 3, 1, n, ST_NOTE, 0);
        press(B_REC); frames(200);
        check(!trk_step_on(TSEL, 3) && !strncmp(ui.msg, "PATTERN", 7), "REC held: PATTERN CLEARED");
        release(B_REC); frames(4);
    }
    /* the header: ARP when the arpeggiator runs */
    TSEL->p[P_AMODE] = 1; ui.force = 1; frames(2); ppm("clipper-home-arp"); TSEL->p[P_AMODE] = 0; frames(2);
    {   /* (1.0) the arp a preset brings is the sound's: the next sound turns it off; the player's own runs on */
        track_t *t = TSEL;
        uint32_t ea = ENGI_ACID, pa = 0, k, eb = 0;
        for (k = 0; k < ENGINES[ea]->npresets; k++)
            if (!strcmp(ENGINES[ea]->presets[k].name, "ACID ARP"))
                pa = k;
        t->p[P_AMODE] = 0;
        set_engine_of(t, ea); apply_preset_to(t, pa); frames(2);
        check(t->p[P_AMODE] != 0, "ACID ARP: its arp on");
        set_engine_of(t, eb); apply_preset_to(t, 0); frames(2);
        check(t->p[P_AMODE] == 0, "the next sound: the preset's arp off again (1.0: it ran on through every sound)");
        set_engine_of(t, eb); apply_preset_to(t, 1); frames(2);
        check(t->p[P_AMODE] == 0, "and the one after: still off");
        t->p[P_AMODE] = 3; frames(2);                   /* (the player turns it on) */
        apply_preset_to(t, 2); frames(2);
        check(t->p[P_AMODE] == 3, "the player's arp: runs on through a sound change (0.7)");
        set_engine_of(t, ea); apply_preset_to(t, pa); frames(2);
        check(t->p[P_AMODE] == 3, "the player's arp: ACID ARP leaves it as it is");
        t->p[P_AMODE] = 0; frames(2);
        set_engine_of(t, ea); apply_preset_to(t, pa); frames(2);
        t->p[P_AMODE] = 5; frames(2);                   /* (the preset's arp, changed by the player: now the player's) */
        set_engine_of(t, eb); apply_preset_to(t, 0); frames(2);
        check(t->p[P_AMODE] == 5, "a preset's arp the player changed: the player's, runs on");
        t->p[P_AMODE] = 0; frames(2);
        set_engine_of(t, eb); apply_preset_to(t, 0); frames(2);
    }
    printf("clipper_ui: %s\n", fails ? "FAILED" : "all passed");
    return fails != 0;
}
