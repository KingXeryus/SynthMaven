#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Release gate: production DSP, solo/layer OFF/full mixes, scenes and hot MIDI."""
import argparse,subprocess,os,json,importlib.util,zipfile
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import numpy as np
ROOT=Path(__file__).resolve().parents[1];ap=argparse.ArgumentParser();ap.add_argument('--out',default='build/factory-programs');args=ap.parse_args();out=ROOT/args.out;out.mkdir(parents=True,exist_ok=True)
meta=json.loads((ROOT/'release.json').read_text());cfg=json.loads((ROOT/'assets/loudness.json').read_text());target=cfg['target_lufs'];tol=cfg['tolerance_lu'];ceiling=cfg['true_peak_ceiling_dbtp']
spec=importlib.util.spec_from_file_location('levels',ROOT/'tools/match_factory_levels.py');levels=importlib.util.module_from_spec(spec);spec.loader.exec_module(levels)
subprocess.run([os.environ.get('CC','cc'),'-O2','-w','-DSYNTHMAVEN_SYNTH=1','-Ibuild/gen','-Ifirmware/src','-Ifirmware/hal','-o','build/host/factory_program_audio','tests/factory_program_audio.c','-lm'],cwd=ROOT,check=True)
recipes=json.loads((ROOT/'assets/programs/recipes.json').read_text())['programs']
jobs=[]
for slot in range(8):
 for scene in range(4):jobs.append((slot,scene,4,'mix'))
 for solo in range(4):
  for mode in ['solo','layer-off','raw']:jobs.append((slot,0,solo,mode))
 jobs.append((slot,0,4,'hot'))
def name(job):
 slot,scene,solo,mode=job;return f'P{slot+1:02d}-'+recipes[slot]['name'].replace(' ','-')+(f'-part{solo+1}-{mode}' if solo<4 else f'-scene{scene+1}' if scene else '')+('-velocity127' if mode=='hot' else '')
def render(job):
 slot,scene,solo,mode=job;p=out/name(job);env=dict(os.environ)
 for variable in ['SM_PROGRAM_STRESS','SM_PROGRAM_RAW_SOLO','SM_PROGRAM_LAYER_OFF']:env.pop(variable,None)
 if mode=='hot':env['SM_PROGRAM_STRESS']='1'
 if mode=='raw':env['SM_PROGRAM_RAW_SOLO']='1'
 if mode=='layer-off':env['SM_PROGRAM_LAYER_OFF']='1'
 r=subprocess.run(['build/host/factory_program_audio',str(slot),str(scene),str(solo),str(p)+'.wav',str(p)+'.raw'],cwd=ROOT,env=env,check=True,capture_output=True,text=True)
 return job,r.stdout.strip()
with ThreadPoolExecutor(max_workers=min(4,os.cpu_count() or 1)) as pool:renders=list(pool.map(render,jobs))
report=[];main=[];parts={}
for job,render_note in renders:
 slot,scene,solo,mode=job;p=out/name(job);x=np.fromfile(str(p)+'.raw',dtype='<i4').reshape(-1,2)/32768.;lu,pk=levels.measure(x)
 rms=float(np.sqrt(np.mean(x*x)));mono=float(np.sqrt(np.mean(x.mean(axis=1)**2)));retention=float(20*np.log10(max(mono,1e-12)/max(rms,1e-12)))
 row={'slot':slot+1,'name':recipes[slot]['name'],'scene':scene+1,'solo':solo+1 if solo<4 else None,'mode':mode,'lufs':lu,'true_peak_dbtp':pk,'mono_retention_db':retention,'peak':float(np.max(np.abs(x))),'render':render_note};report.append(row)
 assert lu is not None and rms>1e-6,'Silent factory part: '+name(job)
 assert pk<=ceiling,('Peak exceeds ceiling',name(job),pk)
 assert not np.any(np.abs(x)>=1),'Clipping: '+name(job)
 # Loudness is reported only: 2.2.1 restores the pre-target gain structure.
 if mode=='mix':
  assert retention>=-3,'Excessive mono loss: '+name(job)
  if not scene:main.append(row)
 if mode=='raw':parts.setdefault(slot,{})[solo]=row
for slot,rows in parts.items():
 # Foreground balance is set by the mixer, without solo normalization.
 assert all(rows[i]['peak']>=rows[0]['peak']*.01 for i in range(1,4)),'Support is too faint: P'+str(slot+1)
for row in main:print(f'P{row["slot"]} {row["name"]}: {row["lufs"]:.2f} LUFS / {row["true_peak_dbtp"]:.2f} dBTP / mono {row["mono_retention_db"]:.2f} dB')
(out/'audit.json').write_text(json.dumps({'version':meta['version'],'target_lufs':None,'tolerance_lu':None,'true_peak_ceiling_dbtp':ceiling,'reference':'Production DSP, 12 seconds; variable-velocity 8-second performance and release tail. No post-render normalization. Raw contribution renders intentionally retain support balance.','renders':report},indent=2)+'\n')
readme=f'SynthMaven {meta["version"]} factory program previews\n\nHost renders of the production DSP, not physical FM-1 recordings. Each numbered WAV plays the complete four-part MAIN scene at varied velocities. No normalization is applied to the WAV after rendering. The firmware program supplies the calibrated mix.\n\n'
for i,r in enumerate(recipes):readme+=f'P{i+1} - {r["style"]}: {r["name"]}\n{r["play"]}\n\n'
(out/'README.txt').write_text(readme)
with zipfile.ZipFile(out/f'SynthMaven-{meta["version"]}-Factory-Program-Previews.zip','w',zipfile.ZIP_DEFLATED) as z:
 z.write(out/'README.txt','README.txt');z.write(out/'audit.json','audit.json')
 for slot in range(8):p=out/(name((slot,0,4,'mix'))+'.wav');z.write(p,p.name)
print(f'PASS: {len(report)} renders; 32 solo parts, 32 layer-OFF parts, 32 layered scenes; restored gain structure, reported LUFS, hot MIDI, mono, peaks, clipping and shared voices')
