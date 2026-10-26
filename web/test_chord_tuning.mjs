// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const html=fs.readFileSync('web/editor.html','utf8');
const source=html.slice(html.indexOf('function smChordMicro(){'),html.indexOf('function paramRow('));
const maqam=JSON.parse(html.match(/const TU_MAQAM = (.*);/)[1]);
let options=[];const input={replaceChildren(...a){options=a;},value:''};
const c={desc:{max:17,names:Array.from({length:18},(_,i)=>'Mode '+i)},input};
const ctx=vm.createContext({tuv:{sys:6,deg:[],period:19200},dev:{sel:0,dump:{p:{49:14}}},smMasterTuning:[1,1,1,1],TU_MAQAM:maqam,tuCustom:null,ctl:{get:()=>c},key:()=>0,P:{CHORD:49},el:(tag,p)=>p,showValue:()=>{}});
vm.runInContext(source,ctx);const run=s=>vm.runInContext(s,ctx);
run('smChordRefresh()');assert.deepEqual(options.map(x=>x.value),[0,16,17]);assert.equal(input.value,'16');assert.equal(ctx.dev.dump.p[49],14);
run('tuv.sys=0;smChordRefresh()');assert.equal(options.length,18);assert.equal(input.value,'14');
run('tuv.sys=14;smChordRefresh()');assert.equal(options.length,18,'zero-offset Ajam keeps all choices');
run('tuv.sys=6;smMasterTuning[0]=0;smChordRefresh()');assert.equal(options.length,18);
run('smMasterTuning[0]=1;tuv.sys=58;tuCustom=Array(12).fill(0);smChordRefresh()');assert.equal(options.length,18);
run('tuCustom[4]=2;smChordRefresh()');assert.deepEqual(options.map(x=>x.value),[0,16,17]);
run('tuv={sys:13,deg:Array.from({length:12},(_,i)=>i*1600),period:19200};smChordRefresh()');assert.equal(options.length,18);
run('tuv.deg[4]-=800;smChordRefresh()');assert.deepEqual(options.map(x=>x.value),[0,16,17]);
console.log('Chord selector: microtonal/normal/zero-offset/custom/Scala/bypass choices and preserved legacy ID PASS');
