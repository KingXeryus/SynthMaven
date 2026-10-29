// GPL-3.0-only. SynthMaven Hybrid iOS shell over the existing FM-1 protocol controls.
(() => {
 const root=document.documentElement;
 const $=id=>document.getElementById(id);
 const button=(label,action)=>{const b=document.createElement('button');b.type='button';b.textContent=label;b.onclick=action;return b;};
 function ready(){
  const shell=document.createElement('header');shell.id='ios-shell';
  const toolbar=document.createElement('div');toolbar.id='ios-toolbar';
  const logo=document.createElement('img');logo.src='synthmaven-logo.svg';logo.alt='SynthMaven';
  const title=document.createElement('strong');title.id='ios-title';title.textContent='SynthMaven FM1';
  const name=document.createElement('div');name.id='ios-preset-title';name.textContent='Connect your FM-1';name.setAttribute('aria-live','polite');
  const page=id=>{root.classList.toggle('ios-sound-page',id==='sound');root.classList.remove('ios-library');document.querySelector(`[data-tab="${id}"][role=tab]`)?.click();};
  function programs(save){page('projects');setTimeout(()=>{const field=document.querySelector('#slots input');if(save)field?.focus();$('slots')?.scrollIntoView({block:'start'});},0);}
  const library=button('Presets',()=>{page('sound');root.classList.toggle('ios-library',true);$('engine')?.focus();});
  const menu=document.createElement('dialog');menu.id='ios-menu';menu.setAttribute('aria-label','Editor pages and settings');
  const menuTitle=document.createElement('h2');menuTitle.textContent='Editor pages';menu.append(menuTitle);
  const nav=document.createElement('nav');
  for(const [id,label] of [['sound','Sound'],['sequencer','Sequencer'],['tracks','Tracks'],['song','Song'],['library','Library'],['samples','Samples'],['kits','Drum kits'],['drumsynth','Drum synth'],['projects','Programs'],['settings','Settings']])nav.append(button(label,()=>{menu.close();page(id);}));
  menu.append(nav);const theme=document.querySelector('.sm-theme');if(theme)menu.append(theme);menu.append(button('Done',()=>menu.close()));document.body.append(menu);
  toolbar.append(logo,title,button('Save',()=>programs(true)),button('Load',()=>programs(false)),name,library,button('Menu',()=>menu.showModal()));
  const parts=document.createElement('div');parts.id='ios-parts';if($('trackbar'))parts.append($('trackbar'));
  const mode=document.createElement('div');mode.id='ios-mode';mode.setAttribute('role','group');mode.setAttribute('aria-label','Editor view');
  const performTabs=document.createElement('nav');performTabs.id='ios-perform-tabs';performTabs.setAttribute('aria-label','Performance pages');
  let perform=false,performancePage='macros';
  function section(id){document.querySelector(`[data-sm-section="${id}"]`)?.click();}
  function showPerformance(id){performancePage=id;root.dataset.iosPerformancePage=id;page('sound');root.classList.toggle('ios-macros',id==='macros');section(id==='macros'?'all':id);for(const b of performTabs.children)b.setAttribute('aria-pressed',String(b.dataset.page===id));markMacros();}
  const perf=button('Perform',()=>setMode(true)),edit=button('Edit',()=>setMode(false));
  function setMode(next){perform=next;root.classList.toggle('ios-perform',next);root.classList.remove('ios-library');perf.setAttribute('aria-pressed',String(next));edit.setAttribute('aria-pressed',String(!next));performTabs.hidden=!next;if(next)showPerformance(performancePage);else{root.classList.remove('ios-macros');page('sound');section('sound');}}
  mode.append(perf,edit);parts.append(mode);
  for(const [id,label] of [['macros','Macros'],['layers','Layers'],['tuning','Tuning']]){const b=button(label,()=>showPerformance(id));b.dataset.page=id;performTabs.append(b);}
  const transport=document.createElement('div');transport.id='ios-performance-navigation';
  const programName=document.createElement('output'),sceneName=document.createElement('output');programName.textContent='Program —';sceneName.textContent='Scene —';
  programName.setAttribute('aria-label','Current program');sceneName.setAttribute('aria-label','Current scene');
  const navStatus=document.createElement('span');navStatus.id='ios-nav-status';navStatus.setAttribute('role','status');
  let navigationBusy=false,navigationPoll=false,lastNavigation='';
  function renderNavigation(state){
    if(!state){programName.textContent='Program —';sceneName.textContent='Scene —';return;}
    programName.textContent=state.program===127?'Unsaved program':`P${state.program+1} · ${state.name}`;
    sceneName.textContent=state.scene===127?'No scene selected':`Scene ${state.scene+1}`;
    for(const b of transport.querySelectorAll('button'))b.disabled=navigationBusy||(Number(b.dataset.action)>=6&&!state.mask);
  }
  async function navigate(action){if(navigationBusy)return;navigationBusy=true;transport.querySelectorAll('button').forEach(b=>b.disabled=true);navStatus.textContent='Loading…';try{const state=await window.SynthMavenPerformanceNavigation.step(action);navStatus.textContent='';navigationBusy=false;renderNavigation(state);}catch(e){navigationBusy=false;navStatus.textContent=e.message;transport.querySelectorAll('button').forEach(b=>b.disabled=false);}}
  for(const [label,action] of [['Program Down',5],['Program Up',4],['Scene Down',7],['Scene Up',6]]){const b=button(label,()=>navigate(action));b.dataset.action=action;transport.append(b);if(action===5)transport.append(programName);if(action===7)transport.append(sceneName);}
  transport.append(navStatus);
  setInterval(async()=>{if(document.hidden||!perform||navigationBusy||navigationPoll)return;navigationPoll=true;try{const state=await window.SynthMavenPerformanceNavigation?.read();const sig=JSON.stringify(state);if(sig!==lastNavigation){lastNavigation=sig;renderNavigation(state);}}catch(e){navStatus.textContent=e.message;}finally{navigationPoll=false;}},1500);
  shell.append(toolbar,parts,performTabs,transport);document.querySelector('.app').prepend(shell);
  const console=document.createElement('section');console.id='ios-performance-controls';console.setAttribute('aria-label','Quick controls for selected part');
  $('p-sound').before(console);
  const imports=document.createElement('nav');imports.id='ios-preset-imports';imports.setAttribute('aria-label','Load external presets');
  const importDialog=document.createElement('dialog');importDialog.id='ios-import-dialog';importDialog.setAttribute('aria-label','Preset loader');
  const importBody=document.createElement('div');let restoreImport=null;
  importDialog.append(button('Done',()=>importDialog.close()),importBody);document.body.append(importDialog);
  importDialog.addEventListener('close',()=>{restoreImport?.();restoreImport=null;});
  function openImport(kind){
    restoreImport?.();restoreImport=null;importBody.replaceChildren();
    const id={Vital:'vital-import',DX7:'fm6panel','Minilogue XD':'korgpanel'}[kind],panel=$(id);
    if(!panel)return;
    const marker=document.createComment('preset loader position');panel.before(marker);
    const wasHidden=panel.hidden;panel.hidden=false;
    restoreImport=()=>{marker.replaceWith(panel);panel.hidden=wasHidden;};
    const title=document.createElement('h2');title.textContent=kind+' preset loader';importBody.append(title);
    if(kind!=='Vital'){
      const engine=kind==='DX7'?'FM6':'HYBRID';
      const hint=document.createElement('p');hint.textContent=`Load into the selected part using ${engine}. Switching engine replaces that part’s current sound.`;
      importBody.append(hint,button('Use '+engine+' on selected part',()=>{const select=$('engine');const option=[...select.options].find(o=>o.textContent.trim()===engine);if(option&&select.value!==option.value){select.value=option.value;select.dispatchEvent(new Event('change',{bubbles:true}));}}));
    }
    importBody.append(panel);importDialog.showModal();
  }
  for(const kind of ['Vital','DX7','Minilogue XD'])imports.append(button('Load '+kind,()=>openImport(kind)));
  console.before(imports);
  const quick=$('sm-quick-tuning');console.after(quick);
  function fitPerformance(){if(!root.classList.contains('ios-macros'))return;const top=console.getBoundingClientRect().top,extra=quick.getBoundingClientRect().height;console.style.setProperty('--performance-height',Math.max(220,innerHeight-top-extra-32)+'px');}
  new ResizeObserver(fitPerformance).observe(shell);new ResizeObserver(fitPerformance).observe(quick);window.addEventListener('resize',fitPerformance);quick.addEventListener('toggle',fitPerformance);
  new MutationObserver(fitPerformance).observe(root,{attributes:true,attributeFilter:['class']});

  const assignments=document.createElement('section');assignments.id='ios-macro-assignments';
  const assignmentTitle=document.createElement('h2');assignmentTitle.textContent='Assign Macros 1–4';
  const assignmentNote=document.createElement('p'),assignmentStatus=document.createElement('p');assignmentStatus.setAttribute('role','status');
  const assignmentRows=document.createElement('div');assignmentRows.className='ios-assignment-grid';
  const saves=document.createElement('div');saves.className='bar';
  saves.append(button('Save sound to library',()=>$('libsave')?.click()),button('Save program…',()=>programs(true)));
  assignments.append(assignmentTitle,assignmentNote,assignmentRows,assignmentStatus,saves);imports.after(assignments);
  let assignmentSignature='',assigning=false;
  function renderAssignments(){
    if(assigning)return;
    const api=window.SynthMavenMacroAssignments,state=api?.snapshot();
    const signature=JSON.stringify(state);if(signature===assignmentSignature)return;assignmentSignature=signature;
    assignmentRows.replaceChildren();assignmentNote.textContent=state?state.note+' Apply each route, then save the sound or program to keep it.':'Connect your FM-1 and select a sound to assign macros.';
    for(const b of saves.children)b.disabled=!state;
    if(!state)return;
    for(const m of state.rows){
      const card=document.createElement('div');card.className='ios-assignment-card';const heading=document.createElement('h3');heading.textContent='Macro '+(m.macro+1);card.append(heading);
      function routeEditor(route){
        const row=document.createElement('div');row.className='ios-assignment-route';
        const dest=document.createElement('select');dest.setAttribute('aria-label',`Macro ${m.macro+1} destination`);
        for(const d of m.destinations){const option=document.createElement('option');option.value=d.id;option.textContent=d.name;dest.append(option);}if(route)dest.value=route.dst;
        const amount=document.createElement('input');amount.type='number';amount.min=-64;amount.max=63;amount.step=1;amount.value=route?.amt??32;amount.setAttribute('aria-label',`Macro ${m.macro+1} amount`);
        async function apply(remove){
          if(!amount.checkValidity())return amount.reportValidity();
          assigning=true;assignments.querySelectorAll('button,select,input').forEach(x=>x.disabled=true);
          try{await api.assign(m.macro,route?.slot??-1,+dest.value,+amount.value,remove);assignmentStatus.textContent='Applied. Save the sound or program to keep this assignment.';}
          catch(e){assignmentStatus.textContent=e.message;}
          finally{assigning=false;assignmentSignature='';renderAssignments();}
        }
        row.append(dest,amount,button(route?'Apply':'Add route',()=>apply(false)));
        if(route){row.append(button('Remove',()=>apply(true)));if(route.via){const via=document.createElement('small');via.textContent='Existing VIA modulation is preserved.';row.append(via);}}
        card.append(row);
      }
      m.routes.forEach(routeEditor);routeEditor(null);assignmentRows.append(card);
    }
  }
  setInterval(()=>{if(!document.hidden&&!perform)renderAssignments();},300);renderAssignments();
  const waitingLabels=['Macro 1','Macro 2','Macro 3','Macro 4','Tone','Distortion','Delay','Reverb','LFO rate','LFO → filter','ENV attack','ENV release'];
  let signature='';
  function syncControls(){
    const liveControls=window.SynthMavenPerform?.controls()||[];
    const controls=liveControls.length?liveControls:waitingLabels.map((label,i)=>({id:-100-i,label,min:0,max:127,value:0,enabled:false,text:'Connect FM-1',context:'disconnected'}));
    const next=JSON.stringify(controls.map(c=>[c.id,c.min,c.max,c.context]));
    if(next!==signature){signature=next;console.replaceChildren();
      for(const c of controls){
        const row=document.createElement('label');row.className='ios-quick-control knob';
        const label=document.createElement('span');label.textContent=c.label;
        const dial=document.createElement('input');dial.type='range';dial.min=c.min;dial.max=c.max;dial.step=1;dial.setAttribute('aria-label',c.label);dial.dataset.param=c.id;
        const value=document.createElement('output');
        const update=final=>{window.SynthMavenPerform.set(c.id,+dial.value,final);syncControls();};
        dial.oninput=()=>update(false);dial.onchange=()=>update(true);
        row.append(label,dial,value);console.append(row);
      }
    }
    controls.forEach((c,i)=>{const row=console.children[i],input=row.querySelector('input');input.disabled=!c.enabled;input.value=c.value;input.setAttribute('aria-valuetext',c.text);const out=row.querySelector('output');if(out.textContent!==c.text)out.textContent=c.text;});
  }
  setInterval(()=>{if(!document.hidden)syncControls();},150);syncControls();
  function markMacros(){for(const group of document.querySelectorAll('#groups>.group')){const heading=group.dataset.smTitle||group.querySelector('h2')?.textContent||'';group.classList.toggle('ios-macro-group',/MACRO/i.test(heading));}}
  function refresh(){markMacros();
    for(const strip of document.querySelectorAll('.sm-layer-strip')){
      if(strip.dataset.iosMixer)continue;strip.dataset.iosMixer='true';
      const details=strip.querySelector('.sm-layer-routing>div');
      for(const extra of [strip.querySelectorAll(':scope>.sm-layer-field')[1],strip.querySelector('.sm-layer-octave'),strip.querySelector('.sm-layer-zone')])if(extra&&details)details.append(extra);
      const editSound=strip.lastElementChild;if(editSound?.tagName==='BUTTON')editSound.addEventListener('click',()=>setMode(false));
    }
const selected=$('preset')?.selectedOptions[0]?.textContent?.trim();const engine=$('engine')?.selectedOptions[0]?.textContent?.trim();const text=selected?[engine,selected].filter(Boolean).join(' / '):'Connect your FM-1';if(name.textContent!==text)name.textContent=text;}
  let queued=false;new MutationObserver(()=>{if(!queued){queued=true;requestAnimationFrame(()=>{queued=false;refresh();});}}).observe($('p-sound'),{childList:true,subtree:true});
  $('preset')?.addEventListener('change',refresh);$('engine')?.addEventListener('change',refresh);
  document.querySelectorAll('#tabs [role=tab]').forEach(b=>b.addEventListener('click',()=>{root.classList.toggle('ios-sound-page',b.dataset.tab==='sound');if(b.dataset.tab!=='sound'){root.classList.remove('ios-perform','ios-macros');performTabs.hidden=true;}}));
  setMode(true);refresh();
 }
 if(document.readyState==='loading')document.addEventListener('DOMContentLoaded',ready);else ready();
})();
