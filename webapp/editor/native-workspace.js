/* SPDX-License-Identifier: GPL-3.0-only
 * Shared native/import workspace. Moving controls preserves their staged patch handlers. */
(()=>{
 const $=id=>document.getElementById(id),make=(tag,text)=>{const n=document.createElement(tag);if(text)n.textContent=text;return n;};
 const host=$('p-native'),nav=make('nav'),shell=make('div'),source=make('div'),edit=make('div'),tools=make('div');
 nav.className='touch-tabs';nav.setAttribute('aria-label','Native patch format');shell.className='native-layout';source.className=edit.className='group native-card';tools.className='native-file-tools';
 shell.append(source,edit);host.prepend(nav,shell);let selected='',locked=false;
 const specs=[['hybridpanel','HYBRID / Vital','hycontrols'],['p5panel','Prophet','p5controls'],['czpanel','CZ-1','czcontrols']];
 function paginate(root){
  const nodes=[...root.children];if(!nodes.length){const message=make('p','Read a part or import a patch to open its native controls.');message.className='dim';root.append(message);return;}
  const pages=[];let label='Patch',items=[];
  const flush=()=>{if(items.length){for(let i=0;i<items.length;i+=4)pages.push({label:label+(items.length>4?' · '+(1+i/4):''),nodes:items.slice(i,i+4)});items=[];}};
  function walk(node){
   if(['H3','LEGEND','SUMMARY'].includes(node.tagName)){flush();label=node.textContent.trim();return;}
   if(node.tagName==='DETAILS'||node.tagName==='FIELDSET'||node.classList.contains('group')){flush();for(const n of [...node.children])walk(n);flush();return;}
   if(node.tagName==='TABLE'){flush();const rows=[...node.querySelectorAll('tbody tr')];for(let i=0;i<rows.length;i+=4){const t=node.cloneNode(false);if(node.tHead)t.append(node.tHead.cloneNode(true));const body=make('tbody');body.append(...rows.slice(i,i+4));t.append(body);pages.push({label:label+' · '+(i/4+1),nodes:[t]});}return;}
   // CZ bars contain several independent labeled fields.
   if(node.classList.contains('bar')&&node.querySelectorAll(':scope>label').length>1){for(const n of [...node.children])items.push(n);return;}
   items.push(node);
  }
  nodes.forEach(walk);flush();root.replaceChildren();
  const toolbar=make('nav'),prev=make('button','‹'),next=make('button','›'),picker=make('select'),body=make('div');toolbar.className='native-page-nav';body.className='native-fields';picker.setAttribute('aria-label','Native parameter section');prev.setAttribute('aria-label','Previous parameter page');next.setAttribute('aria-label','Next parameter page');
  pages.forEach((p,i)=>{const o=make('option',p.label);o.value=i;picker.append(o);});toolbar.append(prev,picker,next);root.append(toolbar,body);let page=0;
  const draw=()=>{picker.value=page;body.replaceChildren(...pages[page].nodes);prev.disabled=page===0;next.disabled=page===pages.length-1;};
  prev.onclick=()=>{page--;draw();};next.onclick=()=>{page++;draw();};picker.onchange=()=>{page=+picker.value;draw();};if(pages.length)draw();
 }
 for(const[, ,id]of specs){const root=$(id);const observe=new MutationObserver(()=>{observe.disconnect();paginate(root);observe.observe(root,{childList:true});});paginate(root);observe.observe(root,{childList:true});}
 function render(){
  if(locked)return;locked=true;
  const available=specs.filter(([id])=>!$(id).hidden);if(!available.some(x=>x[0]===selected))selected=available[0]?.[0]||'';
  nav.replaceChildren(...available.map(([id,label])=>{const b=make('button',label);b.setAttribute('aria-pressed',String(id===selected));b.onclick=()=>{selected=id;render();};return b;}));
  // Keep panels as ownership containers; only the selected panel is displayed inside the shell.
  for(const[id]of specs)$(id).classList.toggle('native-inactive',id!==selected);
  source.replaceChildren(make('h2','Import & patch files'));edit.replaceChildren(make('h2','Native sound controls'));
  if(selected){const panel=$(selected),root=$(specs.find(x=>x[0]===selected)[2]);
   for(const node of [...panel.children])if(node!==root&&node.tagName!=='H2')source.append(node);
   edit.append(root);
  }else edit.append(make('p','Choose a supported native engine to edit its patch.'));
  source.append(tools);locked=false;
 }
 // Retain source controls in their original panel when changing format/rebuilding navigation.
 const homes=new Map();for(const[id,,rootId]of specs)for(const n of [...$(id).children])homes.set(n,$(id));
 function restore(){for(const[n,parent]of homes)if(n.parentNode!==parent)parent.append(n);}
 const originalRender=render;render=()=>{restore();originalRender();};
 const obs=new MutationObserver(()=>{obs.disconnect();const bar=[...host.children].find(n=>n.classList.contains('bar'));if(bar)tools.append(bar);render();obs.observe(host,{childList:true});});obs.observe(host,{childList:true});
 for(const[id]of specs)new MutationObserver(()=>{if(!locked)render();}).observe($(id),{attributes:true,attributeFilter:['hidden']});
 const initialBar=[...host.children].find(n=>n.classList.contains('bar'));if(initialBar)tools.append(initialBar);
 render();
})();
