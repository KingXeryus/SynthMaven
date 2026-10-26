// SPDX-License-Identifier: GPL-3.0-only
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
const html=readFileSync(new URL('./editor.html',import.meta.url),'utf8');
const source=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(source+';({makeMockDevice,Link,req,parse,CMD,Reader,ORIENTAL_STOCK})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder,Blob,Response,DecompressionStream});
const m=E.makeMockDevice({synthmaven:true,oriental:true,auto:false});const input=[...m.access.inputs.values()][0],output=[...m.access.outputs.values()][0];
const link=new E.Link(d=>output.send(d),{timeout:500});input.onmidimessage=e=>link.receive(e.data);
const rq=r=>link.request(r);
try{
 const info=E.parse[E.CMD.INFO](await rq(E.req.info()));assert.equal(info.proto,16);
 for(const engine of new Set(E.ORIENTAL_STOCK.map(x=>x.engine))){const names=E.parse[E.CMD.NAMES](await rq(E.req.names(engine)));for(const x of E.ORIENTAL_STOCK.filter(x=>x.engine===engine))assert.equal(names.names[x.preset],x.shortName);}
 for(const x of E.ORIENTAL_STOCK){const loaded=E.parse[E.CMD.PRESET](await rq(E.req.preset(x.engine,x.preset)));assert.equal(loaded.preset,x.preset);const d=E.parse[E.CMD.DUMP](await rq(E.req.dump()),info);assert.equal(d.preset,x.preset);assert.deepEqual(Array.from(d.p),Array.from(x.values));}
 assert.equal(new E.Reader([126]).preset(),126);assert.equal(new E.Reader([127,0]).preset(),127);assert.equal(new E.Reader([127,12]).preset(),139);assert.throws(()=>new E.Reader([127]).preset(),/short/);
 console.log('Oriental editor: all 44 names and full patches; escaped high indices/counts; MIDI-safe framing PASS');
}finally{link.close();m.stop();}
