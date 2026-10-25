// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';
import vm from 'node:vm';
const html=fs.readFileSync('web/editor.html','utf8');
const begin=html.indexOf('const HY = (() => {'), end=html.indexOf('\n})();',begin)+6;
const context=vm.createContext({TextDecoder,Uint8Array,b64dec:s=>new Uint8Array(Buffer.from(s,'base64')),KORG_STOCK:{items:[]}});
vm.runInContext(html.slice(begin,end)+'\n'+fs.readFileSync('web/korg_import.js','utf8')+'\nthis.api={HY,KORG};',context);
const {HY,KORG}=context.api;
const items=JSON.parse(fs.readFileSync('assets/korg/sources.json')).map((meta,i)=> {
 const r=KORG.program(new Uint8Array(fs.readFileSync('assets/korg/'+meta.record)),HY,{substituteDigital:true});
 r.patch.name=meta.name;
 return {...meta,originalName:r.name,patch:r.patch,packed:Array.from(HY.pack(r.patch)),amp:r.amp,warnings:r.warnings,index:27+i};
});
function patch(p) {
 const nums=a=>'{'+a.join(',')+'}';
 const osc=p.osc.map(o=>nums([o.type,o.coarse,o.fine,o.level,o.shape,(o.reset?1:0)|(o.fixed?2:0)]));
 const flt=p.flt.map(o=>nums([o.type,o.cut,o.res,o.env,o.key,o.vel]));
 const lfo=p.lfo.map(o=>nums([o.wave,o.rate,(o.key?1:0)|(o.sync?2:0),o.fade]));
 const mod=p.mod.map(o=>nums([o.src,o.via,o.dst,o.amt]));
 return `{.osc={${osc}},.ring=${p.ring},.drift=${p.drift},.drive=${p.drive},.route=${p.route},.fbal=${p.fbal},.flt={${flt}},.env2=${nums(p.env2)},.env3=${nums(p.env3)},.lfo={${lfo}},.mod={${mod}},.bend=${p.bend},.level=${p.level},.name=${JSON.stringify(p.name)}}`;
}
const macro=(name,rows)=>'#define '+name+' \\\n'+rows.map((r,i)=>r+(i<rows.length-1?' \\':'')).join('\n')+'\n';
let header='/* Generated from supplied Korg programs by tools/gen_korg_stock.mjs. Approximate HYBRID translations. */\n#pragma once\n';
header+=macro('SM_KORG_PATCHES',items.map(x=>patch(x.patch)+','));
header+=macro('SM_KORG_NAMES',items.map(x=>JSON.stringify(x.name)+','));
header+=macro('SM_KORG_PRESETS',items.map(x=>`HYP(${JSON.stringify(x.name)},${x.index},0,0,${x.amp.join(',')},0,FX(0,0,0,0)),`));
header+=macro('SM_KORG_BANK',items.map(x=>`{BK_XD,ENGI_HYBRID,${JSON.stringify(x.name)}},`));
fs.writeFileSync('firmware/src/korg_factory.h',header);
fs.writeFileSync('assets/korg/translated.json',JSON.stringify({items},null,2)+'\n');
const block='/* KORG_STOCK_BEGIN */\nconst KORG_STOCK = '+JSON.stringify({items})+';\n/* KORG_STOCK_END */';
let result=html;
if(result.includes('/* KORG_STOCK_BEGIN */'))result=result.replace(/\/\* KORG_STOCK_BEGIN \*\/[\s\S]*?\/\* KORG_STOCK_END \*\//,()=>block);
else result=result.replace('/* KORG_IMPORT_BEGIN */',block+'\n/* KORG_IMPORT_BEGIN */');
fs.writeFileSync('web/editor.html',result);
console.log('Korg defaults:',items.length,'HYBRID translations; original 27 factory IDs retained');
