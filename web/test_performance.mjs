import {readFileSync} from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const html=readFileSync(new URL('./editor.html',import.meta.url),'utf8'),src=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(src+';({makeMockDevice,Link,PERF,pack7,SM_RELEASE,bkProjFormat,bkDevFormat})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder,Blob,Response,DecompressionStream});
const mock=E.makeMockDevice({modern:true,maqam:true,synthmaven:true,oriental:true,auto:false,factoryPrograms:false}),input=[...mock.access.inputs.values()][0],output=[...mock.access.outputs.values()][0],link=new E.Link(d=>output.send(d),{timeout:500});input.onmidimessage=e=>link.receive(e.data);const rq=(cmd,a=[])=>link.request([cmd,a]);
try{
 let s=E.PERF.status(await rq(112));assert.equal(s.repeat,0);await rq(112,[1,4]);mock.sim.pedal(1,3,64,127,100);s=E.PERF.status(await rq(112));assert.equal(s.learn,2);assert.equal(s.count,0);assert.equal((await rq(112,[4,4]))[2],0);await rq(112,[2]);assert.equal((await rq(112,[4,4]))[2],1);
 await rq(112,[5,80,0]);mock.sim.pedal(1,3,64,127,200);mock.sim.pedal(1,3,64,127,210);assert.equal(E.PERF.status(await rq(112)).result,2);mock.sim.pedal(1,3,64,0,211);mock.sim.pedal(1,3,64,127,212);assert.equal(E.PERF.status(await rq(112)).result,1);
 await rq(112,[1,5]);mock.sim.pedal(1,3,64,127);assert.equal((await rq(112,[2]))[0],2);await rq(112,[3]);
 assert.equal((await rq(113,[1,0]))[0],0);assert.equal((await rq(113))[1],1);assert.equal((await rq(113,[4,0])).length,57);assert.equal((await rq(113,[5,0,...E.pack7(new Uint8Array(48)),1]))[0],1);
 assert.equal((await rq(114,[1,3,0,0,0]))[0],0);assert.equal((await rq(114))[2],127);await rq(114,[2,0]);assert.equal((await rq(114))[2],0);await rq(114,[2,0]);assert.equal((await rq(114))[2],1);assert.equal((await rq(114,[1,1,8]))[0],1);assert.equal((await rq(114))[1],3);
 for(let e=0;e<23;e++){const a=await rq(115,[e]);assert.equal(a[0],0);assert.equal(a[1],e);}
 const f=E.PERF.filters(await rq(116));assert.equal(f.fields.length,7);assert.equal(f.fields[1].min,0);assert.equal(f.fields[1].max,127);
 assert.equal(E.bkProjFormat(Uint8Array.of(0x50,0x42,0x4b,0x32)),10);assert.equal(E.bkProjFormat(Uint8Array.of(0x50,0x52,0x47,0x33)),10);assert.equal(E.bkDevFormat({proto:21,version:'SynthMaven 1.9'}),9);assert.equal(E.bkDevFormat({proto:22,version:'SynthMaven 2.0'}),10);
 console.log('Performance protocol: staged learn/accept, conflicts, CC re-arm, scenes, ordered setlists, engine capabilities, descriptors and backup guards PASS');
}finally{link.close();mock.stop();}
