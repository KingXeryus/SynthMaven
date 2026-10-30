/* SPDX-License-Identifier: GPL-3.0-only
 * Editor-only readable labels and accessible cyclic choices. Wire values remain unchanged. */
(()=>{
 const names={TUN:'Tune',TUNA:'Tune A',TUNB:'Tune B',PWA:'PW A',PWB:'PW B',MIXA:'Mix A',MIXB:'Mix B',GLMOD:'Glide mode',DTUNE:'Detune',KEYTRK:'Key tracking',KEYSYNC:'Key sync',VELSENS:'Velocity sensitivity',MODW:'Mod wheel',ATK:'Attack',DEC:'Decay',SUS:'Sustain',REL:'Release',LVL:'Level',VCE:'Voice',GLD:'Glide',PRIO:'Priority',ALLOC:'Allocation',SPRD:'Spread',PHS:'Phase',PIT:'Pitch',FLT:'Filter',SHP:'Shape',AMP:'Amplitude',SWG:'Swing',PROB:'Probability',ORD:'Order',TRIG:'Trigger',POL:'Polarity',DST:'Destination',SRC:'Source',AMT:'Amount',RES:'Resonance',CUT:'Cutoff',DRV:'Drive',OCT:'Octaves',TRNSP:'Transpose'};
 const readable=value=>{const text=value.trim();if(names[text])return names[text];const m=/^(TUN|PW|MIX|OSC|LFO|ENV|SRC|DST|AMT|MOD|SEND|WAVE)([A-D1-9])$/.exec(text);return m?`${names[m[1]]||({OSC:'Osc',WAVE:'Wave',MIX:'Mix',SEND:'Send'})[m[1]]||m[1]} ${m[2]}`:value;};
 const choices=new Map();let pending=false;
 function update(select,c){
  c.box.hidden=select.hidden;
  const options=[...select.options].filter(o=>!o.disabled&&!o.hidden&&!o.parentElement.disabled),selected=select.selectedOptions[0];
  const text=selected?readable(selected.textContent.replace(/[\uE000-\uF8FF]/g,'')):'—';if(c.output.textContent!==text)c.output.textContent=text;
  const disabled=select.disabled||options.length<2;c.prev.disabled=c.next.disabled=disabled;
  const label=select.getAttribute('aria-label')||readable(select.title)||select.closest('label')?.childNodes[0]?.textContent?.trim()||'choice';
  c.prev.setAttribute('aria-label','Previous '+label);c.next.setAttribute('aria-label','Next '+label);c.output.setAttribute('aria-label',label+': '+text);
 }
 function decorate(select){
  if(choices.has(select)||['engine','preset'].includes(select.id)||select.multiple)return;
  const box=document.createElement('span');box.className='cycle-choice';const prev=document.createElement('button'),next=document.createElement('button'),output=document.createElement('output');prev.type=next.type='button';prev.textContent='‹';next.textContent='›';output.setAttribute('aria-live','polite');
  select.classList.add('cycle-source');select.tabIndex=-1;select.before(box);box.append(prev,select,output,next);
  const c={box,prev,next,output};choices.set(select,c);
  function step(dir){if(select.disabled)return;const options=[...select.options].filter(o=>!o.disabled&&!o.hidden&&!o.parentElement.disabled);if(options.length<2)return;const i=options.findIndex(o=>o.value===select.value);select.value=options[(i+dir+options.length)%options.length].value;select.dispatchEvent(new Event('change',{bubbles:true}));update(select,c);}
  prev.onclick=()=>step(-1);next.onclick=()=>step(1);box.onkeydown=e=>{if(e.key==='ArrowLeft'||e.key==='ArrowRight'){e.preventDefault();step(e.key==='ArrowLeft'?-1:1);}};select.addEventListener('change',()=>update(select,c));update(select,c);
 }
 function refresh(){pending=false;
  for(const node of document.querySelectorAll('.row>span:first-child,label,.perform-knob>span:first-child')){
   for(const t of [...node.childNodes])if(t.nodeType===3){const value=readable(t.textContent);if(value!==t.textContent)t.textContent=value;}
  }
  document.querySelectorAll('select').forEach(decorate);
  for(const[s,c]of choices){if(!s.isConnected){c.box.remove();choices.delete(s);}else update(s,c);}
 }
 const observer=new MutationObserver(()=>{if(!pending){pending=true;queueMicrotask(()=>{observer.disconnect();refresh();observe();});}});
 function observe(){observer.observe(document.body,{childList:true,subtree:true,attributes:true,attributeFilter:['disabled','hidden']});}
 refresh();observe();setInterval(()=>{if(!document.hidden){observer.disconnect();for(const[s,c]of choices)if(s.isConnected)update(s,c);observe();}},200);
})();
