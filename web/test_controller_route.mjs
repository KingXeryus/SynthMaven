// SPDX-License-Identifier: GPL-3.0-only
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
const html=readFileSync(new URL('editor.html',import.meta.url),'utf8');
const code=html.slice(html.indexOf('const controllerForwards=new Map();'),html.indexOf('let connecting = false;'));
const sent=[],errors=[];
const fm={id:'fm',state:'connected'},pedal={id:'pedal',state:'connected',async open(){}};
const device={input:fm,output:{send:bytes=>sent.push([...bytes])}};
const c={access:{inputs:new Map([['fm',fm],['pedal',pedal]])},dev:device,window:{addEventListener(){}},say:e=>errors.push(e)};
vm.createContext(c);vm.runInContext(code,c);
await c.chooseControllerRoute('pedal');const old=pedal.onmidimessage;
pedal.onmidimessage({data:[0xb0,80,127]});pedal.onmidimessage({data:[0xc2,8]});
pedal.onmidimessage({data:[0xf0,1,2,0xf7]});pedal.onmidimessage({data:[0xf8]});
assert.deepEqual(sent,[[0xb0,80,127],[0xc2,8]]);
c.stopControllerRoute();assert.equal(pedal.onmidimessage,null);assert.equal(sent.length,34);
for(let ch=0;ch<16;ch++)assert.deepEqual(sent.slice(2+ch*2,4+ch*2),[[0xb0|ch,64,0],[0xb0|ch,123,0]]);
old({data:[0x90,60,100]});assert.equal(sent.length,34);
await assert.rejects(c.chooseControllerRoute('fm'),/unavailable/);
pedal.state='disconnected';await assert.rejects(c.chooseControllerRoute('pedal'),/unavailable/);
assert.equal(errors.length,0);
// Several controllers, each moved to its target's assigned channel.
pedal.state='connected';const pads={id:'pads',state:'connected'};c.access.inputs.set('pads',pads);device.routes={enabled:1,input:[0,0,0,0,1],channel:[1,2,0,0,10]};
sent.length=0;await c.setControllerForward('pads','4');await c.setControllerForward('pedal','1');
pads.onmidimessage({data:[0x93,36,100]});pedal.onmidimessage({data:[0xb0,64,127]});assert.deepEqual(sent,[[0x99,36,100],[0xb1,64,127]]);
sent.length=0;c.stopControllerForward('pads');assert.deepEqual(sent,[[0xb9,64,0],[0xb9,123,0]]);assert.ok(pedal.onmidimessage);c.stopControllerRoute();
console.log('Controller route: CC/PC forwarding, SysEx/clock isolation, note cleanup, stale callback and endpoint rejection passed');
