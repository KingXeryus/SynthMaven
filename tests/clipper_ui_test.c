/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper's synth interface (CLIPPER_SYNTH 1), on the harness of tests/ui_pages_test.c (the real UI on a
 * framebuffer, the audio running between frames):
 *   power-on: the instrument plays CLIPPER; HOME is the synth screen (no tracks; 0.3: no visualiser)
 *   PRESETS browses the sounds, ALGORITHM jumps from kind to kind (and back); not on a page that edits the sound
 *   one instrument: an old project / the editor selecting another track comes back to track 1; tracks 2..4 and the
 *   drums are silent even with patterns and notes; MIDI plays the instrument on every channel (omni, 10 too)
 *   no groovebox layer: FX / EDIT / ARP / GLO / SAVE held, the keys still play; tapped (or held long), their pages;
 *   SAVE opens the SAVE pages (no song); SEQ and SEL keep their layers (steps; the key)
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

    /* the buttons with no layer: held, the keys play; their pages on release */
    {
        static const struct { uint32_t b, fam; const char *n; } B[] = {
            {B_FX, FAM_FX, "FX"}, {B_EDIT, FAM_EDIT, "EDIT"}, {B_ARP, FAM_ARP, "ARP"}, {B_GLO, FAM_GLO, "GLO"},
            {B_SAVE, FAM_SAVE, "SAVE"}};
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
    printf("clipper_ui: %s\n", fails ? "FAILED" : "all passed");
    return fails != 0;
}
