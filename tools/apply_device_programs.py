#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Put the programs saved on the user's FM-1 into the factory bank, by slot.

assets/factory/device-programs-2026-11-01.bin holds P1-P3 and P5-P8 exactly as read over USB
(P4 was never saved on that FM-1, so it plays the factory one). Run after
tools/make_factory_programs.c, then python3 tools/gen_factory_program_pack.py.
"""
from pathlib import Path
RECORD = 3556
SLOTS = (0, 1, 2, 4, 5, 6, 7)
root = Path(__file__).resolve().parents[1]
bank = bytearray((root / 'assets/factory/programs-3.5.bin').read_bytes())
device = (root / 'assets/factory/device-programs-2026-11-01.bin').read_bytes()
assert len(bank) == 8 * RECORD and len(device) == len(SLOTS) * RECORD
for k, slot in enumerate(SLOTS):
    bank[slot * RECORD:(slot + 1) * RECORD] = device[k * RECORD:(k + 1) * RECORD]
(root / 'assets/factory/programs-3.5.bin').write_bytes(bank)
print(len(SLOTS), 'device programs in the factory bank')
