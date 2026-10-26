// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const html=fs.readFileSync('web/editor.html','utf8');const src=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(src+';({makeMockDevice,Link,req,parse,CMD,ORIENTAL_LAYOUT})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder,Blob,Response,DecompressionStream});
const m=E.makeMockDevice({synthmaven:true,modern:true,oriental:true,auto:false}),input=[...m.access.inputs.values()][0],output=[...m.access.outputs.values()][0],link=new E.Link(d=>output.send(d));input.onmidimessage=e=>link.receive(e.data);const rq=x=>link.request(x);
try{const info=E.parse[E.CMD.INFO](await rq(E.req.info()));assert.match(info.version,/2\.1/);
for(const [label,names] of [['CHORD',['OFF','TRIAD','7TH','9TH','SUS4','POWER','MAJ','MIN','DOM7','MAJ7','MIN7','SUS2','DIM','AUG','AUTO','AUTO2']],['VOIC',['CLOSE','SMOOTH','OPEN','INV1','INV2','+OCT']]]){
const id=E.ORIENTAL_LAYOUT.track.findIndex(x=>x.label===label);assert.ok(id>=0);const d=E.parse[E.CMD.DESC](await rq(E.req.desc(0,id)));assert.deepEqual(Array.from(d.names),names);await rq(E.req.set(0,id,names.length-1));const dump=E.parse[E.CMD.DUMP](await rq(E.req.dump()),info);assert.equal(dump.p[id],names.length-1);
}
}finally{link.close();m.stop();}console.log('2.1 chord controls: 16 modes, six voicings and high-value MIDI settings PASS');
