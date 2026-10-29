/* SPDX-License-Identifier: GPL-3.0-only
 * Shared SynthMaven instrument controls, diagrams and appearance preferences. */
(() => {
 const root=document.documentElement, key='synthmaven-appearance', editorOnlyDark=root.dataset.smApp==='editor';
 let preference='dark';try{preference=localStorage.getItem(key)||'dark';}catch{}
 const media=matchMedia('(prefers-color-scheme: dark)');
 function theme(){root.dataset.theme=editorOnlyDark?'dark':preference==='system'?(media.matches?'dark':'light'):preference;window.dispatchEvent(new Event('resize'));}
 theme();media.addEventListener('change',theme);
 const NS='http://www.w3.org/2000/svg';
 const svg=(tag,attrs={})=>{const e=document.createElementNS(NS,tag);for(const[k,v]of Object.entries(attrs))e.setAttribute(k,v);return e;};
 const point=(a,r)=>[35+Math.sin(a)*r,35-Math.cos(a)*r];
 function arc(v){const start=-Math.PI*.75,end=start+Math.PI*1.5*Math.max(.0001,v),a=point(start,29),b=point(end,29);return `M${a} A29,29 0 ${v>2/3?1:0},1 ${b}`;}
 const dials=new Map();let section='sound';
 function dial(input){
  if(dials.has(input)||input.closest('#mixer,.mixer,.strip')||input.id==='stmorph'||input.id==='stamt')return;
  const wrap=document.createElement('span');wrap.className='sm-dial-wrap';
  const graphic=svg('svg',{viewBox:'0 0 70 70','aria-hidden':'true'});
  graphic.append(svg('path',{d:arc(1),class:'sm-dial-track'}));const progress=svg('path',{class:'sm-dial-progress'});
  const needle=svg('line',{x1:35,y1:17,x2:35,y2:24,class:'sm-dial-needle'});
  graphic.append(svg('circle',{cx:35,cy:35,r:22,class:'sm-dial-face'}),progress,needle);
  input.before(wrap);wrap.append(graphic,input);dials.set(input,{progress,needle,last:null});
  if(!input.getAttribute('aria-label'))input.setAttribute('aria-label',input.title||input.closest('.knob')?.querySelector('.kl')?.textContent||input.closest('label')?.textContent?.trim()||'Parameter');
  let drag=null;
  input.addEventListener('pointerdown',e=>{if(input.disabled||e.button!==0)return;drag={x:e.clientX,y:e.clientY,v:+input.value};input.setPointerCapture(e.pointerId);e.preventDefault();});
  input.addEventListener('pointermove',e=>{if(!drag)return;const range=+input.max-(+input.min),step=+input.step||1;const v=drag.v+((drag.y-e.clientY)+(e.clientX-drag.x)*.25)*range/(e.shiftKey?800:200);input.value=String(Math.max(+input.min,Math.min(+input.max,Math.round(v/step)*step)));input.dispatchEvent(new Event('input',{bubbles:true}));paintDial(input);});
  const end=()=>{if(!drag)return;drag=null;input.dispatchEvent(new Event('change',{bubbles:true}));};
  input.addEventListener('pointerup',end);input.addEventListener('pointercancel',end);input.addEventListener('input',()=>paintDial(input));paintDial(input);
 }
 function paintDial(input){const d=dials.get(input);if(!d)return;const v=(+input.value-(+input.min))/(+input.max-(+input.min)||1);if(d.last===v)return;d.last=v;d.progress.setAttribute('d',arc(v));d.needle.setAttribute('transform',`rotate(${-135+270*v} 35 35)`);}
 const labels={ATK:'Attack',DEC:'Decay',SUS:'Sustain',REL:'Release',LVL:'Level',LEVEL:'Level',PAN:'Pan',TRIM:'Level trim',RATE:'Rate',WAVE:'Waveform',FADE:'Fade-in',PIT:'Pitch',FILT:'Filter',VOL:'Volume',CUT:'Cutoff',RES:'Resonance',DST:'Distortion',CHO:'Chorus',DLY:'Delay',REV:'Reverb',GLD:'Portamento time',VCE:'Voice mode',DETUNE:'Detune'};
 function category(title){title=title.replace(/[\uE000-\uF8FF]/g,'').trim();return /^(LFO|MOD|MATRIX)/.test(title)?'modulation':/^(FX|SPACE|MASTER)/.test(title)?'effects':/^(SEL|TUN)/.test(title)?'tuning':/^LAYER/.test(title)?'layers':'sound';}
 function filterSections(){
  root.dataset.smSection=section;
  for(const group of document.querySelectorAll('#groups>.group')){const title=group.dataset.smTitle||group.querySelector('h2')?.textContent||'';group.dataset.smSectionHidden=String(section!=='all'&&category(title)!==section);}
  const special={hypanel:'sound',fm6panel:'sound',fxpanel:'sound',mspanel:'modulation',lypanel:'layers',tupanel:'tuning','sm-patches':'sound','sm-layer-mixer':'layers'};
  for(const[id,cat]of Object.entries(special)){const e=document.getElementById(id);if(e)e.dataset.smSectionHidden=String(section!=='all'&&cat!==section);}
  for(const b of document.querySelectorAll('[data-sm-section]'))b.setAttribute('aria-pressed',String(b.dataset.smSection===section));
  const tools=document.getElementById('sndtools');if(tools)tools.dataset.smSectionHidden=String(section==='layers'||section==='tuning');
  const layer=document.getElementById('sm-program-mode-card');if(layer)layer.dataset.smSectionHidden=String(section!=='all'&&section!=='layers');
 }
 function text(g,x,y,value){const e=svg('text',{x,y});e.textContent=value;g.append(e);return e;}
 const graphics=new Map();
 function graphic(group){
  if(graphics.has(group))return;const title=(group.dataset.smTitle||group.querySelector('h2')?.textContent||'').replace(/[\uE000-\uF8FF]/g,'').trim();
  const kind=/^ENV(?: [23])?$/.test(title)?'envelope':/^LFO(?: [23])?$/.test(title)?'lfo':/^(FILTER|FLT)$/.test(title)?'filter':/^DLY$/.test(title)?'delay':/^REVERB$/.test(title)?'reverb':/^CHORUS$/.test(title)?'chorus':/^INSERT$/.test(title)?'distortion':/^FX/.test(title)?'effects':/^MASTER/.test(title)?'eq':/^(EDIT|OSC)/.test(title)?'oscillator':null;
  if(!kind)return;
  const g=svg('svg',{viewBox:'0 0 640 168',class:'sm-graphic',role:'img','aria-label':kind+' parameter preview'});
  text(g,12,18,{envelope:'ENVELOPE / NORMALIZED SHAPE',lfo:'LFO / WAVEFORM PREVIEW',effects:'EFFECT SENDS',eq:'EQ / ILLUSTRATIVE RESPONSE',oscillator:'OSCILLATOR / WAVEFORM PREVIEW',filter:'FILTER / ILLUSTRATIVE RESPONSE',delay:'DELAY / ECHO PREVIEW',reverb:'REVERB / DECAY PREVIEW',chorus:'MODULATED DELAY / PREVIEW',distortion:'DISTORTION / TRANSFER PREVIEW'}[kind]);
  for(let i=0;i<=8;i++)g.append(svg('line',{x1:16+76*i,y1:32,x2:16+76*i,y2:136,class:'sm-grid'}));
  for(let i=0;i<=4;i++)g.append(svg('line',{x1:16,y1:32+26*i,x2:624,y2:32+26*i,class:'sm-grid'}));
  const path=svg('path',{class:'sm-curve'});g.append(path);if(group.matches('.knobs'))group.before(g);else group.querySelector('h2')?.after(g);const state={g,path,kind,group,last:''};graphics.set(group,state);
  if(kind==='envelope'){
   text(g,16,158,'Attack / Decay / Sustain / Release · Drag handles');
   for(let i=0;i<3;i++){const c=svg('circle',{r:4,'data-handle':i});g.append(c);}
   let drag=null;g.addEventListener('pointerdown',e=>{const h=e.target.getAttribute('data-handle');if(h===null)return;drag=+h;g.setPointerCapture(e.pointerId);e.preventDefault();});
   g.addEventListener('pointermove',e=>{if(drag===null)return;const inputs=envelopeInputs(group);const r=g.getBoundingClientRect(),x=((e.clientX-r.left)/r.width*640-16)/608,y=(e.clientY-r.top)/r.height;const set=(id,v)=>{const inp=inputs[id];if(inp){inp.value=String(Math.round(+inp.min+Math.max(0,Math.min(1,v))*(+inp.max-(+inp.min))));inp.dispatchEvent(new Event('input',{bubbles:true}));}};if(drag===0)set(0,(x-.04)/.25);else if(drag===1){const a=inputs[0]?(+inputs[0].value-(+inputs[0].min))/(+inputs[0].max-(+inputs[0].min)||1):0;set(1,(x-.09-.25*a)/.2);set(2,1-(y*168-32)/104);}else set(3,(x-.75)/.24);drawGraphic(state);});
   g.addEventListener('pointerup',()=>{if(drag===null)return;for(const inp of envelopeInputs(group))inp?.dispatchEvent(new Event('change',{bubbles:true}));drag=null;});g.addEventListener('pointercancel',()=>{drag=null;});
  }
  drawGraphic(state);
 }
 function envelopeInputs(group){return ['ATK','DEC','SUS','REL'].map(n=>Array.from(group.querySelectorAll('input[type=range]')).find(i=>i.title===n));}
 function drawGraphic(s){
  if(!s.group.isConnected){graphics.delete(s.group);return;}
  const ranges=Array.from(s.group.querySelectorAll('input[type=range]'));const values=ranges.map(i=>(+i.value-(+i.min))/(+i.max-(+i.min)||1));const selections=Array.from(s.group.querySelectorAll('select')).map(i=>i.selectedOptions[0]?.textContent||'');const signature=values.join()+'|'+selections.join();if(signature===s.last)return;s.last=signature;
  const xy=(x,y)=>[16+x*608,136-y*104];let d='';
  if(s.kind==='envelope'){
   const v=envelopeInputs(s.group).map(i=>i?(+i.value-(+i.min))/(+i.max-(+i.min)||1):0);const a=xy(.04+.25*v[0],1),b=xy(.09+.25*v[0]+.2*v[1],v[2]),c=xy(.75+.24*v[3],0);d=`M${xy(0,0)} L${a} Q${a[0]},${b[1]} ${b} L${xy(.72,v[2])} L${c}`;Array.from(s.g.querySelectorAll('circle')).forEach((e,i)=>{e.setAttribute('cx',[a,b,c][i][0]);e.setAttribute('cy',[a,b,c][i][1]);});
  } else if(s.kind==='effects'){
   d=values.slice(0,4).map((v,i)=>`M${52+i*150},136 L${52+i*150},${136-v*104} L${120+i*150},${136-v*104} L${120+i*150},136`).join(' ');textOnce(s,'Distortion / Chorus / Delay / Reverb · Current parameter values');
  } else {
   const wave=selections.find(x=>/SIN|TRI|SAW|SQR|PULSE|SQUARE/.test(x))||'SIN';const rate=values[0]||0;
   for(let i=0;i<=300;i++){const x=i/300,t=(x*(1+rate*3))%1;let y=s.kind==='filter'?.08+.82/(1+Math.exp((x-(values[0]||.5))*22))+(values[1]||0)*.2*Math.exp(-(((x-(values[0]||.5))*20)**2)):s.kind==='delay'?.12+.8*Math.exp(-x*5/(.2+(values[0]||.5)))*Math.max(0,Math.cos(x*50))**12:s.kind==='reverb'?.1+.85*Math.exp(-x*(5-4*(values[0]||0)))*(.78+.22*Math.cos(x*80)):s.kind==='distortion'?.5+.42*Math.tanh((x*2-1)*(1+10*(values[0]||0))):s.kind==='eq'?.5+(values[0]-.5||0)*Math.exp(-x*5)+(values[1]-.5||0)*Math.exp(-(((x-.5)*5)**2))+(values[2]-.5||0)*Math.exp(-(1-x)*5):wave.includes('TRI')?.5+.4*(1-4*Math.abs(t-.5)):wave.includes('SAW')?.1+.8*t:/SQR|SQU|PULSE/.test(wave)?(t<.5?.9:.1):.5+.4*Math.sin(t*Math.PI*2);d+=(i?' L':'M')+xy(x,y);}
   textOnce(s,s.kind==='eq'||s.kind==='filter'?'Illustrative frequency response':'Parameter preview · No audio is generated by this graphic');
  }
  s.path.setAttribute('d',d);
 }
 function textOnce(s,v){if(!s.caption)s.caption=text(s.g,16,158,v);}
 let scanQueued=false;
 function scan(){scanQueued=false;
  for(const input of document.querySelectorAll('.knob input[type=range],#hybody input[type=range],#fm6ops input[type=range],#fm6voice input[type=range],#fxops input[type=range]'))dial(input);
  for(const group of document.querySelectorAll('#groups>.group,#setgroups>.group')){if(!group.dataset.smTitle)group.dataset.smTitle=group.querySelector('h2')?.textContent||'';if(!group.querySelector('h3'))graphic(group);for(const row of group.querySelectorAll(':scope>.knobs')){let h=row.previousElementSibling;if(h?.tagName==='H3'){row.dataset.smTitle=h.textContent;graphic(row);}}}
  for(const e of document.querySelectorAll('.knob .kl'))if(labels[e.textContent])e.textContent=e.textContent==='DST'&&e.parentElement.querySelector('select')?'Destination':labels[e.textContent];filterSections();
 }
 function ready(){
  if(!editorOnlyDark){const appearance=document.createElement('label');appearance.className='sm-theme';appearance.textContent='Appearance';const select=document.createElement('select');select.setAttribute('aria-label','Appearance');for(const [value,label]of [['dark','Dark'],['light','Light'],['system','System']]){const o=new Option(label,value);select.add(o);}select.value=preference;select.addEventListener('change',()=>{preference=select.value;try{localStorage.setItem(key,preference);}catch{}theme();});appearance.append(select);(document.querySelector('.side')||document.querySelector('.top')||document.body).append(appearance);}
  if(root.dataset.smApp==='editor'){
   const sound=document.getElementById('p-sound'),stage=document.createElement('div');stage.className='sm-instrument-stage';if(sound){sound.classList.add('sm-instrument-layout');const first=sound.firstElementChild;first?.classList.add('sm-factory-library');for(const child of Array.from(sound.children).slice(1))stage.append(child);sound.append(stage);}
   const nav=document.createElement('nav');nav.className='sm-sections';nav.setAttribute('aria-label','Instrument pages');
   for(const [id,label]of [['sound','Sound'],['modulation','Modulation'],['effects','Effects'],['layers','Layers'],['tuning','Tuning'],['all','All parameters']]){const b=document.createElement('button');b.type='button';b.textContent=label;b.dataset.smSection=id;b.addEventListener('click',()=>{section=id;filterSections();});nav.append(b);}document.getElementById('groups')?.before(nav);
   const mode=document.createElement('div');mode.className='card';mode.id='sm-program-mode-card';const h=document.createElement('h2');h.textContent='PROGRAM / FOUR SOUNDS';mode.append(h);const check=document.getElementById('sm-program-layer');if(check?.closest('p'))mode.append(check.closest('p'));const p=document.createElement('p');p.className='dim';p.textContent='Layer on: play enabled sounds in their keyboard zones. Layer off: play the selected sound with its mixer settings. Select a sound above to edit it.';mode.append(p);document.getElementById('groups')?.after(mode);
   scan();new MutationObserver(()=>{if(!scanQueued){scanQueued=true;requestAnimationFrame(scan);}}).observe(document.getElementById('p-sound'),{childList:true,subtree:true});
   setInterval(()=>{if(document.hidden)return;for(const [i]of dials)if(!i.isConnected)dials.delete(i);else if(i.getClientRects().length)paintDial(i);for(const [group,s]of graphics)if(!group.isConnected)graphics.delete(group);else if(group.getClientRects().length)drawGraphic(s);},100);
  }
 }
 if(document.readyState==='loading')document.addEventListener('DOMContentLoaded',ready);else ready();
})();
