// SPDX-License-Identifier: GPL-3.0-only
// Library filters list only what the library holds; the sound browser has the same search and source filters.
import {readFileSync} from 'node:fs';import assert from 'node:assert/strict';import {JSDOM,VirtualConsole} from 'jsdom';
let html=readFileSync(new URL('editor.html',import.meta.url),'utf8');
html=html.replace(/<script src="([^"?]+)[^"]*"><\/script>/g,(m,f)=>'<script>'+readFileSync(new URL(f,import.meta.url),'utf8')+'</script>');
html=html.replace('function fillSelect(sel, opts) {','window.libTest={renderLib,set lib(v){lib=v;}};function fillSelect(sel, opts) {');
const dom=new JSDOM(html,{runScripts:'dangerously',pretendToBeVisual:true,virtualConsole:new VirtualConsole(),url:'http://localhost/',
  beforeParse(w){w.ResizeObserver=class{observe(){}disconnect(){}};}});
const w=dom.window,$=id=>w.document.getElementById(id),tick=()=>new Promise(r=>setTimeout(r,20));
const fire=(id,type)=>$(id).dispatchEvent(new w.Event(type,{bubbles:true}));
const shown=()=>[...$('liblist').querySelectorAll('tr')].filter(r=>r.querySelector('td')&&!r.hidden).map(r=>r.dataset.id);
const entry=(id,name,engine,engineName,tags)=>({id,name,engine,engineName,tags,p:[],created:'2026-01-0'+id.length,modified:'2026-01-0'+id.length});
w.libTest.lib=[entry('a','Brass',19,'PROPHET',['factory']),entry('bb','Bell',12,'FM6',['file']),{...entry('ccc','Old',12,'',undefined)}];
w.libTest.renderLib();await tick();
assert.deepEqual([...$('libeng').options].map(o=>o.value),['','PROPHET','FM6','12'],'engine filter offers only the engines in the library (a nameless entry offline: its index)');
$('libeng').value='FM6';fire('libeng','change');await tick();
assert.deepEqual(shown().sort(),['bb'],'engine filter matches by name');
$('libeng').value='';fire('libeng','change');$('libtag').value='factory';fire('libtag','change');await tick();
assert.deepEqual(shown(),['a'],'tag filter');
$('libtag').value='';fire('libtag','change');$('libq').value='bel';fire('libq','input');await tick();
assert.deepEqual(shown(),['bb'],'search, with an entry stored without tags');
const ps=$('preset');
ps.replaceChildren(...[['0','SOLID BASS'],['1','PAD'],['2','ALIAS'],['u:0','U001 MY BASS']].map(([v,t])=>{const o=w.document.createElement('option');o.value=v;o.textContent=t;return o;}));
ps.options[2].setAttribute('data-alias','');
const visible=()=>[...ps.options].filter(o=>!o.hidden).map(o=>o.value);
$('presetq').value='bass';fire('presetq','input');
assert.deepEqual(visible(),['0','u:0'],'sound search');assert.equal($('presetcount').textContent,'2 / 3');
$('presetkind').value='u';fire('presetkind','change');assert.deepEqual(visible(),['u:0'],'source filter: user slots');
$('presetq').value='';fire('presetq','input');$('presetkind').value='';fire('presetkind','change');
assert.deepEqual(visible(),['0','1','u:0'],'cleared filters keep aliases hidden');
console.log('Library filters: engine/tag/search on the library and search/source on the sound browser passed');
process.exit(0);
