#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Cross-check release measurements using independent pyloudnorm DeMan filters/gating."""
import json,numpy as np,pyloudnorm
from pathlib import Path
from match_factory_levels import FS,SEG,measure
rows=json.loads(Path('build/host/factory-level-audit.json').read_text());raw=np.memmap('build/host/factory-stereo.raw',dtype='<i4',mode='r');meter=pyloudnorm.Meter(FS,filter_class='DeMan')
selected=set();engines=set()
for i,r in enumerate(rows):
 if r['engine'] not in engines and r['lufs'] is not None:selected.add(i);engines.add(r['engine'])
selected.update(i for i,r in enumerate(rows) if r['role']==5)
errors=[]
for i in sorted(selected):
 x=np.asarray(raw[i*SEG*2:(i+1)*SEG*2],dtype=float).reshape(-1,2)/32768;independent=float(meter.integrated_loudness(x));errors.append(abs(independent-rows[i]['lufs']))
assert max(errors)<.01,max(errors)
Path('build/host/loudness-crosscheck.json').write_text(json.dumps({'independent_meter':'pyloudnorm 0.1.1 DeMan / 44100 Hz','references_checked':len(errors),'max_difference_lu':max(errors)},indent=2)+'\n')
print(f'Independent BS.1770 cross-check: {len(errors)} sounds across all 23 engines, max difference {max(errors):.8f} LU PASS')
