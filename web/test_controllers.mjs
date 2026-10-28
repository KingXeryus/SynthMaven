// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const html=fs.readFileSync('web/editor.html','utf8');
const src=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(src+';({makeMockDevice,Link})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder,Blob,Response,DecompressionStream});
for(const controller of [undefined,{name:'Launchkey 49 MK4',family:'NOVATION LAUNCHKEY',vid:0x1235,pid:42,mode:2,daw:1}]){
 const m=E.makeMockDevice({synthmaven:true,modern:true,oriental:true,auto:false,controller});const input=[...m.access.inputs.values()][0],out=[...m.access.outputs.values()][0],link=new E.Link(d=>out.send(d));input.onmidimessage=e=>link.receive(e.data);const rq=p=>link.request([117,p]);
 try{let a=await rq([0]);assert.equal(a[0],0);assert.equal(a[1],+!!controller);assert.equal(a[2],0);assert.equal(a[6]+128*a[7]+16384*a[8],controller?.vid||0);
 for(let mode=0;mode<4;mode++){assert.equal((await rq([1,mode,16]))[0],0);a=await rq([]);assert.equal(a[2],mode);assert.equal(a[3],16);}
 assert.equal((await rq([1,4,0]))[0],1);assert.equal((await rq([1,0,17]))[0],1);a=await rq([0]);assert.equal(a[2],3);assert.equal(a[3],16);
 assert(html.includes('USB controller auto-detection')&&html.includes('return to COMPUTER mode'));
 }finally{link.close();m.stop();}
}
console.log('Controller editor: automatic identity, override roundtrip, atomic invalid writes and role guidance PASS');
