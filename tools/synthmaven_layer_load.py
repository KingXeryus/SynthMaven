#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""CPU and clipping of every program with all its layers sounding, on an FM-1.

Loads programs into working memory and turns layering on while it plays; layering
is put back at the end. Never saves a program. Back up the device first.
"""
import argparse, json, time
import mido
import numpy as np
import sounddevice as sd
from prophet_device import Device, pack, param_ids, unpack
from usb_audio_stats import snapshot

CHORDS = ((60,), (48, 60, 64), (48, 55, 60, 64, 67, 72), (36, 43, 48, 55, 60, 64, 67, 72, 76, 79))
HALF_US = 128 * 1000000 / 44100
COUNTERS = ('audio_deadline_misses', 'audio_late', 'voices_shed', 'voices_given_up', 'capacity_steals',
            'admission_rejections', 'core1_faults', 'core1_timeouts', 'resource_failures')


def stats(d):
    return snapshot(d.incoming, d.outgoing, True, True, True, True, True, True)


def layers(d, raw=None):
    r = d.request(111, [0] if raw is None else [1] + pack(raw))
    if r[1]:
        raise RuntimeError(f'Layers rejected: {r[:3]}')
    return unpack(r[2:])[:16]


def budget_id():
    return param_ids()[1]['G_BUDGET']


def set_budget(d, value):
    v = value + 8192
    r = d.request(3, [1, budget_id(), v & 127, v >> 7])
    print(json.dumps(dict(budget=r[2] + (r[3] << 7) - 8192)), flush=True)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--midi', default='Melodee FM-1')
    ap.add_argument('--audio', default='SynthMaven FM1')
    ap.add_argument('--hold', type=float, default=3.0)
    ap.add_argument('--slots', default='0,1,2,3,4,5,6,7')
    ap.add_argument('--velocity', type=int, default=110)
    ap.add_argument('--budget', type=int, help='Voice budget for this run; 64 is restored afterwards')
    a = ap.parse_args()
    audio = next(i for i, x in enumerate(sd.query_devices()) if x['name'] == a.audio and x['max_input_channels'] >= 2)
    d = Device(a.midi)
    original = layers(d)
    start = d.request(112, [0])[2]
    bad = 0
    try:
        if a.budget:
            set_budget(d, a.budget)
        for slot in map(int, a.slots.split(',')):
            d.silence()
            r = d.request(112, [1, slot])
            if r[:3] != [1, 0, slot]:
                raise RuntimeError(f'Load {slot + 1}: {r[:4]}')
            program = bytearray(layers(d))
            program[4] = 1
            layers(d, bytes(program))
            time.sleep(.5)
            for chord in CHORDS:
                d.silence()
                before = stats(d)
                take = sd.rec(int(44100 * (a.hold + .3)), samplerate=44100, channels=2, dtype='int16', device=audio)
                time.sleep(.1)
                for n in chord:
                    d.outgoing.send(mido.Message('note_on', note=n, velocity=a.velocity))
                time.sleep(a.hold)
                s = stats(d)
                for n in chord:
                    d.outgoing.send(mido.Message('note_off', note=n))
                sd.wait()
                x = take.astype(np.float64)
                row = dict(program=slot + 1, keys=len(chord), voices=s['voices_active'], cpu_pct=round(s['cpu_q8'] / 2.56),
                           peak_pct=round(s['audio_full_max_us'] / HALF_US * 100),
                           core1_max_us=s['core1_max_job_us'], peak_dbfs=round(20 * np.log10(max(np.abs(x).max(), 1) / 32768), 1),
                           rail=int(np.sum(np.abs(x) >= 32767)), **{k: s[k] - before[k] for k in COUNTERS if s[k] - before[k]})
                bad += bool(row['rail'] or row['peak_pct'] >= 100 or row.get('audio_deadline_misses') or row.get('voices_shed'))
                print(json.dumps(row), flush=True)
    finally:
        d.silence()
        d.request(112, [1, start if start < 8 else 0])
        set_budget(d, 64)
        layers(d, bytes(original))
        d.close()
    print(json.dumps(dict(problem_rows=bad)))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
