import fs from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
const html=fs.readFileSync('web/editor.html','utf8'),source=fs.readFileSync('web/korg_import.js','utf8');
assert(html.includes(source),'embedded Korg translator matches source');
const start=html.indexOf('const HY = (() => {'),end=html.indexOf('\n})();',start)+6;
const ctx=vm.createContext({TextDecoder,Uint8Array,Blob,Response,DecompressionStream,b64dec:s=>new Uint8Array(Buffer.from(s,'base64')),KORG_STOCK:{items:[]}});
const zs=html.indexOf('async function zipRead('),ze=html.indexOf('\n}\n',zs)+2;
vm.runInContext(html.slice(start,end)+'\n'+source+'\n'+html.slice(zs,ze)+'\nthis.api={KORG,HY,zipRead};',ctx);
const {KORG,HY,zipRead}=ctx.api;
const inputs=JSON.parse(fs.readFileSync('assets/korg/sources.json'));
for(const x of inputs) {
 const bytes=new Uint8Array(fs.readFileSync('assets/korg/'+x.record)),before=bytes.slice();
 const p=KORG.program(bytes,HY),a=KORG.program(bytes,HY,{substituteDigital:true});
 assert.equal(HY.pack(a.patch).length,128);assert.deepEqual(bytes,before);
 assert(p.warnings.length);assert(a.patch.osc.some(o=>o.type&&o.level),'stock approximation produces an active oscillator');
 if(p.multi!==0 && ((bytes[58]|bytes[59]<<8)>0))assert.equal(p.patch.osc[2].type,0,'unsupported digital osc stays muted by default');
}
const stock=JSON.parse(fs.readFileSync('assets/korg/translated.json')).items;
assert.equal(stock.length,9);assert.equal(stock[2].originalName,'Dark Chord');assert.equal(stock[2].entry,'Prog_253.prog_bin');
assert.equal(KORG.init(HY).name,'XD INIT');
assert.throws(()=>KORG.program(new Uint8Array(1024),HY),/PROG/);
const bad=new Uint8Array(fs.readFileSync('assets/korg/'+inputs[0].record));bad[24]=255;bad[25]=255;
assert.throws(()=>KORG.program(bad,HY),/10-bit/);
const archive=files=>new Uint8Array(execFileSync('python3',['-c',`import io,sys,json,zipfile,base64
files=json.load(sys.stdin);b=io.BytesIO()
with zipfile.ZipFile(b,'w',compression=zipfile.ZIP_DEFLATED) as z:
 for name,data in files:z.writestr(name,base64.b64decode(data))
sys.stdout.buffer.write(b.getvalue())`],{input:JSON.stringify(files.map(([n,d])=>[n,Buffer.from(d).toString('base64')]))}));
const b=fs.readFileSync('assets/korg/'+inputs[0].record);
const pack=archive([['FileInformation.xml','<Product>minilogue xd</Product>'],['Prog_000.prog_bin',b]]);
assert.equal((await KORG.archive(pack,HY,zipRead)).name,'80s Bass');
const unit=archive([['osc/manifest.json',JSON.stringify({header:{platform:'minilogue-xd',module:'osc',name:'THMoog',params:[['Detune',0,100,'%']]}})],['osc/payload.bin',new Uint8Array([1,2,3])]]);
const u=await KORG.archive(unit,HY,zipRead);assert.equal(u.kind,'unit');assert.equal(u.params[0],'Detune');assert(!u.patch);
await assert.rejects(()=>KORG.archive(archive([['FileInformation.xml','<Product>minilogue</Product>'],['p.prog_bin',b]]),HY,zipRead),/not a minilogue xd/);
for(const path of process.argv.slice(2)) {const r=await KORG.archive(new Uint8Array(fs.readFileSync(path)),HY,zipRead);console.log(path.split('/').pop(),r.kind,r.name);}
console.log('Korg: nine defaults, native substitutions, safe missing-unit handling, archive and parameter validation PASS');
