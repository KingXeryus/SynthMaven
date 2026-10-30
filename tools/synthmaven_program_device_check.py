#!/usr/bin/env python3
"""Measure the eight factory programs through the real layered MIDI path.

Loads programs into working memory without explicitly saving them. Existing
autosave behavior is unchanged. Use after backing up the device.
"""
import argparse
import json
import time
from pathlib import Path
import wave
import mido
from prophet_device import Device, pack
from usb_audio_stats import snapshot


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='Melodee FM-1')
    parser.add_argument('--capture-dir', type=Path)
    parser.add_argument('--drums',action='store_true',help='Run independent drums alongside every layered program')
    args = parser.parse_args()
    device = Device(args.port)
    failures = []
    if args.capture_dir:
        import sounddevice as sd
        import numpy as np
        audio_device = next(i for i, d in enumerate(sd.query_devices())
                            if d['name'] == args.port and d['max_input_channels'] >= 2)
        args.capture_dir.mkdir(parents=True, exist_ok=True)
    try:
        info = device.request(1)
        if 112 not in info:
            raise RuntimeError('Connected firmware lacks the program API')
        if args.drums:
            masks=[0x1111,0x1010,0x5555,0x8000,0x1010,0x0400,0x0040,0x4000]
            config=bytes(sum(([m&255,m>>8] for m in masks),[])+[90,75,55,45,55,50,40,45]+[96]*8+[80,0])
            assert device.request(116,[1]+pack(config))[:2]==[1,0]
        for slot in range(8):
            if args.drums:assert device.request(116,[2,0])[:2]==[2,0]
            device.silence()
            reply = device.request(112, [1, slot])
            if reply[:3] != [1, 0, slot]:
                raise RuntimeError(f'Program {slot + 1} load failed: {reply[:5]}')
            time.sleep(.6)
            if args.drums:assert device.request(116,[2,1])[:2]==[2,0]
            for notes in ([48], [48, 52, 55]):
                device.silence()
                before = snapshot(device.incoming, device.outgoing, True, True, True, True, True, True, True)
                if args.capture_dir:
                    audio = sd.rec(44100 * 4, samplerate=44100, channels=2,
                                   dtype='int16', device=audio_device)
                    time.sleep(.15)
                for note in notes:
                    device.outgoing.send(mido.Message('note_on', note=note, velocity=100))
                time.sleep(3)
                after = snapshot(device.incoming, device.outgoing, True, True, True, True, True, True, True)
                device.silence()
                capture = {}
                if args.capture_dir:
                    status = sd.wait()
                    path = args.capture_dir / f'P{slot+1}-{len(notes)}keys.wav'
                    with wave.open(str(path), 'wb') as wav:
                        wav.setnchannels(2); wav.setsampwidth(2); wav.setframerate(44100)
                        wav.writeframes(audio.astype('<i2').tobytes())
                    values = audio.astype(np.float64)
                    peak = float(np.abs(values).max())
                    capture = dict(peak=peak, rms_dbfs=float(20*np.log10(max(1e-9, np.sqrt(np.mean(values**2)))/32768)),
                                   rail_samples=int(np.sum(np.abs(values)>=32767)), status=str(status or 'ok'))
                    if peak < 50 or capture['rail_samples'] or status:
                        failures.append((slot+1, len(notes), 'capture'))
                counters = ('audio_deadline_misses', 'core1_faults', 'core1_timeouts', 'resource_failures')
                delta = {key: after[key] - before[key] for key in counters}
                result = dict(program=slot+1, keys=len(notes), drums=args.drums, deltas=delta, stats=after, capture=capture)
                print(json.dumps(result), flush=True)
                if any(delta.values()) or not after['core1_online']:
                    failures.append((slot+1, len(notes)))
        print(json.dumps(dict(failed=failures)), flush=True)
    finally:
        try:
            device.silence()
            if args.drums:device.request(116,[2,0])
            device.request(112, [1, 0])
        finally:
            device.close()
    return bool(failures)


if __name__ == '__main__':
    raise SystemExit(main())
