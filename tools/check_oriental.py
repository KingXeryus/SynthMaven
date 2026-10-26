#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import json,re
from oriental_selection import select_presets
from pathlib import Path
root=Path(__file__).resolve().parents[1]
reference=json.loads((root/'assets/oriental/software-0.2.json').read_text())
reference=select_presets(reference)
actual=json.loads((root/'build/host/oriental-loaded.json').read_text())
assert len(reference)==len(actual)==88
levels=[list(map(int,row.split(','))) for row in re.findall(r' \{([^{}]+)\}',(root/'firmware/src/factory_levels.h').read_text())]
web=json.loads((root/'assets/oriental/editor.json').read_text())
for r,a,w in zip(reference,actual,web):
 assert r['engine']==a['engine']
 expected=r['values'][:126].copy();expected[8]=levels[w['engine']][w['preset']]
 assert expected==w['values']
 assert expected==a['values'],(r['name'],[(p,x,y) for p,(x,y) in enumerate(zip(r['values'],a['values'])) if x!=y])
print('Oriental bank: all 88 loaded patches match all 125 desktop synthesis settings and the calibrated FM1 trim exactly PASS')
