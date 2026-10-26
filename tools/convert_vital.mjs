#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-only
// Stage user-supplied Vital translations for listening review, never publish factory sounds automatically.
import fs from 'node:fs';import vm from 'node:vm';import path from 'node:path';import {createHash} from 'node:crypto';
const args=process.argv.slice(2),approximate=args.includes('--approximate');const inputs=args.filter(x=>x!=='--approximate');if(!inputs.length){console.error('Usage: node tools/convert_vital.mjs [--approximate] file.vital [...] > translations.json');process.exit(1);}
const root=path.resolve(import.meta.dirname,'..'),html=fs.readFileSync(path.join(root,'web/editor.html'),'utf8'),start=html.indexOf('const HY = (() => {'),end=html.indexOf('\n})();',start)+6,source=fs.readFileSync(path.join(root,'web/vital_import.js'),'utf8');
const decode=s=>new Uint8Array(Buffer.from(s,'base64'));const ctx=vm.createContext({Uint8Array,Float32Array,DataView,KORG_STOCK:{items:[]},b64dec:decode});vm.runInContext(html.slice(start,end)+'\n'+source+'\nthis.api={HY,VITAL};',ctx);const {HY,VITAL}=ctx.api;
const staged=[];for(const filename of inputs){const bytes=fs.readFileSync(filename);if(bytes.length>VITAL.MAX_BYTES)throw Error('File exceeds 16 MB: '+filename);const j=VITAL.read(bytes.toString('utf8')),r=VITAL.convert(j,HY,{approximate,decode});staged.push({source:path.basename(filename),sha256:createHash('sha256').update(bytes).digest('hex'),name:r.name,author:r.author,sourceVersion:r.sourceVersion,canSend:r.canSend,patch:r.patch,amp:r.amp,packed:Array.from(HY.pack(r.patch)),mapped:r.mapped,warnings:r.warnings,wavetableCount:r.tables.length,review:'pending-listening-and-level-matching'});}
console.log(JSON.stringify({format:'synthmaven-vital-staging',version:1,items:staged},null,2));
