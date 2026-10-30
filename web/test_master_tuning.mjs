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
keys()[4].click();await d.tuningQueue;assert.equal(d.masterTuning.scale,47);assert.equal(d.masterTuning.cents[4],0);
keys()[4].click();keys()[4].click();await d.tuningQueue;assert.equal(d.masterTuning.cents[4],-50);
const slider=dom.window.document.querySelector('input[type=range]');
assert.equal(slider.min,'-100');assert.equal(slider.max,'100');assert.equal(slider.step,'1');
slider.value='-37';slider.dispatchEvent(new dom.window.Event('change'));await d.tuningQueue;
assert.equal(d.masterTuning.cents[4],-37);assert.match(keys()[4].textContent,/-37 ct/);
const selects=dom.window.document.querySelectorAll('select');assert.equal(selects[0].options.length,48);assert.equal(selects[2].options.length,9);
selects[2].value='8';selects[2].dispatchEvent(new dom.window.Event('change'));await d.tuningQueue;assert.equal(d.masterTuning.octave,8);
const checks=dom.window.document.querySelectorAll('input[type=checkbox]');checks[3].checked=false;checks[3].dispatchEvent(new dom.window.Event('change'));await d.tuningQueue;assert.equal(d.masterTuning.apply,13);
assert.equal(errors.length,0);assert.ok(writes.length>=5);
console.log('Master tuning editor: selection, double-tap, one-cent wheel, keyboard value, octave controls and per-part bypass passed');
