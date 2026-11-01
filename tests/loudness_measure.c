/* SPDX-License-Identifier: GPL-3.0-only */
/* Loudness of every factory sound and program, as momentary loudness (a K-style weighting, the loudest 400 ms) of one held
 * note at velocity 100: the numbers tools/gen_loud_comps.py turns into firmware/src/loudness_comp.h.
 *   loudness_measure sounds | programs | check */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
#include <assert.h>
#define LOUDNESS_TARGET (-11.14)                   /* tools/gen_loudness_comp.py TARGET */
typedef struct { double hx, hy, sx, sy; } kw_t;
static double kw(kw_t *f, double x)
{
    double y = 0.9915 * (f->hy + x - f->hx), s;    /* high-pass near 60 Hz */
    f->hx = x; f->hy = y;
    s = f->sy + 0.19 * (y - f->sy);                /* +4 dB above about 1.5 kHz */
    f->sy = s;
    return s + 1.585 * (y - s);
}
static double loudness(unsigned note, unsigned *peak_out)
{
    kw_t l = {0}, r = {0};
    int32_t out[2 * CTL];
    unsigned win = FS * 4 / 10 / CTL, blocks = FS * 3 / CTL, peak = 0, head = 0, n = 0;
    static double ring[8000];
    double sum = 0, best = 0;
    assert(win < 8000);
    memset(ring, 0, sizeof ring);
    mix_block(out, CTL);                           /* (as the factory audio test: the engines settle a block first) */
    midi_event_source = 1; midi_event(0x90, 0, note, 100);
    for (unsigned b = 0; b < blocks; b++) {
        double e = 0;
        mix_block(out, CTL);
        for (unsigned i = 0; i < CTL; i++) {
            double a = kw(&l, out[2 * i] / 32768.0), c = kw(&r, out[2 * i + 1] / 32768.0);
            unsigned p = (unsigned)abs(out[2 * i]), q = (unsigned)abs(out[2 * i + 1]);
            if (p > peak) peak = p;
            if (q > peak) peak = q;
            e += (a * a + c * c) / 2;
        }
        e /= CTL;
        sum += e - ring[head]; ring[head] = e; head = (head + 1) % win; n++;
        if (n >= win && sum / win > best) best = sum / win;
    }
    midi_event(0x80, 0, note, 0); midi_event_source = 0;
    for (unsigned b = 0; b < FS / CTL; b++) mix_block(out, CTL);
    *peak_out = peak;
    return 10 * log10(best + 1e-12);
}
int main(int argc, char **argv)
{
    unsigned peak;
    int check = argc > 1 && !strcmp(argv[1], "check");
    unsigned checked = 0, near = 0, bad = 0;
    if (check || (argc > 1 && !strcmp(argv[1], "programs"))) {
        for (unsigned s = 0; s < 8; s++) {
            /* Unison voices start at random phases, so one press can measure several dB low: take the loudest of a few. */
            double v = -200; unsigned top = 0;
            for (unsigned take = 0; take < 6; take++) {
                reset(); assert(!sm_factory_program(s));
                for (unsigned k = 0; k < 4; k++) { trk[k].engine = trk[k].eng_req; if (!eng_state_prepare(&trk[k])) return 2; }
                loudness_comp_service();
                double one = loudness(60, &peak);
                if (one > v) v = one;
                if (peak > top) top = peak;
            }
            peak = top;
            if (check) {
                if (fabs(v - LOUDNESS_TARGET) > 1.0 || peak >= 32767) { printf("program %s: %.2f dB, peak %u\n", sm_program_name, v, peak); bad++; }
            } else
                printf("%u\t%s\t%.2f\t%u\t%u\n", s, sm_program_name, v, peak, sm_layers.enabled);
        }
        if (!check) return 0;
    }
    for (unsigned e = 0; e < NENGINES; e++) {
        const engine_t *en = ENGINES[e];
        if (e == ENGI_MIDI) continue;
        for (unsigned p = 0; p < en->npresets; p++) {
            reset();
            sm_layers.enabled = 0; song.sel = 0;
            for (unsigned k = 1; k < 4; k++) trk[k].p[P_MUTE] = 1;
            set_engine_of(&trk[0], e); apply_preset_to(&trk[0], p); trk[0].engine = trk[0].eng_req;
            if (!eng_state_prepare(&trk[0])) { printf("%u\t%u\t%s\tSKIP\t0\n", e, p, en->name); continue; }
            loudness_comp_service();
            double v = loudness(e == ENGI_DRUM ? 38 : 60, &peak);
            if (check) {
                checked++; near += fabs(v - LOUDNESS_TARGET) <= 3.0;
                if (peak >= 32767) { printf("%s %u: at the rail\n", en->name, p); bad++; }
            } else {
                printf("%u\t%u\t%s\t%.2f\t%u\n", e, p, en->name, v, peak); fflush(stdout);
            }
        }
    }
    if (check) {
        printf("Loudness: %u of %u factory sounds within 3 dB of the DX7 program, programs within 1 dB\n", near, checked);
        return bad || near * 100u < checked * 90u;
    }
    return 0;
}
