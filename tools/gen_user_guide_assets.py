#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Capture the current production UI and export its parameter reference for the PDF."""
from pathlib import Path
import json,os,subprocess
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
os.chdir(ROOT)
VERSION=json.loads((ROOT/"release.json").read_text())["version"]
SCREEN_DIR=ROOT/"docs/screenshots"/VERSION
SCREEN_DIR.mkdir(parents=True,exist_ok=True)
subprocess.run([os.environ.get('CC','cc'),'-O1','-w','-DSYNTHMAVEN_SYNTH=1','-Ibuild/gen','-Ifirmware/src','-Ifirmware/hal','-o','build/host/user_guide_screens','tests/user_guide_screens.c','-lm'],check=True)
subprocess.run(['build/host/user_guide_screens',str(SCREEN_DIR)],check=True)
raw=json.loads(Path('build/host/guide-inventory.json').read_text());seen=set();out=[]
for page in raw:
 signature=json.dumps([page['page_id'],page['engine'],page['controls']],sort_keys=True)
 if signature not in seen:out.append(page);seen.add(signature)
for image in SCREEN_DIR.glob('*.ppm'):
 with Image.open(image) as im:im.save(image.with_suffix('.png'))
Path('assets/guide/screen-inventory.json').write_text(json.dumps(out,indent=2)+'\n')
assert len({p['page_id'] for p in out})==66
assert {p['engine'] for p in out if p['title']=='EDIT 1'}==set(['ANALOG','DIGITAL','PHASE','LOFI','SAMPLE','VOICE','TRIO','WHEEL','GRAIN','FM6','PHYS','NOISE','HYBRID','WAVE','ACID','STRINGS','MACRO','VPS','TERRAIN','FMX','WSEQ','JUNO','VECTOR'])
print(f'Guide inventory: {len(out)} distinct layouts; all 66 reachable menu pages.')
