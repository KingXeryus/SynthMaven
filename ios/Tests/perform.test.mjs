import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
const calls=[];const desc=Array.from({length:126},()=>({min:0,max:127}));
const ctx={window:{},dev:{info:{pe0:118,engines:['ANALOG','DIGITAL']},dump:{engine:0,p:Array(126).fill(64)},pdesc:desc},SM_FILTER_MAP:[{engine:0,cutoff:'P_E4'},{engine:1,cutoff:'-1'}],fmtValue:(_,v)=>[String(v),'%'],userSet:(...a)=>calls.push(a)};
vm.runInNewContext(readFileSync('ios/UI/perform-bindings.js','utf8'),ctx);
const api=ctx.window.SynthMavenPerform;assert.equal(api.controls().length,12);
for(const [label,id] of [['Tone',122],['Distortion',33],['Delay',35],['Reverb',36]]){assert.equal(api.controls().find(c=>c.label===label).id,id);api.set(id,77);assert.deepEqual(calls.at(-1),[0,id,77,true]);}
api.set(33,900);assert.equal(calls.at(-1)[2],127);
ctx.dev.dump.engine=1;assert.equal(api.controls().find(c=>c.label==='Tone').enabled,false);const n=calls.length;api.set(-1,40);assert.equal(calls.length,n);
ctx.dev.info.engines[1]='HYBRID';assert.equal(api.controls()[0].id,122);
ctx.dev=null;assert.equal(api.controls().length,0);
console.log('Perform controls: mappings, limits, engine capabilities and disconnect PASS');
