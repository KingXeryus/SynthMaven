#!/usr/bin/env python3
"""Check that native and browser capabilities match the reviewed engine map."""
import json,re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
rows=json.loads((root/'assets/filter-capabilities.json').read_text())
source=(root/'firmware/src/filter_caps.h').read_text()
for i,c in enumerate(rows):
 assert c['engine']==i
 model='P_FMODEL' if c['architecture']=='multi' else '-2' if c['name']=='HYBRID' else 'P_E4' if c['name']=='TRIO' else '-1'
 expected='{'+','.join([model,c['cutoff'],c['resonance'],c['envelope'],c['drive'],'"'+c['architecture']+'"'])+'}'
 assert expected in source,(c['name'],'native capability map drift')
web=(root/'web/editor.html').read_text();embedded=web.split('const SM_FILTER_MAP = ',1)[1]
assert json.JSONDecoder().raw_decode(embedded)[0]==rows,'browser capability map drift'
assert len(rows)==23
print('23 engine capability maps match native and browser controls')
