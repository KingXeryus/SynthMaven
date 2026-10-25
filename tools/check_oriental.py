#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import json
from oriental_selection import select_presets
from pathlib import Path
root=Path(__file__).resolve().parents[1]
reference=json.loads((root/'assets/oriental/software-0.2.json').read_text())
reference=select_presets(reference)
actual=json.loads((root/'build/host/oriental-loaded.json').read_text())
assert len(reference)==len(actual)==88
for r,a in zip(reference,actual):
 assert r['engine']==a['engine']
 assert r['values'][:126]==a['values'],(r['name'],[(p,x,y) for p,(x,y) in enumerate(zip(r['values'],a['values'])) if x!=y])
print('Oriental bank: all 88 loaded patches match all 126 desktop synthesis settings exactly PASS')
