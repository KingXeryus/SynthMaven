#!/usr/bin/env python3
"""Capture documentation screen examples without flashing or changing firmware."""
import json,subprocess
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
work=ROOT/'build/guide';(work/'ppm').mkdir(parents=True,exist_ok=True)
(work/'renderer.h').write_text((ROOT/'tests/ui_render.c').read_text().replace('int main(int argc, char **argv)','int renderer_main(int argc, char **argv)'))
subprocess.run(['cc','-O1','-ffp-contract=off','-w','-DMELODEE_USB_AUDIO=1','-DMELODEE_USB_HOST=1','-Ibuild/gen','-Ifirmware/src','-Itests','tools/guide_inventory.c','-lm','-o',str(work/'inventory')],cwd=ROOT,check=True)
raw=json.loads(subprocess.check_output([str(work/'inventory')],cwd=ROOT))
seen={}
for p in raw:
    key=(p['title'],p['effect'] if p['title'] not in ['FX','SHARED'] else '',str(p['controls']),str(p['list']),p['engine'] if p['title'].startswith('EDIT') else '')
    if key not in seen:seen[key]=p
out=ROOT/'assets/guide';(out/'screens').mkdir(parents=True,exist_ok=True)
for p in seen.values():Image.open(work/'ppm'/('NIGHT_'+p['image']+'.ppm')).save(out/'screens'/(p['image']+'.png'))
(out/'screen-inventory.json').write_text(json.dumps(list(seen.values()),indent=2)+'\n')
print(len(seen),'distinct screen/control layouts captured')
