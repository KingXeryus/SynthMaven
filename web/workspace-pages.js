/* SPDX-License-Identifier: GPL-3.0-only
 * Shared, presentation-only touch sections for Library and Settings. */
(() => {
 const $=id=>document.getElementById(id),make=(tag,text)=>{const n=document.createElement(tag);if(text)n.textContent=text;return n;};
 function sections(panel,entries,key){
  const nav=make('nav');nav.className='touch-tabs';nav.setAttribute('aria-label',key+' sections');panel.prepend(nav);
  let active='';
  function refresh(){
   const items=entries().filter(x=>x.node&&!x.node.hidden);if(!items.some(x=>x.id===active))active=items[0]?.id||'';
   nav.replaceChildren(...items.map(x=>{const b=make('button',x.name);b.type='button';b.setAttribute('aria-pressed',String(x.id===active));b.onclick=()=>{active=x.id;refresh();};return b;}));
   for(const x of entries())if(x.node)x.node.classList.toggle('workspace-inactive',x.id!==active);
  }
  refresh();return refresh;
 }
 const project=$('p-projects');if(project){const top=make('div'),saved=make('div');top.className='project-top';saved.className='group';saved.id='project-slots-card';for(const node of [...project.children]){if(node.id==='backup-panel')break;saved.append(node);}const backup=$('backup-panel');backup.classList.add('group');top.append(saved,backup);project.prepend(top);const song=$('songchain');const refresh=sections(project,()=>[{id:'arranger',name:'Song arranger',node:song},{id:'storage',name:'Projects & backup',node:top}],'Projects');new MutationObserver(refresh).observe(song,{attributes:true,attributeFilter:['hidden']});}
 const library=$('p-library'),bank=$('bankpane').parentElement;bank.id='library-bank-section';
 const libRefresh=sections(library,()=>[{id:'device',name:'Device sounds',node:$('device-presets')},{id:'library',name:'My library',node:$('libpane')},{id:'bank',name:'User slots',node:bank}],'Library');
 new MutationObserver(libRefresh).observe($('device-presets'),{attributes:true,attributeFilter:['hidden']});
 const system=$('sysinfo').parentElement;system.id='settings-system';
 const settings=$('p-settings');
 const settingsEntries=()=>[{id:'learn',name:'MIDI Learn',node:$('sm-mapping')},...[...$('setgroups').children].map((node,i)=>({id:'global-'+i,name:node.querySelector('h2')?.textContent.replace(/[\uE000-\uF8FF]/g,'').trim().replace(/^GLOBAL$/i,'Global')||'Global '+(i+1),node})),{id:'display',name:'Display',node:$('device-display')},{id:'recording',name:'Recording',node:$('device-recording')},{id:'system',name:'System',node:system}];
 const settingsRefresh=sections(settings,settingsEntries,'Settings');
 new MutationObserver(settingsRefresh).observe($('setgroups'),{childList:true});
 for(const id of ['sm-mapping','device-display','device-recording'])new MutationObserver(settingsRefresh).observe($(id),{attributes:true,attributeFilter:['hidden']});
 for(const id of ['device-preset-list','liblist','banklist']){
  const list=$(id),nav=make('nav');nav.className='list-pages';nav.setAttribute('aria-label',id+' pages');list.after(nav);let page=0;let query='';
  if(id==='device-preset-list'){const search=make('input');search.type='search';search.placeholder='Search preset or engine';search.setAttribute('aria-label','Search device sounds');search.oninput=()=>{query=search.value.toLowerCase().trim();draw(true);};list.before(search);}
  function draw(reset=false){
   const allRows=[...list.querySelectorAll('tr')].filter(r=>r.querySelector('td'));const rows=allRows.filter(r=>!query||r.textContent.toLowerCase().includes(query));allRows.forEach(r=>r.hidden=true);const size=innerHeight<800?4:6,total=Math.max(1,Math.ceil(rows.length/size));
   if(reset)page=0;page=Math.min(page,total-1);rows.forEach((row,i)=>row.hidden=i<page*size||i>=(page+1)*size);
   const prev=make('button','Previous'),next=make('button','Next'),label=make('span',`${rows.length? page*size+1:0}–${Math.min(rows.length,(page+1)*size)} / ${rows.length}`);
   prev.type=next.type='button';prev.disabled=page===0;next.disabled=page===total-1;prev.onclick=()=>{page--;draw();};next.onclick=()=>{page++;draw();};nav.replaceChildren(prev,label,next);
  }
  new MutationObserver(()=>draw(true)).observe(list,{childList:true,subtree:true});window.addEventListener('resize',()=>draw());draw();
 }
})();
