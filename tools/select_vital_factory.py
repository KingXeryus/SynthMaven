#!/usr/bin/env python3
"""Select native HYBRID approximations from user-supplied Vital files; never embeds source audio."""
import argparse,json,base64,hashlib,subprocess,math
from pathlib import Path
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('--node',default='node');a=p.parse_args()
files=[]
for f in sorted(a.source.rglob('*')):
 if not f.is_file() or f.name.startswith('.'):continue
 try:j=json.loads(f.read_text());assert isinstance(j.get('settings'),dict)
 except Exception:continue
 files.append(f)
raw=subprocess.check_output([a.node,str(ROOT/'tools/convert_vital.mjs'),'--approximate',*map(str,files)],cwd=ROOT)
staged=json.loads(raw)['items'];x=np.arange(2048)/2048
# Native waveform templates follow eng_hybrid.c. Magnitude comparison avoids phase-alignment bias.
sine=np.sin(2*np.pi*x);tri=1-4*np.abs(x-.5);saw=2*x-1;square=np.where(x<.5,1.,-1.)
templates=[(4,0,sine),(3,0,tri),(1,0,saw)]
for shape in range(0,128,8):
 duty=.5+shape/127*.4375;templates.append((2,shape,np.where(x<duty,1.,-1.)))
for shape in range(0,128,8):
 seg=min(2,shape//43);t=min(1,(shape-seg*43)/43);waves=[sine,tri,saw,square];templates.append((5,shape,(1-t)*waves[seg]+t*waves[seg+1]))
def spectrum(w):
 h=np.abs(np.fft.rfft(w))[1:33];return h/(np.linalg.norm(h)+1e-12)
tspec=np.array([spectrum(t[2]) for t in templates])
def sample(table,pos):
 groups=table.get('groups',[])
 if not groups:raise ValueError('no wavetable groups')
 c=groups[0]['components'][0];frames=sorted(c.get('keyframes',[]),key=lambda x:x.get('position',0))
 if not frames:raise ValueError('no source keyframes')
 lo=max([f for f in frames if f.get('position',0)<=pos] or frames[:1],key=lambda f:f.get('position',0));hi=min([f for f in frames if f.get('position',0)>=pos] or frames[-1:],key=lambda f:f.get('position',0));t=0 if hi.get('position',0)==lo.get('position',0) else (pos-lo.get('position',0))/(hi['position']-lo['position'])
 typ=c.get('type');extra=[]
 if len(groups)>1 or len(groups[0]['components'])>1:extra.append('Wavetable graph modifiers/groups omitted in native projection.')
 def frame(f):
  if typ=='Wave Source':
   b=base64.b64decode(f['wave_data']);v=np.frombuffer(b,dtype='<f4').astype(float)
   if len(v)!=2048:raise ValueError('non-2048 float frame')
   return v
  if typ=='Line Source':
   line=f['line'];pts=np.array(line['points']).reshape(-1,2);return np.interp(x,pts[:,0],1-2*pts[:,1])
  raise ValueError('unsupported source')
 if typ=='Audio File Source':
  # Vital serializes Audio File Source as PCM16. Approximate its selected window, not its resynthesis DSP.
  pcm=np.frombuffer(base64.b64decode(c['audio_file']),dtype='<i2').astype(float)/32768
  start=(1-t)*lo.get('start_position',0)+t*hi.get('start_position',0);size=(1-t)*lo.get('window_size',2048)+t*hi.get('window_size',2048)
  if not 16<=size<=len(pcm) or not 0<=start<len(pcm):raise ValueError('invalid audio window')
  v=np.interp(start+x*size,np.arange(len(pcm)),pcm,right=0);extra.append('Audio File Source window projected onto a native periodic waveform; resynthesis/fades are not reproduced.')
 else:
  v=(1-t)*frame(lo)+t*frame(hi)
  if typ=='Line Source':extra.append('Line Source curves approximated by linear points before native projection.')
 if not np.isfinite(v).all() or np.max(np.abs(v))<1e-7:raise ValueError('silent/invalid source frame')
 return v,extra
out=[]
for f,r in zip(files,staged):
 j=json.loads(f.read_text());s=j['settings'];score=100.;evidence=[];patch=r['patch'];active=0
 for i,o in enumerate(patch['osc']):
  base=float(s.get(f'osc_{i+1}_level',.707));boost=0
  for mi,m in enumerate(s.get('modulations',[])):
   if m.get('destination')==f'osc_{i+1}_level':
    boost=max(boost,float(s.get(f'modulation_{mi+1}_amount',0)))
  if not s.get(f'osc_{i+1}_on',i==0) or max(base,boost)<=0:continue
  if base<=0 and boost>0:
   o['level']=round(min(1,boost)*127);score-=2;r['warnings'].append(f'Oscillator {i+1}: modulation-driven zero-base level approximated at its modulation peak with the shared amp envelope; per-oscillator level motion differs.')
   for route in patch['mod']:
    if route['dst']==8+i and route['src'] in (1,2,3):route.update(src=0,dst=0,amt=0)
  active+=1;table=(s.get('wavetables') or [{}]*3)[i]
  try:
   v,extra=sample(table,s.get(f'osc_{i+1}_wave_frame',0));h=spectrum(v);dist=1-tspec@h;k=int(np.argmin(dist));typ,shape,_=templates[k];error=float(dist[k]);o['type']=typ;o['shape']=shape
   if 'noise' in str(table.get('name','')).lower():o['type']=10;o['shape']=80 if 'brown' in str(table.get('name','')).lower() else 0;error=max(error,.15)
   if o['type']==1 and s.get(f'osc_{i+1}_unison_voices',1)>1:o['type']=6;o['shape']=min(100,max(8,round(s.get(f'osc_{i+1}_unison_detune',.2)*50)));extra.append('Saw unison approximated by native seven-saw SUPER oscillator.')
   evidence.append({'oscillator':i+1,'source':str(table.get('name','')),'native_type':o['type'],'native_shape':o['shape'],'spectral_distance':round(error,5),'approximations':extra});score-=error*25+len(extra)*2
  except Exception as e:evidence.append({'oscillator':i+1,'fallback':'native saw','reason':str(e)});score-=17
  score-=5*bool(s.get(f'osc_{i+1}_spectral_morph_type',0))+5*bool(s.get(f'osc_{i+1}_distortion_type',0))
 score-=min(20,2*sum('unsupported' in w or 'omitted' in w for w in r['warnings']))
 if s.get('sample_on'):score-=15
 if not active:score=-100
 r.update(source=str(f.relative_to(a.source)),category=f.relative_to(a.source).parts[0],compatibility_score=round(score,2),waveform_projection=evidence,source_sha256=hashlib.sha256(f.read_bytes()).hexdigest(),patch=patch,review='automated compatibility/spectral ranking; listening approval pending')
 # Remove obsolete generic saw warnings replaced by explicit projection evidence.
 r['warnings']=[w for w in r['warnings'] if 'replaced with a native saw' not in w]
 out.append(r)
print('Unplayable:',[(r['source'],r['compatibility_score'],[(o['type'],o['level']) for o in r['patch']['osc']]) for r in out if not any(o['type'] and o['level'] for o in r['patch']['osc'])])
ranked=sorted(out,key=lambda r:(-r['compatibility_score'],r['source']))
selected=ranked[:64];excluded=ranked[64:]
assert len(selected)==64 and all(any(o['type'] and o['level'] for o in r['patch']['osc']) for r in selected)
# Four complementary startup parts, followed by grouped families. These IDs are append-only afterwards.
prefs=[('Keys','Strobe'),('Pad','Dark'),('Bass','Reese'),('Pluck','Pluck')];first=[]
for cat,word in prefs:
 choices=[r for r in selected if r['category']==cat and word.lower()==Path(r['source']).stem.lower()];first.append(choices[0] if choices else next(r for r in selected if r['category']==cat and r not in first))
order=first+sorted([r for r in selected if r not in first],key=lambda r:({'Keys':0,'Pluck':1,'Pad':2,'Synth':3,'Bass':4,'Guitar':5,'Arp':6,'Lo-Fi':7,'FX':8}.get(r['category'],9),-r['compatibility_score'],r['source']))
for i,r in enumerate(order):
 r['name']=f'V{i+1:02d} '+Path(r['source']).stem.upper()[:8];r['name']=r['name'][:12];r['patch']['name']=r['name'][:10];r['index']=36+i;r['preset']=41+i;r['originalName']=Path(r['source']).stem;r.pop('packed',None)
(ROOT/'assets/vital/selected.json').write_text(json.dumps({'format':'synthmaven-vital-factory','version':1,'selection_method':'Native spectral fit, supported-feature penalties and family diversity; not a listening-equivalence certification.','items':order,'excluded':[{'source':r['source'],'author':r['author'],'score':r['compatibility_score'],'reason':'Sample-only (no playable native oscillator)' if r['compatibility_score']<0 else 'Among the four lowest compatibility scores; substantial unsupported synthesis.'} for r in excluded]},indent=2)+'\n')
print('Selected',len(order),'of',len(out),'excluded',[(r['source'],r['compatibility_score']) for r in excluded]);print('startup',[r['name'] for r in first])
