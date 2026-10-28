// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const html=fs.readFileSync('web/editor.html','utf8');const source=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(source+';({makeMockDevice,Link,req,parse,CMD})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder,Blob,Response,DecompressionStream});
for(const modern of [true,false]){
 const m=E.makeMockDevice({synthmaven:true,modern,oriental:true,auto:false});const input=[...m.access.inputs.values()][0],out=[...m.access.outputs.values()][0],link=new E.Link(d=>out.send(d));input.onmidimessage=e=>link.receive(e.data);const rq=r=>link.request(r);
 try{
 let get=()=>rq(E.req.tuneGet()).then(a=>E.parse[E.CMD.TUNE_GET](a));let state=await get();assert.equal(state.floor,modern?0:null);
 if(!modern)continue;
 for(let floor=0;floor<9;floor++){
 const a=await rq(E.req.tunePut(6,0,state.deg,state.period,440,floor));assert.equal(a[0],0);state=await get();assert.equal(state.floor,floor);
 }
 const old=E.req.tunePut(6,0,state.deg,state.period,440);assert.equal((await rq(old))[0],0);assert.equal((await get()).floor,8);
 const bad=E.req.tunePut(6,0,state.deg,state.period,440,3);bad[1][bad[1].length-1]=9;assert.equal((await rq(bad))[0],1);assert.equal((await get()).floor,8);
 assert(html.includes('Array.from({length:9}') && html.includes('aria-label="Apply tuning from octave"'));
 }finally{link.close();m.stop();}
}
console.log('Tuning threshold: nine octave choices, native-compatible optional byte, atomic rejection and old-device gating PASS');
