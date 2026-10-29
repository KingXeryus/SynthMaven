// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const html=fs.readFileSync('web/editor.html','utf8'),src=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(src+';({makeMockDevice,Link,req,parse,CMD,SM_FACTORY_PROGRAM_BANK,backupCapture,backupRestore})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder,Blob,Response,DecompressionStream});
const m=E.makeMockDevice({synthmaven:true,modern:true,oriental:true,auto:false}),input=[...m.access.inputs.values()][0],out=[...m.access.outputs.values()][0],link=new E.Link(d=>out.send(d));input.onmidimessage=e=>link.receive(e.data);const rq=r=>link.request(r);
try{
 const info=E.parse[E.CMD.INFO](await rq(E.req.info()));
 const wanted=['Piano','Epiano','Lead Synth','Oriental Lead','Disco Synth','Brass','Strings','Bass'];
 assert.deepEqual(Array.from(E.SM_FACTORY_PROGRAM_BANK,p=>p.style),wanted);
 for(let i=0;i<8;i++){
  const f=E.SM_FACTORY_PROGRAM_BANK[i];let name=await rq([111,[i]]);assert.equal(name[2],1);assert.equal(String.fromCharCode(...name.slice(3)).replace(/\0.*$/,''),f.name);
  assert.equal((await rq([118,[i]]))[0],0);assert.equal((await rq([108,[]]))[2],1);assert.equal((await rq([113,[]]))[1],15,'all four factory scenes available');
  for(let part=0;part<4;part++){await rq(E.req.track(part));const d=E.parse[E.CMD.DUMP](await rq(E.req.dump()),info);assert.deepEqual(Array.from(d.p),Array.from(f.parts[part].p));const mix=await rq([109,[part]]);assert.equal(mix[2],1);assert.equal(mix[7],f.parts[part].p[0]);}
 }
 await rq([111,[0,...Array.from({length:12},(_,k)=>'MY PIANO'.charCodeAt(k)||0)]]);await rq(E.req.project(1,0));
 await rq([118,[0]]);const name=await rq([111,[0]]);assert.equal(String.fromCharCode(...name.slice(3)).replace(/\0.*$/,''),'MY PIANO','ROM recall does not overwrite saved name');
 assert.equal((await rq([118,[8]]))[0],1);assert.equal((await rq([118,[]]))[0],1);
 const backup=await E.backupCapture(rq,info);const oldInfo=Array.from(await rq(E.req.info()));oldInfo[oldInfo.length-1]=23;let writes=0;
 await assert.rejects(()=>E.backupRestore(async r=>{if(r[0]===E.CMD.INFO)return oldInfo;writes++;return rq(r);},backup));assert.equal(writes,0,'old firmware refuses 2.2 gains before any restore write');
 assert(html.includes('Saved program slots are unchanged.')&&html.includes('Load factory program'));
}finally{link.close();m.stop();}
console.log('Factory programs: exact P1-P8 instrument order, native catalogue parameters, four active layers and saved override preservation PASS');
