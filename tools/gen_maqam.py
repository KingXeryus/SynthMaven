#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Generate compact 12-key maqam tables and matching editor data. --check is read-only."""
import argparse,json,re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');args=p.parse_args()
rows=json.loads((root/'assets/maqam/scales.json').read_text());profiles=[]
assert len(rows)==46 and [r['id'] for r in rows[:2]]==['RAST','BAYATI']
assert len({r['id'] for r in rows})==len(rows)
for row in rows:
 off=row['offsets'];assert len(off)==12 and off[0]==0 and all(c in (0,-50,50,-25) for c in off)
 assert all(any(k*100+v==c for k,v in enumerate(off)) for c in row['degrees'])
 masks=[sum(1<<k for k,v in enumerate(off) if v==c) for c in [-50,50,-25]]
 if masks not in profiles:profiles.append(masks)
 assert row['profile']==profiles.index(masks)
text='/* SPDX-License-Identifier: GPL-3.0-only\n * 12-key reference tunings; factual source/provenance: assets/maqam/scales.json.\n * First two entries preserve the existing RAST/BAYATI IDs. */\n'
for macro,rs in [('MAQAM_EXISTING',rows[:2]),('MAQAM_ADDED',rows[2:])]:
 text+='#define '+macro+'(X) \\\n'+' \\\n'.join(f' X({r["id"]}, "{r["name"].upper()}", {r["profile"]})' for r in rs)+'\n'
text+='static const uint16_t MAQAM_MASKS[][3] = { /* down 50, up 50, down 25 cents */\n'
text+=''.join('    {'+','.join(hex(v) for v in row)+'},\n' for row in profiles)+'};\n'
text+='static const uint16_t MAQAM_NOTE_MASKS[46] = { /* chromatic keys for documented maqam degrees */\n'
for row in rows:
 mask=sum(1<<next(k for k,v in enumerate(row['offsets']) if k*100+v==c) for c in row['degrees'])
 text+='    '+hex(mask)+',\n'
text+='};\n'
names=['EQUAL','JUST','PYTHAG','MEANTONE','WERCK III','KIRNB III','RAST','BAYATI','SLENDRO','PELOG','19-EDO','24-EDO','31-EDO','USER']+[r['name'].upper() for r in rows[2:]]+['CUSTOM1','CUSTOM2','CUSTOM3','CUSTOM4']
cat=[dict(sys=(6+i if i<2 else 12+i),name=r['name'],offsets=r['offsets'],note=r['note']) for i,r in enumerate(rows)]
constants='const TU_NAMES = '+json.dumps(names)+';\nconst TU_USER = 13;\nconst TU_MAQAM = '+json.dumps(cat,separators=(',',':'))+';\n'
h=root/'firmware/src/maqam_scales.h';web=root/'web/editor.html';html=web.read_text();a=html.index('const TU_NAMES = [');b=html.index('let tuv = null;',a)
if args.check:
 assert h.read_text()==text,'regenerate firmware table'
 assert html[a:b]==constants,'regenerate editor table'
else:
 h.write_text(text);web.write_text(html[:a]+constants+html[b:])
print('Maqam data: 46 settings, 15 compact tuning profiles, stable IDs and matching editor PASS')
