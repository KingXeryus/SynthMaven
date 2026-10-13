/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam (Clipper) */
/* Clipper 0.6, FMX on the FM-1 (the harness of tests/ui_pages_test.c):
 *   the FMX pages (OP, OP 2, OP ENV, ROUTE, FMX, STORE) come in the EDIT family on FMX only (another engine: EDIT
 *   walks past them; a sound leaving FMX on one of them: back to EDIT 1); each knob writes its operator's value into
 *   the part's patch (OP chooses which), the sound plays it from the next block; ROUTE writes mod[TO][FROM] and OUT,
 *   the diagonal reads FB; with a template on (ALG), changing the routing keeps the template's connections and turns
 *   ALG to PAT; a sound loaded: its patch again (the edits not stored go); SWAP: the patches trade places
 *   screens: DIR/fmx-*.ppm */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"

static void ok(int c, const char *what) { check(c, what); }
static int page_idx(const char *t)
{
    uint32_t i;
    for (i = 0; i < NPAGES; i++)
        if (!strcmp(PAGES[i].title, t))
            return (int)i;
    return -1;
}
static void go_page(const char *t)
{
    int i = page_idx(t);
    if (i < 0)
        return;
    open_family(PAGES[i].fam);
    ui.page = (uint8_t)i;
    ui.fam_last[PAGES[i].fam] = (uint8_t)i;
    page_entered();
    ui.force = 1;
    frame();
}
static void knob(uint32_t k, int32_t s) { encs[panel.enc[EN_K1 + k]] = s; frame(); }
/* the titles EDIT walks through, tapping EDIT again and again from its first page */
static uint32_t walk_edit(int *saw_op)
{
    uint32_t n = 0, first;
    *saw_op = 0;
    open_family(FAM_EDIT);
    first = ui.page;
    do {
        if (!strcmp(PAGES[ui.page].title, "OP"))
            *saw_op = 1;
        n++;
        open_family(FAM_EDIT);
    } while (ui.page != first && n < 40u);
    return n;
}

int main(int argc, char **argv)
{
    uint32_t i, n_other, n_fmx;
    int saw;
    track_t *t = &trk[0];
    outdir = argc > 1 ? argv[1] : "build/host";
    bank_resolve();
    panel = PANEL_DEFAULT;
    layers_init();
    settings.palette = 4;
    palette_set(4);
    host_tracks_init();
    for (i = 0; i < NPART; i++) { set_engine_of(&trk[i], i ? 1u : 0u); apply_preset_to(&trk[i], 0); trk[i].engine = trk[i].eng_req; }
    clipper_proj_neutral();
    go_home(); ui.force = 1; frames(3);

    n_other = walk_edit(&saw);
    ok(!saw, "FMX pages: not on another engine (EDIT walks past them)");
    set_engine_of(t, ENGI_FMX);
    apply_preset_to(t, 0);                              /* FMX EP */
    frames(4);
    n_fmx = walk_edit(&saw);
    ok(saw && n_fmx == n_other + 6u, "FMX pages: on FMX, six more pages in EDIT (OP, OP 2, OP ENV, ROUTE, FMX, STORE)");

    go_page("OP");
    ppm("fmx-op");
    knob(0, 1);                                         /* OP 1 -> 2 */
    knob(1, 2);                                         /* WAVE of operator 2: SINE -> SAW */
    ok(xv[X_FOP] == 2 && fmx_cur[0].op[1].wave == FXW_SAW && fmx_cur[0].op[0].wave == FXW_SINE,
       "OP: OP chooses the operator, WAVE writes its wave into the part's patch");
    knob(2, 1);                                         /* MODE: PM -> FM */
    knob(3, 3);                                         /* RATIO: 14 -> 17 */
    frames(2);
    ok(fmx_cur[0].op[1].mode == FXM_FM && fmx_cur[0].op[1].coarse == 17 && fmx_eff[0].mode[1] == FXM_FM,
       "OP: MODE and RATIO written, the sound plays them from the next block");
    go_page("OP ENV");
    knob(3, -10);
    ok(fmx_cur[0].op[1].adsr[3] == 40, "OP ENV: REL of operator 2 (50 -> 40)");
    ppm("fmx-openv");
    go_page("ROUTE");
    knob(0, 2), knob(1, 3);                             /* TO 3, FROM 5 */
    knob(2, 40);
    ok(fmx_cur[0].mod[2][4] == 40, "ROUTE: AMT writes mod[TO][FROM] (5 into 3)");
    ppm("fmx-route");
    {
        const param_desc_t *d;
        int16_t *vp;
        knob(1, -2);                                    /* FROM 3: TO = FROM, the feedback */
        d = page_desc(cur_page(), 2, &vp);
        ok(d && !strcmp(d->label, "FB"), "ROUTE: TO = FROM reads FB (an operator into itself)");
    }
    /* a template on (ALG 1), then a change of the routing: PAT, the template's connections kept */
    t->p[P_E1] = 1;
    frames(2);
    knob(0, -2), knob(1, 3);                            /* TO 1, FROM 6 (not in template 1) */
    knob(2, 30);
    ok(t->p[P_E1] == 0 && fmx_cur[0].mod[0][5] == 30 && fmx_cur[0].mod[0][1] == 100 && fmx_cur[0].mod[5][5] == 60 &&
       fmx_cur[0].mod[2][3] && fmx_cur[0].out[0] && fmx_cur[0].out[2] && !fmx_cur[0].out[1],
       "ROUTE with ALG 1 on: ALG to PAT, template 1's connections and carriers kept, the new one added");
    go_page("FMX");
    ppm("fmx-patch");
    go_page("STORE");
    ppm("fmx-store");
    knob(1, 1);                                         /* STORE (no flash in the harness) */
    ok(!xv[X_FSTORE], "STORE: the action back to 0 (here: no flash)");
    /* a sound loaded: its patch again */
    apply_preset_to(t, 0);
    frames(4);
    ok(fmx_cur[0].op[1].wave == FXW_SINE && fmx_cur[0].mod[2][4] == 0, "a sound loaded: its patch from PTCH again (the edits not stored go)");
    /* leaving FMX on one of its pages: back to EDIT 1 */
    go_page("OP 2");
    set_engine_of(t, 0);
    apply_preset_to(t, 0);
    frames(2);
    ok(strcmp(PAGES[ui.page].title, "OP 2") != 0 && PAGES[ui.page].fam == FAM_EDIT, "another engine on an FMX page: back to the EDIT pages");
    printf(fails ? "fmx_ui: %d FAILED\n" : "fmx_ui: all passed\n", fails);
    return fails;
}
