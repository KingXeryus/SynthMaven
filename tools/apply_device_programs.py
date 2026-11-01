#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Put the programs saved on the user's FM-1 into the factory bank, by slot.

assets/factory/device-programs-2026-11-01.bin holds P1..P3 exactly as read over USB
(P4..P8 were never saved on that FM-1, so it plays the factory ones). Run after
tools/make_factory_programs.c, then python3 tools/gen_factory_program_pack.py.
"""
from pathlib import Path
RECORD = 3556
root = Path(__file__).resolve().parents[1]
bank = bytearray((root / 'assets/factory/programs-3.5.bin').read_bytes())
device = (root / 'assets/factory/device-programs-2026-11-01.bin').read_bytes()
assert len(bank) == 8 * RECORD and len(device) % RECORD == 0
bank[:len(device)] = device
(root / 'assets/factory/programs-3.5.bin').write_bytes(bank)
print(len(device) // RECORD, 'device programs in the factory bank')
