/* SPDX-License-Identifier: GPL-3.0-only */
/* Builds assets/factory/programs-3.5.bin with the firmware's own preset and capture code:
 * cc -O1 -w -Ibuild/gen -Ifirmware/src -Itests tools/make_factory_programs.c -lm, run from the repo root.
 * Then python3 tools/gen_factory_program_pack.py. Layer 1 is the lead; layering starts off.
 * Vital sources (assets/factory/vital) are approximate translations made by the editor's importer. */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
#include <assert.h>
typedef struct { uint8_t engine, preset, level; int8_t pan, trans; uint8_t send, low, high; const char *vital; } part_def_t;
typedef struct { const char *name; uint8_t chorus; part_def_t part[4]; } program_def_t;
enum { E_FM6 = ENGI_FM6, E_HY = ENGI_HYBRID, E_P5 = ENGI_PROPHET, E_AN = 23, E_DG = 24, E_STR = 26, E_JU = 27 };
static const program_def_t PROGRAMS[8] = {
 {"DX7 E.PIANO", 1, {{E_FM6,35,110,0,0,30,0,127},{E_STR,14,80,-20,0,40,0,127},{E_AN,13,76,20,0,40,0,127},{E_FM6,8,83,0,12,30,48,127}}},
 {"DX RHODES", 1, {{E_DG,1,110,0,0,28,0,127},{E_JU,4,77,-18,0,35,0,127},{E_FM6,0,69,18,0,30,0,127},{E_HY,23,80,0,0,35,0,127,"RHODEX"}}},
 {"JUNO 106", 1, {{E_JU,0,108,0,0,22,0,127},{E_JU,3,73,-22,0,30,0,127},{E_STR,0,74,22,0,35,0,127},{E_HY,37,75,0,-12,25,0,72}}},
 {"E. ORGAN", 1, {{E_FM6,41,108,0,0,22,0,127},{E_FM6,14,87,-18,0,25,0,127},{E_JU,13,71,18,0,25,0,127},{E_HY,88,115,0,0,25,0,127}}},
 {"DISCO LEAD", 1, {{E_HY,78,106,0,0,26,0,127},{E_STR,0,79,-22,0,35,0,127},{E_JU,1,74,22,0,28,0,127},{E_AN,8,93,0,-12,0,0,59}}},
 {"PROPHET LEAD", 1, {{E_P5,82,108,0,0,26,0,127},{E_AN,27,73,-16,0,28,0,127},{E_HY,23,77,16,0,28,0,127,"MOOG"},{E_P5,102,61,0,-12,32,0,127}}},
 {"RETRO SYNTH", 1, {{E_HY,39,106,0,0,26,0,127},{E_AN,12,63,-20,0,30,0,127},{E_HY,23,127,20,0,30,0,127,"KEYS80"},{E_AN,22,87,0,-12,0,0,52}}},
 {"ORCHESTRA", 1, {{E_FM6,31,106,0,0,34,0,127},{E_STR,3,78,-20,0,42,0,127},{E_HY,78,84,20,0,36,48,127},{E_STR,13,75,0,-12,40,0,59}}},
};
static void build(const program_def_t *d, FILE *out)
{
    sm_layers = (sm_layers_t){.enabled = 0};
    for (unsigned k = 0; k < 4; k++) {
        const part_def_t *q = &d->part[k]; track_t *t = &trk[k];
        set_engine_of(t, q->engine); apply_preset_to(t, q->preset); t->engine = t->eng_req;
        if (q->vital) {
            char path[80]; hy_patch_t patch; snprintf(path, sizeof path, "assets/factory/vital/%s.hybrid", q->vital);
            FILE *f = fopen(path, "rb"); assert(f && sizeof patch == 128 && fread(&patch, 1, 128, f) == 128); fclose(f);
            hy_set_patch(k, &patch);
        }
        t->p[P_LEVEL] = q->level; t->p[P_PAN] = q->pan; t->p[P_TRANS] = q->trans; t->p[P_FXSEND] = q->send;
        t->p[P_MUTE] = 0; if (q->engine != E_P5) t->p[P_VOICE] = t->p[P_VOICE];
        sm_layers.low[k] = q->low; sm_layers.high[k] = q->high; track_fx_normalize(t->p);
    }
    /* A slight, slow chorus into a bright reverb: the "shimmer". There is no pitch-shifted reverb to use. */
    static const uint8_t HALL[4] = {FX_REVERB, 127, 66, 44}, CHORUS[4] = {FX_CHORUS, 45, 28, 64}, NONE[4] = {0, 127, 60, 64};
    for (unsigned i = 0; i < 4; i++) {
        song.g[G_MFXTYPE + i] = d->chorus ? CHORUS[i] : HALL[i];
        song.g[G_MFX2TYPE + i] = d->chorus ? HALL[i] : NONE[i];
        song.g[G_MFX3TYPE + i] = NONE[i];
    }
    song.g[G_FXRETURN] = 100; song.g[G_FXMODE] = 0; song.g[G_MFXTONE] = song.g[G_MFX3TONE] = 0; song.g[G_MFX2TONE] = 12;
    song.sel = 0; memset(sm_program_meta, 0, SM_PROGRAM_META);
    uint8_t meta[SM_PROGRAM_META]; sm_program_meta_capture(meta, 0); assert(sm_program_meta_valid(meta));
    project_capture(&proj_scratch); memset(proj_scratch.name, 0, sizeof proj_scratch.name);
    memcpy(proj_scratch.name, d->name, strlen(d->name)); assert(strlen(d->name) <= 12);
    assert(sm_program_pack_sound(&proj_scratch)); assert(SM_SOUND_BYTES + SM_PROGRAM_META == 3556);
    fwrite(proj_image_store(), 1, SM_SOUND_BYTES, out); fwrite(meta, 1, SM_PROGRAM_META, out);
}
int main(void)
{
    reset(); FILE *out = fopen("assets/factory/programs-3.5.bin", "wb"); assert(out);
    for (unsigned s = 0; s < 8; s++) build(&PROGRAMS[s], out);
    fclose(out); puts("eight factory programs written"); return 0;
}
