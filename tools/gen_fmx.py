#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 3dSam
"""FMX's tables (firmware/src/eng_fmx.c): the operators' waves and the algorithms. Written to
firmware/src/fmx_tables.h (committed: no build step). Usage: python3 tools/gen_fmx.py [out.h]

Waves, FMX_WN x 1024 int16 (peak 32767), each band-limited for the operators' pitch range it is used in:
  SINE (eng_fmx.c reads osc_sine instead: kept for the index), TRI (odd harmonics to the 31st), SAW and SQR (to the
  48th: the full one), SAW8 and SQR8 (to the 8th: the high notes, eng_fmx.c picks one by the operator's increment),
  HALF (a half-wave rectified sine, the OPL's), ABS (a rectified sine, one arch a period: at the note's pitch, not the
  octave up the OPL's has), QUART (a quarter-sine pulse a period, the same),
  PULSE (25 %, to the 48th) and PULSE8, SIN8 (a sine on 16 steps: lo-fi on purpose)
Algorithms, FMX_NALG: the 32 of the DX7, derived from fm6_core.c's FM6_ALG bus table (the same sounds as FM6), then
8 more (33..40). Each: per operator (1..6) the operators that modulate it (bit s-1 for operator s, its own bit a
feedback), and the carriers."""
import math
import sys

N = 1024
TAU = 2 * math.pi


def additive(amps):
    v = [sum(a * math.sin(TAU * k * i / N + ph) for k, a, ph in amps) for i in range(N)]
    pk = max(abs(x) for x in v)
    return [int(round(32767 * x / pk)) for x in v]


def shaped(f):
    v = [f(TAU * i / N) for i in range(N)]
    mean = sum(v) / N
    v = [x - mean for x in v]                       # (no DC: a modulator's DC is a pitch shift in FM mode)
    pk = max(abs(x) for x in v)
    return [int(round(32767 * x / pk)) for x in v]


def saw(nh):
    return additive([(k, 1.0 / k * (0.5 + 0.5 * math.cos(math.pi * k / (nh + 1))), 0) for k in range(1, nh + 1)])


def sqr(nh):
    return additive([(k, 1.0 / k * (0.5 + 0.5 * math.cos(math.pi * k / (nh + 1))), 0) for k in range(1, nh + 1, 2)])


def pulse(nh, w=0.25):
    return additive([(k, math.sin(math.pi * k * w) / k * (0.5 + 0.5 * math.cos(math.pi * k / (nh + 1))), -math.pi / 2)
                     for k in range(1, nh + 1)])


def tri(nh):
    return additive([(k, (1.0 if (k // 2) % 2 == 0 else -1.0) / (k * k), 0) for k in range(1, nh + 1, 2)])


WAVES = [
    ("SINE", shaped(math.sin)),
    ("TRI", tri(31)),
    ("SAW", saw(48)),
    ("SQR", sqr(48)),
    ("HALF", shaped(lambda x: max(math.sin(x), 0.0))),
    ("ABS", shaped(lambda x: abs(math.sin(x / 2)))),          # (one arch a period: at the note's pitch)
    ("QUART", shaped(lambda x: math.sin(2 * x) if x < math.pi / 2 else 0.0)),   # (a quarter-sine pulse a period)
    ("PULSE", pulse(48)),
    ("SIN8", [int(round(32767 * (max(-8, min(7, math.floor(8 * math.sin(TAU * i / N)))) + 0.5) / 7.5)) for i in range(N)]),
    ("NOISE", [0] * N),                             # (eng_fmx.c makes it: a table only for the index)
    ("SAW8", saw(8)),
    ("SQR8", sqr(8)),
    ("PULS8", pulse(8)),
]
NWAVE_USER = 10                                     # SINE .. NOISE: what a patch names (the rest: the "8" ones)

# fm6_core.c FM6_ALG: per operator, the sixth first: out bus (1, 2: low bits), add (4), in bus (16, 32), feedback in
# (64), feedback out (128)
FM6_ALG = [
    [0xc1, 0x11, 0x11, 0x14, 0x01, 0x14], [0x01, 0x11, 0x11, 0x14, 0xc1, 0x14], [0xc1, 0x11, 0x14, 0x01, 0x11, 0x14],
    [0xc1, 0x11, 0x94, 0x01, 0x11, 0x14], [0xc1, 0x14, 0x01, 0x14, 0x01, 0x14], [0xc1, 0x94, 0x01, 0x14, 0x01, 0x14],
    [0xc1, 0x11, 0x05, 0x14, 0x01, 0x14], [0x01, 0x11, 0xc5, 0x14, 0x01, 0x14], [0x01, 0x11, 0x05, 0x14, 0xc1, 0x14],
    [0x01, 0x05, 0x14, 0xc1, 0x11, 0x14], [0xc1, 0x05, 0x14, 0x01, 0x11, 0x14], [0x01, 0x05, 0x05, 0x14, 0xc1, 0x14],
    [0xc1, 0x05, 0x05, 0x14, 0x01, 0x14], [0xc1, 0x05, 0x11, 0x14, 0x01, 0x14], [0x01, 0x05, 0x11, 0x14, 0xc1, 0x14],
    [0xc1, 0x11, 0x02, 0x25, 0x05, 0x14], [0x01, 0x11, 0x02, 0x25, 0xc5, 0x14], [0x01, 0x11, 0x11, 0xc5, 0x05, 0x14],
    [0xc1, 0x14, 0x14, 0x01, 0x11, 0x14], [0x01, 0x05, 0x14, 0xc1, 0x14, 0x14], [0x01, 0x14, 0x14, 0xc1, 0x14, 0x14],
    [0xc1, 0x14, 0x14, 0x14, 0x01, 0x14], [0xc1, 0x14, 0x14, 0x01, 0x14, 0x04], [0xc1, 0x14, 0x14, 0x14, 0x04, 0x04],
    [0xc1, 0x14, 0x14, 0x04, 0x04, 0x04], [0xc1, 0x05, 0x14, 0x01, 0x14, 0x04], [0x01, 0x05, 0x14, 0xc1, 0x14, 0x04],
    [0x04, 0xc1, 0x11, 0x14, 0x01, 0x14], [0xc1, 0x14, 0x01, 0x14, 0x04, 0x04], [0x04, 0xc1, 0x11, 0x14, 0x04, 0x04],
    [0xc1, 0x14, 0x04, 0x04, 0x04, 0x04], [0xc4, 0x04, 0x04, 0x04, 0x04, 0x04]]


def dx7(a):
    """the modulation graph of a DX7 algorithm: mods[op] = set of modulating ops, carriers"""
    bus = {1: set(), 2: set()}
    mods = {o: set() for o in range(1, 7)}
    car = set()
    fb_in = fb_out = None
    for k, f in enumerate(a):
        op = 6 - k
        inb, outb, add = (f >> 4) & 3, f & 3, f & 4
        if f & 64:
            fb_in = op
        if f & 128:
            fb_out = op
        if inb:
            mods[op] |= bus[inb]
        if outb == 0:
            car.add(op)
        elif add:
            bus[outb] = bus[outb] | {op}
        else:
            bus[outb] = {op}
    if fb_in is not None and fb_out is not None:
        mods[fb_in].add(fb_out)
    return mods, car


def graph(edges, car):
    mods = {o: set() for o in range(1, 7)}
    for s, d in edges:
        mods[d].add(s)
    return mods, set(car)


EXTRA = [   # (name, (src, dst) edges, carriers)
    ("CASCADE", [(6, 6), (6, 5), (5, 4), (4, 3), (3, 2), (2, 1)], [1]),
    ("2 STACKS", [(6, 6), (3, 2), (2, 1), (6, 5), (5, 4)], [1, 4]),
    ("STAR", [(6, 6), (2, 1), (3, 1), (4, 1), (5, 1), (6, 1)], [1]),
    ("2 x 2>1", [(6, 6), (3, 1), (4, 1), (5, 2), (6, 2)], [1, 2]),
    ("RING 3", [(6, 6), (2, 1), (4, 3), (6, 5)], [1, 3, 5]),
    ("LOOP", [(2, 1), (1, 2), (4, 3), (3, 4), (6, 5), (5, 6)], [1, 3, 5]),
    ("Y", [(6, 6), (6, 5), (5, 4), (5, 3), (4, 1), (3, 2)], [1, 2]),
    ("1 > 5", [(6, 6), (6, 1), (6, 2), (6, 3), (6, 4), (6, 5)], [1, 2, 3, 4, 5]),
]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "firmware/src/fmx_tables.h"
    algs = [dx7(a) for a in FM6_ALG] + [graph(e, c) for _, e, c in EXTRA]
    # (the DX7's algorithm 1: 6 (feedback) > 5 > 4 > 3 carrier, 2 > 1 carrier)
    assert algs[0][0][3] == {4} and algs[0][0][4] == {5} and algs[0][0][5] == {6} and algs[0][0][6] == {6}
    assert algs[0][0][1] == {2} and algs[0][1] == {1, 3}
    assert algs[31][1] == {1, 2, 3, 4, 5, 6} and algs[31][0][6] == {6}
    lines = ["/* generated by tools/gen_fmx.py: FMX's waves and algorithms (eng_fmx.c) */",
             "#pragma once",
             "#define FMX_TN %du" % N,
             "#define FMX_WN %du" % len(WAVES),
             "#define FMX_NWAVE %du                    /* the waves a patch names: SINE .. NOISE */" % NWAVE_USER,
             "static const char *const N_FMX_WAVE[FMX_NWAVE] = {%s};" % ", ".join('"%s"' % n for n, _ in WAVES[:NWAVE_USER]),
             "static const int16_t FMX_WAVE[FMX_WN][FMX_TN] = {"]
    for name, t in WAVES:
        lines.append("    {   /* %s */" % name)
        for i in range(0, N, 16):
            lines.append("        " + ", ".join("%d" % x for x in t[i:i + 16]) + ",")
        lines.append("    },")
    lines.append("};")
    lines.append("#define FMX_NALG %du" % len(algs))
    lines.append("/* per algorithm: [0..5] the modulators of operators 1..6 (bit s-1: operator s; its own bit: feedback),")
    lines.append(" * [6] the carriers (bit o-1) */")
    lines.append("static const uint8_t FMX_ALG[FMX_NALG][7] = {")
    for i, (mods, car) in enumerate(algs):
        row = [sum(1 << (s - 1) for s in mods[o]) for o in range(1, 7)] + [sum(1 << (o - 1) for o in car)]
        lines.append("    {%s},   /* %d%s */" % (", ".join("0x%02x" % x for x in row), i + 1,
                                                 "" if i < 32 else " " + EXTRA[i - 32][0]))
    lines.append("};")
    lines.append("static const char *const N_FMX_ALG[FMX_NALG + 1u] = {\"PAT\", %s};" %
                 ", ".join('"%d"' % (i + 1) for i in range(len(algs))))
    # DX7 fixed frequencies: 10^(fine / 100), Q16 (FMX's FIX mode: 1, 10, 100, 1000 Hz x this)
    lines.append("static const uint32_t FMX_P10[100] = {")
    p10 = [int(round(65536 * 10 ** (k / 100.0))) for k in range(100)]
    for i in range(0, 100, 10):
        lines.append("    " + ", ".join("%d" % x for x in p10[i:i + 10]) + ",")
    lines.append("};")
    # operator LEVEL 0..127 -> Q15 gain: 0.75 dB a step from 127 (0: silent)
    lv = [0] + [int(round(32767 * 10 ** (-(127 - k) * 0.75 / 20))) for k in range(1, 128)]
    lines.append("static const uint16_t FMX_LVL[128] = {")
    for i in range(0, 128, 16):
        lines.append("    " + ", ".join("%d" % x for x in lv[i:i + 16]) + ",")
    lines.append("};")
    with open(out, "w") as fh:
        fh.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
