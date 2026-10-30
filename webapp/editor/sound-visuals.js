/* SPDX-License-Identifier: GPL-3.0-only
 * SynthMaven shared web/iPad sound controls. Graphs represent parameters, not audio telemetry. */
(()=>{
 const $=id=>document.getElementById(id),make=(tag,text)=>{const n=document.createElement(tag);if(text)n.textContent=text;return n;};
 function arrows(id){
  const select=$(id);if(!select)return;select.hidden=true;
  const box=make('div'),prev=make('button','‹'),name=make('output'),next=make('button','›');box.className='sound-browser-arrows';
  prev.type=next.type='button';prev.setAttribute('aria-label','Previous '+id);next.setAttribute('aria-label','Next '+id);name.setAttribute('aria-live','polite');box.append(prev,name,next);select.after(box);
  const options=()=>[...select.options].filter(o=>!o.disabled&&!o.hidden&&!o.parentElement.disabled);
  const sync=()=>{name.textContent=select.selectedOptions[0]?.textContent||'—';prev.disabled=next.disabled=select.disabled||options().length<2;};
  function step(dir){const list=options();if(select.disabled||!list.length)return;const i=list.findIndex(o=>o.value===select.value);select.value=list[(i+dir+list.length)%list.length].value;select.dispatchEvent(new Event('change',{bubbles:true}));sync();}
  prev.onclick=()=>step(-1);next.onclick=()=>step(1);select.addEventListener('change',sync);new MutationObserver(sync).observe(select,{childList:true,subtree:true,attributes:true});sync();return sync;
 }
 const syncArrows=['engine','preset'].map(arrows).filter(Boolean);
 function paginate(card){
  if(card.dataset.voicePages)return;card.dataset.voicePages='true';const pages=[];let current=null;
  if(card.querySelector('.modrow')){
   const rows=[...card.querySelectorAll('.modrow')],templates=[...card.children].filter(n=>n.classList.contains('template-actions')||n.tagName==='P');
   const sets=[{label:'Routes 1–2',nodes:rows.slice(0,2)},{label:'Routes 3–4',nodes:rows.slice(2)}];
   if(templates.length)sets.push({label:'Templates',nodes:templates});
   const nav=make('nav');nav.className='voice-pages';nav.setAttribute('aria-label','Modulation pages');card.querySelector('h2').after(nav);
   function choose(index){sets.forEach((p,i)=>{p.nodes.forEach(n=>{n.hidden=i!==index;n.classList.toggle('voice-page-hidden',i!==index);});nav.children[i].setAttribute('aria-pressed',String(i===index));});}
   sets.forEach((p,i)=>{const b=make('button',p.label);b.type='button';b.onclick=()=>choose(i);nav.append(b);});choose(0);return;
  }

  let title='Controls',count=0;const fxSend=/FX SEND/.test(card.querySelector('h2')?.textContent||'');
  for(const child of [...card.children]){
   if(child.tagName==='H2')continue;
   if(child.tagName==='H3'){let next=child.textContent.replace(/[\uE000-\uF8FF]/g,'').trim();if(fxSend)next=['TRACK SEND','RETURN','SEND MODE'].includes(next)?'Routing':next==='SEPARATE SENDS'?'Sends':next.replace('SEND FX','Send FX');if(next!==title){title=next;current=null;count=0;}child.hidden=true;continue;}
   if(!child.classList.contains('row'))continue;
   if(!current||count===4){current={label:title+(count===4?' · more':''),nodes:[]};pages.push(current);count=0;}
   current.nodes.push(child);count++;
  }
  if(fxSend){const order=['Send FX 1','Send FX 2','Send FX 3','Routing','Sends'];pages.sort((a,b)=>order.indexOf(a.label)-order.indexOf(b.label));}
  if(pages.length<2)return;
  const nav=make('nav');nav.className='voice-pages';nav.setAttribute('aria-label',(card.querySelector('h2')?.textContent||'Sound')+' settings pages');card.querySelector('h2').after(nav);
  pages.forEach((p,index)=>{const b=make('button',p.label);b.type='button';b.onclick=()=>choose(index);nav.append(b);});
  function choose(index){pages.forEach((p,i)=>{p.nodes.forEach(n=>{n.hidden=i!==index;n.classList.toggle('voice-page-hidden',i!==index);});nav.children[i].setAttribute('aria-pressed',String(i===index));});}
  choose(0);
 }
 const graphs=new Map();
 function addGraph(card,kind){
  if(graphs.has(card))return;
  const panel=make('div');panel.className='group sound-graph';panel.dataset.graphFor=kind;panel.dataset.shapeKind=kind;
  panel.append(make('h2',kind==='filter'?'Filter response':kind==='envelope'?'Envelope shape':'Modulation routes'));
  const svg=document.createElementNS('http://www.w3.org/2000/svg','svg');svg.setAttribute('viewBox','0 0 480 240');svg.setAttribute('role','img');svg.setAttribute('aria-label',kind+' parameter visualization');panel.append(svg);
  panel.append(make('p',kind==='filter'?'Response sketch · drag to adjust cutoff / resonance':kind==='envelope'?'Parameter preview · not an audio waveform':'Source → destination · current route amounts'));
  card.after(panel);graphs.set(card,{panel,svg,kind,last:null});
  if(kind==='filter'){
   svg.style.touchAction='none';let active=false;
   const edit=e=>{const r=svg.getBoundingClientRect();for(const[label,value]of [['CUT',(e.clientX-r.left)/r.width],['RES',1-(e.clientY-r.top)/r.height]]){const input=card.querySelector(`input[title="${label}"]`);if(!input||input.disabled)continue;input.value=Math.round(+input.min+Math.max(0,Math.min(1,value))*(input.max-input.min));input.dispatchEvent(new Event('input',{bubbles:true}));}draw(card);};
   svg.onpointerdown=e=>{active=true;svg.setPointerCapture(e.pointerId);edit(e);};svg.onpointermove=e=>{if(active)edit(e);};
   svg.onpointerup=e=>{if(!active)return;edit(e);active=false;for(const label of ['CUT','RES'])card.querySelector(`input[title="${label}"]`)?.dispatchEvent(new Event('change',{bubbles:true}));};svg.onpointercancel=()=>{active=false;};
  }
 }
 const escape=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
 function draw(card){
  const g=graphs.get(card);g.panel.dataset.shapeHidden=card.dataset.shapeHidden||'false';
  const controls=[...card.querySelectorAll('input,select')],sig=controls.map(n=>n.value+':'+n.disabled).join('|');if(sig===g.last)return;g.last=sig;
  const val=label=>{const n=controls.find(n=>n.title===label);return n?Math.max(0,Math.min(1,(+n.value-+n.min)/(+n.max-+n.min||127))):0;};
  let content='<path d="M24 24V212H456 M24 118H456 M132 24V212 M240 24V212 M348 24V212" fill="none" stroke="#2A2A30"/>';
  if(g.kind==='filter'){
   if(!controls.some(n=>n.title==='CUT'))content+='<text x="240" y="120" text-anchor="middle">No exposed filter cutoff</text>';
   else {const x=36+val('CUT')*396,y=96-val('RES')*64;content+=`<path d="M24 96 H${Math.max(24,x-50)} Q${x} 96 ${x} ${y} Q${x+22} ${y} ${x+35} 145 L456 204" class="signal"/><circle cx="${x}" cy="${y}" r="9" fill="#FF2C34" stroke="#F2F2F2"/><text x="24" y="234">Low cutoff</text><text x="456" y="234" text-anchor="end">High cutoff</text>`;}
  }else if(g.kind==='envelope'){
   if(!controls.some(n=>n.title==='ATK')){g.svg.innerHTML='<text x="240" y="120" text-anchor="middle">Native envelope controls</text>';return;}const a=40+val('ATK')*100,d=40+val('DEC')*100,r=40+val('REL')*100,s=200-val('SUS')*160,scale=400/(a+d+r+80),x1=24+a*scale,x2=x1+d*scale,x3=x2+80*scale;
   content+=`<path d="M24 200 L${x1} 40 L${x2} ${s} H${x3} L424 200" class="signal"/><text x="24" y="234">Attack · Decay · Sustain · Release</text>`;
  }else{
   [...card.querySelectorAll('.modrow')].forEach((row,i)=>{const sels=row.querySelectorAll('select'),amt=row.querySelector('input'),y=42+i*46;const label=n=>n?.selectedOptions[0]?.textContent||'—';content+=`<text x="24" y="${y}">${i+1}. ${escape(label(sels[0]))}</text><path d="M160 ${y-5}H290" stroke="#9ED3F0" stroke-width="2"/><text x="300" y="${y}">${escape(label(sels[1]))}</text><text x="456" y="${y}" text-anchor="end">${escape(amt?.value||'0')}</text>`;});
  }
  g.svg.innerHTML=content;
 }
 function scan(){for(const kind of ['tone','envelope','mod']){const cards=[...$('groups').children].filter(c=>!c.classList.contains('sound-graph')&&c.dataset.shapeKind===kind);if(cards.length<(kind==='tone'?2:1)){const card=make('div');card.className='group';card.dataset.shapeKind=kind;card.dataset.shapeHidden=String(!document.querySelector('#shape-nav button[aria-pressed="true"]')?.textContent.toLowerCase().startsWith(kind==='envelope'?'envelop':kind==='mod'?'mod':kind));card.append(make('h2',kind==='tone'?'EDIT NATIVE SOUND':kind==='envelope'?'ENV':'MOD'),make('p','Use Native / import for this engine’s dedicated controls.'));$('groups').append(card);}}for(const card of [...$('groups').children]){if(card.classList.contains('sound-graph'))continue;const title=card.querySelector('h2')?.textContent||'';paginate(card);const kind=card.dataset.shapeKind;if(['filter','envelope','mod'].includes(kind))addGraph(card,kind);}for(const[card,g]of graphs)if(!card.isConnected){g.panel.remove();graphs.delete(card);}else draw(card);}
 for(const id of ['effects-groups','motionfx-groups','chords-groups']){const effects=$(id);if(effects){const refresh=()=>[...effects.children].forEach(paginate);new MutationObserver(refresh).observe(effects,{childList:true});refresh();}}
 new MutationObserver(scan).observe($('groups'),{childList:true,attributes:true,subtree:false});scan();
 // Poll only the visible editor's parameter state, including incoming MIDI updates.
 setInterval(()=>{if(document.hidden)return;syncArrows.forEach(f=>f());for(const[card]of graphs)if(card.dataset.shapeHidden!=='true')draw(card);},160);
 document.addEventListener('click',()=>{for(const[card,g]of graphs)g.panel.dataset.shapeHidden=card.dataset.shapeHidden||'false';});
})();
