// SPDX-License-Identifier: GPL-3.0-only
import {readFileSync} from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';import {JSDOM} from 'jsdom';
const html=readFileSync(new URL('editor.html',import.meta.url),'utf8'),dom=new JSDOM('<div id="sm-mapping"></div>'),document=dom.window.document;
const el=(tag,attrs={},...children)=>{const n=document.createElement(tag);for(const[k,v]of Object.entries(attrs)){if(k==='text')n.textContent=v;else if(k.startsWith('on'))n[k]=v;else if(k==='disabled'||k==='hidden')n[k]=v;else n.setAttribute(k,v);}for(const c of children)n.append(c);return n;};
const errors=[],sent=[];
const dev={mapping:{maps:Array.from({length:24},()=>Array(12).fill(0)),learn:0,serial:0},info:{pe0:20},dump:{engine:0},pdesc:[],input:{},
 routes:{enabled:0,input:[0,0,0,0,0],channel:[1,2,3,4,10],drumPart:3},routeLayers:{enabled:0,low:[0,0,0,0],high:[127,127,127,127]}};
const CMD={ROUTES:118,LAYERS:111};
const ctx={document,window:{},dev,el,$:id=>document.getElementById(id),access:null,controllerForwards:new Map(),pack7:x=>x,say:x=>errors.push(x),CMD,
 sessionRequest:async(d,[cmd,args])=>{sent.push([cmd,args]);return {cmd,args};},
 parse:{[CMD.ROUTES]:r=>({rc:0,drumPart:3,...ctx.routesDecode(r.args.slice(1))}),[CMD.LAYERS]:r=>({layers:ctx.__layers})}};
ctx.__REQ__={layersPut:s=>{ctx.__layers=s;return [CMD.LAYERS,[1]];}};
vm.createContext(ctx);
const start=html.indexOf('function routesEncode'),end=html.indexOf('function layersDecode');vm.runInContext(html.slice(start,end),ctx);
vm.runInContext(html.slice(html.indexOf('const SM_ACTIONS='),html.indexOf('async function mappingRequest')),ctx);
vm.runInContext(html.slice(html.indexOf('function renderMapping(){'),html.indexOf('\nsetInterval(async()=>{if(!dev?.info.synthmaven?.mapping')).replaceAll(/\b[A-Za-z_]+\.layersPut\(/g,'__REQ__.layersPut('),ctx);
const q=s=>document.querySelector(s),button=text=>[...document.querySelectorAll('button')].find(b=>b.textContent===text),tick=()=>new Promise(r=>setImmediate(r));
const pick=(label,value)=>{const n=q(`[aria-label="${label}"]`);n.value=String(value);n.onchange();};
ctx.renderMapping();
assert.equal(button('Routing').getAttribute('aria-pressed'),'true','routing opens first when the firmware has it');
assert.equal(q('.midi-route-table').hidden,true,'off by default: ROUT applies');assert.equal(button('Save to FM-1').disabled,true);
const toggle=q('[aria-label="Use custom routing"]');toggle.checked=true;toggle.onchange();
assert.equal(q('.midi-route-table').hidden,false);assert.equal(button('Save to FM-1').disabled,false);
pick('Layer 1 input',2);pick('Layer 1 channel',1);pick('Layer 2 input',2);pick('Layer 2 channel',1);pick('Layer 1 highest note',59);pick('Layer 2 lowest note',60);
assert.match(q('.midi-route-notice').textContent,/Layer 1 and Layer 2 split TRS, channel 1/);
pick('Layer 2 lowest note',50);assert.equal(q('.midi-route-notice').dataset.kind,'stack');pick('Layer 2 lowest note',60);
pick('Drummer input',2);pick('Drummer channel',1);assert.equal(q('[data-kind=conflict]')?.textContent.includes('Drummer both receive TRS, channel 1'),true);assert.equal(button('Save to FM-1').disabled,true,'a conflict blocks saving');
pick('Layer 3 channel',0);pick('Drummer input',1);pick('Drummer channel',17);assert.equal(q('[data-kind=conflict]'),null);
assert.equal(q('[aria-label="Layer 4 channel"]').disabled,true,'the drummer part is not also a layer');
button('Save to FM-1').click();await tick();
assert.deepEqual(sent.map(x=>x[0]),[CMD.LAYERS,CMD.ROUTES]);assert.deepEqual(Array.from(sent[1][1].slice(0,6)),[1,0x53,0x4d,0x52,1,1]);
assert.equal(dev.routes.channel[4],17);assert.deepEqual(Array.from(ctx.__layers.high).slice(0,2),[59,127]);
button('Learn & actions').click();assert.equal(q('.midi-routing'),null);
dev.mapping.maps[0]=[1,0,0,80,127,0,4,9,80,0,1,0];ctx.renderMapping();assert.match(q('.learned-action strong').textContent,/Transport Record \(selected part\)/);
dev.mapping.maps[0]=[1,0,0,80,127,0,4,9,80,0,255,0];ctx.renderMapping();assert.match(q('.learned-action strong').textContent,/All notes off/);
assert.deepEqual(errors,[]);console.log('MIDI routing UI: off by default, split/stack/conflict notices, saved records and transport actions passed');
