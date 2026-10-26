/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 3dSam */
/* SynthMaven: the editor's HYBRID commands (editor_hybrid.c, protocol v11), through hostsim.c:
 *   HY_GET of a factory patch gives its 128 bytes; of a part, the patch it plays (or the one sent and not played
 *   yet); HY_PUT makes it the part's own from its next block, put into range (the bytes of a file or of another
 *   editor may be anything); bad requests answer rc 1 and change nothing. */
#define main hostsim_main
#include "hostsim.c"
#undef main
static struct { uint8_t force; } ui;
static uint8_t ed_out[600];
static uint32_t ed_n;
static void ed_b(uint32_t v) { if (ed_n < sizeof ed_out) ed_out[ed_n++] = (uint8_t)(v & 0x7Fu); }
static uint32_t ed_unpack7(const uint8_t *a, uint32_t na, uint8_t *out, uint32_t max)
{
    uint32_t n = 0;
    while (na && n < max) {
        uint32_t m = *a++, j;
        na--;
        for (j = 0; j < 7u && na && n < max; j++, na--)
            out[n++] = (uint8_t)(*a++ | ((m >> j) & 1u) << 7);
    }
    return n;
}
static void ed_pack7(const uint8_t *p, uint32_t n)
{
    while (n) {
        uint32_t k = n > 7u ? 7u : n, m = 0, i;
        for (i = 0; i < k; i++)
            m |= (uint32_t)(p[i] >> 7) << i;
        ed_b(m);
        for (i = 0; i < k; i++)
            ed_b(p[i] & 127u);
        p += k;
        n -= k;
    }
}
#include "../firmware/src/editor_hybrid.c"

static int fails;
static void check(int ok, const char *what) { printf("hy_editor: %-80s %s\n", what, ok ? "ok" : "FAIL"); fails += !ok; }

static uint8_t req[400];
/* a command with the arguments a[0..n) (and pack7 of p[0..np) after them): the reply's length */
static uint32_t cmd(uint32_t c, const uint8_t *a, uint32_t n, const void *p, uint32_t np)
{
    uint32_t i, k = n;
    const uint8_t *q = p;
    for (i = 0; i < n; i++)
        req[i] = a[i];
    while (np) {                                         /* pack7 into req */
        uint32_t g = np > 7u ? 7u : np, m = 0;
        for (i = 0; i < g; i++)
            m |= (uint32_t)(q[i] >> 7) << i;
        req[k++] = (uint8_t)m;
        for (i = 0; i < g; i++)
            req[k++] = q[i] & 127u;
        q += g;
        np -= g;
    }
    ed_n = 0;
    return ed_hy_handle(c, req, k) ? ed_n : 0;
}

static void block(void)
{
    static int32_t b[CTL];
    track_render(&trk[0], b, CTL);
}

int main(int argc, char **argv)
{
    hy_patch_t p, got;
    uint32_t n, i, k;
    if (argc > 1) {                                    /* the factory patches for web/test_web.mjs */
        FILE *f = fopen(argv[1], "w");
        if (!f)
            return 1;
        fprintf(f, "{\"size\": %u, \"factory\": [", (unsigned)sizeof(hy_patch_t));
        for (i = 0; i < HY_NFACTORY; i++) {
            const uint8_t *b = (const uint8_t *)&HY_FACTORY[i];
            fprintf(f, "%s[", i ? ", " : "");
            for (k = 0; k < sizeof(hy_patch_t); k++)
                fprintf(f, "%s%u", k ? "," : "", b[k]);
            fprintf(f, "]");
        }
        fprintf(f, "], \"names\": [");
        for (i = 0; i < HY_NFACTORY; i++)
            fprintf(f, "%s\"%s\"", i ? ", " : "", N_HY_PATCH[i]);
        fprintf(f, "]}\n");
        fclose(f);
    }
    host_tracks_init();
    host_preset(&trk[0], ENGI_HYBRID, 0);
    block();
    {
        static const uint8_t a[2] = {ED_HY_FACTORY, 5};
        n = cmd(ED_HY_GET, a, 2, 0, 0);
        check(n > 3u && ed_out[0] == 1 && ed_out[1] == 5 && ed_out[2] == 0 &&
                  ed_unpack7(ed_out + 3, n - 3u, (uint8_t *)&got, sizeof got) == sizeof got &&
                  !memcmp(&got, &HY_FACTORY[5], sizeof got),
              "HY_GET factory 5: its 128 bytes");
    }
    {
        static const uint8_t a[2] = {ED_HY_TRACK, 0};
        n = cmd(ED_HY_GET, a, 2, 0, 0);
        check(n > 3u && ed_out[2] == 0 && ed_unpack7(ed_out + 3, n - 3u, (uint8_t *)&got, sizeof got) == sizeof got &&
                  !memcmp(&got, &HY_FACTORY[0], sizeof got),
              "HY_GET part 1: the patch it plays (PTCH 0: SYNTHMAVEN)");
    }
    {
        static const uint8_t a[2] = {ED_HY_TRACK, 0};
        p = HY_FACTORY[3];
        p.osc[1].type = HO_FM;
        p.osc[1].coarse = -12;
        p.flt[1].env = -30;
        p.mod[7] = (hy_slot_t){HS_MW, HS_AT, HD_CUT2, -40};
        memcpy(p.name, "MY ACID   ", 10);
        n = cmd(ED_HY_PUT, a, 2, &p, sizeof p);
        check(n == 3u && ed_out[2] == 0, "HY_PUT part 1: rc 0");
        n = cmd(ED_HY_GET, a, 2, 0, 0);
        check(n > 3u && ed_unpack7(ed_out + 3, n - 3u, (uint8_t *)&got, sizeof got) == sizeof got && !memcmp(&got, &p, sizeof p),
              "HY_GET right after: the patch sent (not played yet)");
        block();
        check(!memcmp(&hy_cur[0], &p, sizeof p), "the next block plays it (negative values intact)");
    }
    {   /* out of range bytes: put into range */
        static const uint8_t a[2] = {ED_HY_TRACK, 0};
        memset(&p, 0xEE, sizeof p);
        n = cmd(ED_HY_PUT, a, 2, &p, sizeof p);
        block();
        check(n == 3u && ed_out[2] == 0 && hy_cur[0].osc[0].type < HO_N && hy_cur[0].flt[0].type < HF_N &&
                  hy_cur[0].mod[0].dst < HD_N && hy_cur[0].mod[0].src < HS_N && hy_cur[0].route < HR_N &&
                  hy_cur[0].osc[0].coarse >= -48 && hy_cur[0].bend <= 24u,
              "HY_PUT of 0xEE bytes: every field in range");
    }
    {   /* bad requests */
        static const uint8_t a1[2] = {ED_HY_TRACK, NPART}, a2[2] = {ED_HY_FACTORY, HY_NFACTORY}, a3[2] = {ED_HY_FACTORY, 0};
        hy_patch_t before = hy_cur[0];
        uint8_t g0 = hy_gen[0];
        n = cmd(ED_HY_GET, a1, 2, 0, 0);
        check(n == 3u && ed_out[2] == 1, "HY_GET of drum part: rc 1");
        n = cmd(ED_HY_GET, a2, 2, 0, 0);
        check(n == 3u && ed_out[2] == 1, "HY_GET past last factory patch: rc 1");
        n = cmd(ED_HY_PUT, a3, 2, &p, sizeof p);
        check(n == 3u && ed_out[2] == 1, "HY_PUT to a factory patch: rc 1");
        n = cmd(ED_HY_PUT, a1, 2, &p, 60);
        check(n == 3u && ed_out[2] == 1 && hy_gen[0] == g0 && !memcmp(&hy_cur[0], &before, sizeof before),
              "HY_PUT short or to drum part: rc 1, nothing changes");
    }
    check(!cmd(76, 0, 0, 0, 0), "other commands are not HYBRID's");
    {   /* PTCH turned: the factory patch again */
        trk[0].p[P_E3] = 2;
        block();
        check(!memcmp(&hy_cur[0], &HY_FACTORY[2], sizeof(hy_patch_t)), "PTCH turned after a HY_PUT: the factory patch");
    }
    printf("hy_editor: %s\n", fails ? "FAILED" : "all passed");
    return fails != 0;
}
