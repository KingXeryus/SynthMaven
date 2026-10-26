// SPDX-License-Identifier: GPL-3.0-only
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
const html=readFileSync(new URL('./editor.html',import.meta.url),'utf8');
const src=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(src+';({makeMockDevice,Link,req,parse,CMD,bkProjFormat,bkDevFormat,backupCapture,backupRestore})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder,Blob,Response,DecompressionStream});
const mock=E.makeMockDevice({modern:true,maqam:true,synthmaven:true,oriental:true,auto:false}),input=[...mock.access.inputs.values()][0],output=[...mock.access.outputs.values()][0];
const link=new E.Link(d=>output.send(d),{timeout:500});input.onmidimessage=e=>link.receive(e.data);const rq=r=>link.request(r);
try{
 const info=E.parse[E.CMD.INFO](await rq(E.req.info()));assert.equal(info.proto,21);assert.equal(info.ntrk,5);
 for(let i=0;i<4;i++){const a=await rq([109,[i]]);assert.equal(a.length,11);assert.equal(a[0],0);assert.equal(a[1],i);assert.equal(a[2],1);}
 for(let i=0;i<4;i++)assert.deepEqual(Array.from(await rq([110,[i]])),[0,i,1]);
 assert.deepEqual(Array.from(await rq([110,[2,0]])),[0,2,0]);assert.deepEqual(Array.from(await rq([110,[1]])),[0,1,1]);
 for(const a of [[],[4],[2,2],[2,0,1]])assert.equal((await rq([110,a]))[0],1);
 assert.deepEqual(Array.from(await rq([110,[2]])),[0,2,0]);
 const decode=a=>String.fromCharCode(...a.slice(3)).replace(/\0.*$/,"");
 for(let i=0;i<8;i++){
  const name="PAD "+(i+1),a=[i,...Array.from({length:12},(_,k)=>name.charCodeAt(k)||0)];
  const before=E.parse[E.CMD.PROJECT](await rq(E.req.project(2,i))).used;
  assert.equal((await rq([111,a]))[0],0);assert.equal(E.parse[E.CMD.PROJECT](await rq(E.req.project(2,i))).used,before,"name staging does not save a program");
  assert.equal(E.parse[E.CMD.PROJECT](await rq(E.req.project(1,i))).used,1);assert.equal(decode(await rq([111,[i]])),name);
 }
 for(const bad of [[],[8],[0,1],[7,...Array(12).fill(127)]])assert.equal((await rq([111,bad]))[0],1);
 assert.throws(()=>E.req.project(1,8));assert.equal(E.parse[E.CMD.PROJECT](await rq([E.CMD.PROJECT,[2,8]])).rc,2);
 await rq([111,[0,...Array.from({length:12},(_,k)=>"FIRST".charCodeAt(k)||0)]]);await rq([111,[7,...Array.from({length:12},(_,k)=>"EIGHTH".charCodeAt(k)||0)]]);
 assert.equal(decode(await rq([111,[0]])),"FIRST");assert.equal(decode(await rq([111,[7]])),"EIGHTH");
 await rq([111,[0,...Array.from({length:12},(_,k)=>"PAD 1".charCodeAt(k)||0)]]);await rq(E.req.project(1,0));await rq([111,[7,...Array.from({length:12},(_,k)=>"PAD 8".charCodeAt(k)||0)]]);await rq(E.req.project(1,7));
 const snapshot=await E.backupCapture(rq,info);await rq([111,[7,...Array.from({length:12},(_,k)=>"CHANGED".charCodeAt(k)||0)]]);await rq(E.req.project(1,7));
 await E.backupRestore(rq,JSON.parse(JSON.stringify(snapshot)));for(let i=0;i<8;i++)assert.equal(decode(await rq([111,[i]])),"PAD "+(i+1),"backup restores all eight committed names");
 const pairMagic=Uint8Array.of(0x50,0x42,0x4b,0x31);assert.equal(E.bkProjFormat(pairMagic),9);assert.equal(E.bkDevFormat({proto:21,version:"SynthMaven 1.7"}),9);assert(E.bkProjFormat(pairMagic)>E.bkDevFormat({proto:20,version:"SynthMaven 1.6"}));
 const value=[2,1,1,1,60,12,80,0,1,1];let r=await rq([109,value]);assert.deepEqual(Array.from(r),[0,...value]);assert.deepEqual(Array.from(await rq([109,[2]])),[0,...value]);
 for(const bad of [[4],[],[0,1],[2,2,...value.slice(2)],[...value.slice(0,5),49,...value.slice(6)]])assert.equal((await rq([109,bad]))[0],1);
 assert.deepEqual(Array.from(await rq([109,[2]])),[0,...value]);
 const newProgram=Uint8Array.of(0x50,0x52,0x47,0x32),oldProgram=Uint8Array.of(0x50,0x52,0x47,0x31);
 assert.equal(E.bkProjFormat(newProgram),8);assert.equal(E.bkProjFormat(oldProgram),7);
 assert.equal(E.bkDevFormat({proto:19,version:'SYNTHMAVEN 1.5'}),8);assert.equal(E.bkDevFormat({proto:18,version:'SYNTHMAVEN 1.4'}),7);
 assert(E.bkProjFormat(newProgram)>E.bkDevFormat({proto:18,version:'SYNTHMAVEN 1.4'}));
 console.log('Four-sound mixer: protocol 19, all slots, split/octave/level/pan/mute/solo, atomic refusal and backup downgrade guard PASS');
}finally{link.close();mock.stop();}
