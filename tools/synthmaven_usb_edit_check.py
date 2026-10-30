#!/usr/bin/env python3
"""Exercise 3.5 tuning, named programs and Learn through the actual USB protocol.

For a backed-up development candidate only. Saves the unchanged P1 recipe into
P1, restores original tuning/assignments, and leaves P1 selected.
"""
import json
import time
import mido
from prophet_device import Device, pack, unpack


def main():
    device = Device('Melodee FM-1')
    original_tuning = original_maps = None
    checks = []
    try:
        device.silence()
        original_tuning = unpack(device.request(110, [0])[2:])
        original_maps = unpack(device.request(113, [0])[33:])
        assert len(original_tuning) == 28 and len(original_maps) == 288
        tuning = bytearray(original_tuning)
        tuning[4:10] = bytes([1, 47, 0, 3, 1, 15])
        tuning[10] = 219  # -37 cents on C
        tuning[22:24] = bytes([255, 15])
        assert device.request(110, [1] + pack(tuning))[:2] == [1, 0]
        for slot in (0, 7, 3, 0):
            assert device.request(112, [1, slot])[:3] == [1, 0, slot]
            assert unpack(device.request(110, [0])[2:]) == tuning
        checks.append('global custom tuning survives four program changes')
        assert device.request(112, [2, 0, 0] + list(b'DX7 STAGE'))[:3] == [2, 0, 0]
        assert device.request(112, [1, 1])[:3] == [1, 0, 1]
        assert device.request(112, [1, 0])[:3] == [1, 0, 0]
        checks.append('named P1 save and reload')
        assert device.request(113, [1, 1])[:2] == [1, 0]
        device.outgoing.send(mido.Message('control_change', control=85, value=127))
        time.sleep(.1)
        learned = device.request(113, [0])
        assert learned[3] == 2
        checks.append('Learn captures a live USB CC')
        record = bytes([1, 0, 0, 85, 127, 0, 4, 2, 100, 0, 255, 0])
        assert device.request(113, [2, 23] + pack(record))[:2] == [2, 0]
        device.outgoing.send(mido.Message('control_change', control=85, value=127))
        time.sleep(.4)
        assert device.request(112, [0])[2] == 1
        assert device.request(113, [0])[4] == 23
        checks.append('accepted mapping triggers Program Up and reports a match')
        print(json.dumps(dict(passed=checks)), flush=True)
    finally:
        try:
            device.request(113, [1, 0])
            if original_maps is not None:
                assert device.request(113, [2, 23] + pack(original_maps[276:288]))[:2] == [2, 0]
            if original_tuning is not None:
                assert device.request(110, [1] + pack(original_tuning))[:2] == [1, 0]
            device.request(112, [1, 0])
            device.silence()
        finally:
            device.close()


if __name__ == '__main__':
    main()
