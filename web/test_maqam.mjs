// SPDX-License-Identifier: GPL-3.0-only
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
const html=readFileSync(new URL('./editor.html',import.meta.url),'utf8');
const catalog=JSON.parse(readFileSync(new URL('../assets/maqam/scales.json',import.meta.url),'utf8'));
const defs=html.slice(html.indexOf('const TU_NAMES = ['),html.indexOf('let tuv = null;'));
const ctx=vm.createContext({dev:{info:{proto:18}}});
vm.runInContext(defs+';this.names=TU_NAMES;this.maqams=TU_MAQAM;this.user=TU_USER;'+html.match(/const tuSystems = [^\n]+/)[0]+';this.systems=tuSystems;',ctx);
assert.equal(ctx.names.length,62);assert.equal(ctx.user,13);assert.equal(ctx.maqams.length,46);
for(let i=0;i<catalog.length;i++){
 const row=catalog[i],web=ctx.maqams[i],sys=i<2?i+6:i+12;
 assert.equal(web.sys,sys);assert.equal(ctx.names[sys],row.name.toUpperCase());
 assert.deepEqual(Array.from(web.offsets),row.offsets);assert.equal(row.offsets[0],0);
 for(const c of row.degrees)assert(row.offsets.some((v,k)=>k*100+v===c),row.name+': every reference degree is reachable');
}
const pitches=JSON.parse(readFileSync(new URL('../build/host/maqam-pitches.json',import.meta.url),'utf8'));
assert.equal(pitches.length,46*12);
for(const p of pitches){const row=ctx.maqams.find(x=>x.sys===p.sys);assert(row);assert.equal(p.pitches.length,128);for(let key=0;key<128;key++)assert.equal(p.pitches[key],key<12?0:row.offsets[(key+12-p.root)%12]*4096/100,`${row.name} root ${p.root}, MIDI ${key}`);}
assert.equal(ctx.systems().length,62);ctx.dev.info.proto=16;assert.equal(ctx.systems().length,14);ctx.dev.info.proto=14;assert.equal(ctx.systems().length,14);
assert(html.includes('sys: TU_USER, deg: sc.deg'));
const source=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(source+';({makeMockDevice,Link,req,parse,CMD})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder,Blob,Response,DecompressionStream});
for(const modern of [false,true]){
 const mock=E.makeMockDevice({synthmaven:true,oriental:true,modern,maqam:modern,auto:false});
 const input=[...mock.access.inputs.values()][0],output=[...mock.access.outputs.values()][0];
 const link=new E.Link(d=>output.send(d),{timeout:500});input.onmidimessage=e=>link.receive(e.data);const rq=r=>link.request(r);
 try{
  const info=E.parse[E.CMD.INFO](await rq(E.req.info()));assert.equal(info.proto,modern?26:16);
  if(modern){
   const encode=n=>[(n+8192)&127,(n+8192)>>7],decode=(a,i)=>a[i]+128*a[i+1]-8192;
   const cents=[-100,-75,-51,-37,-1,0,1,17,49,62,99,100];
   let r=await rq([121,[2,...cents.flatMap(encode)]]);assert.equal(r[0],0);
   r=await rq([121,[2]]);assert.deepEqual(Array.from({length:12},(_,i)=>decode(r,2+2*i)),cents);
   r=await rq([121,[2,...[101,...cents.slice(1)].flatMap(encode)]]);assert.equal(r[0],1);
   r=await rq([121,[2]]);assert.equal(decode(r,2),-100,'bad write is atomic');
   r=await rq([107,[2,...Array(12).fill(2)]]);assert.equal(r[0],0);
   r=await rq([121,[2]]);assert.equal(decode(r,2),-50,'legacy write clears precise override');
  }
  for(const sys of [6,7,13,14,57,58,61,62]){
   const rc=E.parse[E.CMD.TUNE_PUT](await rq(E.req.tunePut(sys,11,[0,1600,3200],19200,432))).rc;
   assert.equal(rc,sys<(modern?62:14)?0:1);
   if(!rc){const got=E.parse[E.CMD.TUNE_GET](await rq(E.req.tuneGet()));assert.equal(got.sys,sys);assert.equal(got.root,11);assert.equal(got.a4,432);}
  }
 }finally{link.close();mock.stop();}
}
console.log('Maqam catalogue: 46 reference settings match firmware at 12 roots / 128 keys; editor names, protocol 17, old-firmware gating and stable Scala ID PASS');
