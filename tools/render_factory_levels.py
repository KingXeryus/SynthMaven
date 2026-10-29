#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Isolated DSP processes give each preset the same clean effects, RNG and limiter state."""
import os,subprocess,argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];os.chdir(ROOT)
a=argparse.ArgumentParser();a.add_argument('--select');a.add_argument('--prefix',default='build/host/factory');args=a.parse_args();prefix=Path(args.prefix);prefix.parent.mkdir(parents=True,exist_ok=True)
subprocess.run([os.environ.get('CC','cc'),'-O2','-w','-DSYNTHMAVEN_SYNTH=1','-Ibuild/gen','-Ifirmware/src','-Ifirmware/hal','tests/factory_level.c','-o','build/host/factory_level','-lm'],check=True)
env=dict(os.environ,SM_AUDIT_LIST='1');catalog=subprocess.check_output(['build/host/factory_level','/dev/null','/dev/null','/dev/null'],env=env,text=True).splitlines()
wanted=set(args.select.split(',')) if args.select else None
entries=[l.split(None,2) for l in catalog];entries=[(int(e),int(p),name) for e,p,name in entries if wanted is None or f'{e}:{p}' in wanted]
assert entries and (wanted is not None or len(entries)==557)
dir=prefix.parent/(prefix.name+'-segments');dir.mkdir(exist_ok=True)
def run(row):
 e,p,name=row;base=dir/f'{e:02}-{p:03}';env=dict(os.environ,SM_AUDIT_SELECT=f',{e}:{p},');env.pop('SM_AUDIT_LIST',None)
 subprocess.run(['build/host/factory_level','/dev/null',str(base)+'.txt',str(base)+'.raw'],env=env,check=True)
 return base
with ThreadPoolExecutor(max_workers=min(4,os.cpu_count() or 1)) as pool:bases=list(pool.map(run,entries))
with open(str(prefix)+'-levels.txt','w') as catalog,open(str(prefix)+'-stereo.raw','wb') as audio:
 for base in bases:
  text=base.with_suffix('.txt').read_text();assert len(text.splitlines())==1,text;catalog.write(text);audio.write(base.with_suffix('.raw').read_bytes())
print(f'Isolated factory renders: {len(entries)} (clean process per preset).',flush=True)
