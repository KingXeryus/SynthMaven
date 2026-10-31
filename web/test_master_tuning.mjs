// SPDX-License-Identifier: GPL-3.0-only
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
import {JSDOM} from 'jsdom';
const html=readFileSync(new URL('editor.html',import.meta.url),'utf8');
const proto=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const ui=html.slice(html.indexOf('async function rereadChordDesc('),html.indexOf('function buildUI()'));
const dom=new JSDOM('<div id="master-tuning"></div>');
const writes=[],errors=[];
const d={info:{synthmaven:{masterTuning:true}},masterTuning:{enabled:1,scale:1,root:0,octave:3,quantize:0,apply:15,cents:Array(12).fill(0),mask:4095}};
const context={document:dom.window.document,dev:d,$:id=>dom.window.document.getElementById(id),say:e=>errors.push(e),setTimeout,clearTimeout,setInterval,clearInterval,console,
 sessionRequest:async(_d,req)=>{writes.push(req);return [1,0,...req[1].slice(1)];}};
vm.runInNewContext(proto+'\n'+ui+'\nrenderMasterTuning();',context);
const keys=()=>[...dom.window.document.querySelectorAll('.mt-keyboard button')];
assert.equal(keys().length,12);
assert.match(keys()[4].className,/tuned/);
keys()[4].click();assert.equal(d.tuningKey,4);assert.equal(d.masterTuning.scale,1);
const hold=async(k,ms)=>{const b=keys()[k];b.onpointerdown();await new Promise(r=>setTimeout(r,ms));if(b.isConnected){b.onpointerup();}await d.tuningQueue;};
keys()[4].click();assert.equal(d.masterTuning.scale,1,'a second tap does not change tuning');
await hold(4,300);assert.equal(d.masterTuning.scale,1,'a short press does not change tuning');
await hold(4,1050);assert.equal(d.masterTuning.scale,47);assert.equal(d.masterTuning.cents[4],0);
await hold(4,1050);assert.equal(d.masterTuning.cents[4],-50);keys()[4].click();assert.equal(d.masterTuning.cents[4],-50,'the click ending a hold changes nothing');
const slider=dom.window.document.querySelector('input[type=range]');
assert.equal(slider.min,'-100');assert.equal(slider.max,'100');assert.equal(slider.step,'1');
slider.value='-37';slider.dispatchEvent(new dom.window.Event('change'));await d.tuningQueue;
assert.equal(d.masterTuning.cents[4],-37);assert.match(keys()[4].textContent,/-37 ct/);
const selects=dom.window.document.querySelectorAll('select');assert.equal(selects[0].options.length,62);assert.equal(selects[2].options.length,9);
selects[2].value='8';selects[2].dispatchEvent(new dom.window.Event('change'));await d.tuningQueue;assert.equal(d.masterTuning.octave,8);
const checks=dom.window.document.querySelectorAll('input[type=checkbox]');checks[3].checked=false;checks[3].dispatchEvent(new dom.window.Event('change'));await d.tuningQueue;assert.equal(d.masterTuning.apply,13);
assert.equal(errors.length,0);assert.ok(writes.length>=5);
console.log('Master tuning editor: selection, one-second hold, one-cent wheel, keyboard value, octave controls and per-part bypass passed');
const storage=new Map();context.localStorage={getItem:k=>storage.get(k)||null,setItem:(k,v)=>storage.set(k,v)};
vm.runInNewContext('',context); // same persistent context created by runInNewContext above
const run=code=>vm.runInNewContext(code,context);
const scl='! test\nJust major\n7\n9/8\n5/4\n4/3\n3/2\n5/3\n15/8\n2/1';
context.scl=scl;const parsed=run('parseScala(scl)');assert.equal(parsed.cents[4],-14);assert.equal(parsed.mask,2741);
for(const invalid of ['Bad\n13\n2/1','Bad\n1\n3/1','Bad\n2\n0/1\n2/1','Bad\n3\n100.0\n110.0\n2/1']){context.scl=invalid;assert.throws(()=>run('parseScala(scl)'));}
d.tuningView='saved';run('renderMasterTuning()');
const byText=text=>[...dom.window.document.querySelectorAll('button')].find(b=>b.textContent===text);
const name=dom.window.document.querySelector('[aria-label="Custom scale name"]');name.value='My Rast';byText('Save as new').click();assert.equal(JSON.parse([...storage.values()][0])[0].name,'My Rast');
d.masterTuning.cents[4]=0;byText('Load scale').click();await d.tuningQueue;assert.equal(d.masterTuning.cents[4],-37);assert.equal(d.masterTuning.octave,8);assert.equal(d.masterTuning.apply,13);
byText('Save as new').click();assert.match(dom.window.document.querySelector('[role=status]').textContent,/already saved/);assert.equal(JSON.parse([...storage.values()][0]).length,1);
console.log('Custom scale library and Scala: saved tuning restored without changing routing, duplicates rejected, ratio parsing and incompatible scales checked');
