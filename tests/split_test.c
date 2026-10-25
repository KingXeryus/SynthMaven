/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* SynthMaven 0.7, the voices on both cores (voice.c track_render, vs_finish): every preset of every engine, a chord of
 * six notes at six velocities, held then released, rendered with the split (cpu1_units -1) and without (0): the same
 * output, bit for bit. Half the cases with a matrix slot moving an EDIT value (the late voices: after the wait; such a
 * part stays on this core), the others handed over (ACID: one voice, never). (1.0) Half with SPREAD (the voices'
 * side, rendered on either core), the matrix cases with the macros and LFO 3 in slot 7.
 * Run under every mode of the host double of core 1 (tests/cpu1_host.c): CORES=1 (inline at the wait), CORES=2 (a
 * thread; with -fsanitize=thread: no race), CHAOS=N (an erratic thread), CPU1_TIMEOUT_AT=K (a lost job: held, its
 * voices rendered here, once). Each case in a child process: the same state from power-on for both renders.
 *   split_test [MAXPRESETS] */
#define main hostsim_main
#include "hostsim.c"
#undef main

static int fails;
static uint32_t cases, split_cases;

static uint64_t render_case(uint32_t e, uint32_t pi, int matrix, int32_t units, uint32_t *jobs)
{
    static int32_t o[CTL * 2];
    static const uint8_t notes[6] = {48, 55, 60, 64, 67, 72};
    uint64_t h = 1469598103934665603ull;
    uint32_t b, i;
    host_tracks_init();
    for (i = 0; i < NPART; i++)
        host_preset(&trk[i], i ? 1u : e, i ? 0u : pi);
    synthmaven_proj_neutral();
    trk[1].p[P_LYMODE] = LY_OFF;
    trk[0].p[P_VOICE] = V_POLY;
    if (matrix) {                                   /* LFO 1 -> the engine's second EDIT value, and VEL -> its fifth */
        trk[0].p[P_M1SRC] = MS_LFO1, trk[0].p[P_M1DST] = MT_E1 + 1, trk[0].p[P_M1AMT] = 40;
        trk[0].p[P_M2SRC] = MS_VEL, trk[0].p[P_M2DST] = MT_E1 + 4, trk[0].p[P_M2AMT] = -30;
        trk[0].p[P_LRATE] = 90;
        trk[0].p[P_MAC1] = 30, trk[0].p[P_MAC3] = 90;   /* (1.0: the macros, LFO 3, slot 7) */
        trk[0].p[P_M7SRC] = MS_LFO3, trk[0].p[P_M7DST] = MT_E1 + 2, trk[0].p[P_M7AMT] = 25;
        trk[0].p[P_L3RATE] = 70;
    }
    if (pi & 2u)                                    /* (1.0) SPREAD: each voice's side, on either core */
        trk[0].p[P_SPREAD] = 110;
    cpu1_units = units;
    for (b = 0; b < 4u; b++)
        mix_block(o, CTL);
    for (i = 0; i < 6u; i++)
        trk_note_on(&trk[0], notes[i], 40u + 15u * i);
    for (b = 0; b < 260u; b++) {
        if (b == 180u)
            for (i = 0; i < 6u; i++)
                trk_note_off(&trk[0], notes[i]);
        mix_block(o, CTL);
        for (i = 0; i < 2u * CTL; i++)
            h = (h ^ (uint32_t)o[i]) * 1099511628211ull;
    }
    *jobs = vs_jobs;
    return h;
}

/* one render in a child: its hash and the blocks it handed over, through a pipe */
static int child_render(uint32_t e, uint32_t pi, int matrix, int32_t units, uint64_t *h, uint32_t *jobs)
{
    int fd[2];
    pid_t pid;
    struct { uint64_t h; uint32_t jobs; } r = {0, 0};
    if (pipe(fd))
        return -1;
    pid = fork();
    if (pid == 0) {
        close(fd[0]);
        r.h = render_case(e, pi, matrix, units, &r.jobs);
        if (write(fd[1], &r, sizeof r) != (ssize_t)sizeof r)
            _exit(2);
        _exit(0);
    }
    close(fd[1]);
    if (read(fd[0], &r, sizeof r) != (ssize_t)sizeof r)
        r.h = 0, r.jobs = 0xFFFFFFFFu;
    close(fd[0]);
    waitpid(pid, 0, 0);
    *h = r.h;
    *jobs = r.jobs;
    return 0;
}

int main(int argc, char **argv)
{
    uint32_t e, pi, maxp = argc > 1 ? (uint32_t)atoi(argv[1]) : 64u;
    for (e = 0; e < NENGINES; e++) {
        const engine_t *en = ENGINES[e];
        uint32_t np = en->npresets < maxp ? en->npresets : maxp, bad = 0, handed = 0;
        if (!vs_ok(en))
            continue;                               /* (the others never split: nothing to compare) */
        for (pi = 0; pi < np; pi++) {
            int matrix = (int)(pi & 1u);
            uint64_t a, b;
            uint32_t ja, jb;
            child_render(e, pi, matrix, 0, &a, &ja);
            child_render(e, pi, matrix, -1, &b, &jb);
            cases++;
            if (a != b || !a) {
                bad++;
                printf("split: %s preset %u (%s)%s%s: %016llx vs %016llx\n", en->name, pi, en->presets[pi].name,
                       matrix ? " +matrix" : "", (pi & 2u) ? " +spread" : "", (unsigned long long)a, (unsigned long long)b);
            }
            if (jb && jb != 0xFFFFFFFFu)
                handed++, split_cases++;
            if (ja)
                bad++, printf("split: %s preset %u: handed over with the split off (%u)\n", en->name, pi, ja);
        }
        if (!getenv("CPU1_TIMEOUT_AT") && en != &ENG_ACID && handed < np / 2u)
            bad++;                                  /* (every case without the matrix: its voices on both cores) */
        printf("split: %-8s %2u presets, the same output with and without the split, %2u handed over %s\n", en->name, np,
               handed, bad ? "FAIL" : "ok");
        fails += bad != 0;
    }
    printf("split: %u cases, %u with voices on core 1 (CORES=%d)\n", cases, split_cases, cpu1_host_mode());
    printf(fails ? "split: %d FAILED\n" : "split: all passed\n", fails);
    return fails;
}
