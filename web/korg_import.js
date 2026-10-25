/* SPDX-License-Identifier: GPL-3.0-only */
/* SynthMaven minilogue xd -> HYBRID translation. Parameter offsets follow
 * Korg MIDI Implementation TABLE 2 (also documented by oxur/minilogue-xd).
 * This is a parameter translator, not an ARM user-unit loader. */
const KORG = (() => {
  const clamp = (n, a, b) => Math.max(a, Math.min(b, Math.round(n)));
  const scale = n => clamp(n * 127 / 1023, 0, 127);
  function init(HY) {
    const p = HY.init();
    p.name = 'XD INIT';
    p.osc = [
      {type:1,coarse:0,fine:0,level:100,shape:0,reset:false,fixed:false},
      {type:2,coarse:0,fine:3,level:80,shape:0,reset:false,fixed:false},
      {type:10,coarse:0,fine:0,level:0,shape:0,reset:false,fixed:false}
    ];
    p.flt[0] = {type:1,cut:100,res:20,env:20,key:0,vel:0};
    p.flt[1].type = 0; p.ring=0; p.drive=0; p.route=0;
    p.env2=[0,60,0,20]; p.env3=[0,0,0,0];
    p.mod.forEach(x => {x.src=0;x.via=0;x.dst=0;x.amt=0;});
    return HY.sanitize(p);
  }
  function program(input, HY, options = {}) {
    const b = new Uint8Array(input);
    if(b.length!==1024 || String.fromCharCode(...b.slice(0,4))!=='PROG')
      throw new Error('Expected a 1024-byte minilogue xd PROG record.');
    const read = i => {const n=b[i]|b[i+1]<<8;if(n>1023)throw new Error(`Invalid 10-bit parameter at ${i}.`);return n;};
    const choice = (i,n) => {if(b[i]>=n)throw new Error(`Invalid selector at ${i}.`);return b[i];};
    const p=init(HY), warnings=[], name=new TextDecoder().decode(b.slice(4,16)).replace(/\0.*$/s,'').trim();
    p.name=name;
    for(let k=0;k<2;k++) {
      const i=22+6*k, pitch=(read(i+2)-512)*1200/512, semi=Math.round(pitch/100);
      p.osc[k]={type:[2,3,1][choice(i,3)],coarse:clamp((choice(i+1,4)-1)*12+semi,-48,48),
        fine:clamp(pitch-semi*100,-64,63),level:scale(read(54+2*k)),shape:scale(read(i+4)),reset:false,fixed:false};
    }
    const multi=choice(38,3), ml=read(58);
    p.osc[2].level=scale(ml); p.osc[2].coarse=(choice(130,4)-1)*12;
    if(multi===0) {p.osc[2].type=10;p.osc[2].shape=scale(read(42));
      if(b[39])warnings.push('Special noise subtype approximated with HYBRID noise.');}
    else if(options.substituteDigital && ml) {p.osc[2].type=multi===1?7:1;p.osc[2].shape=scale(read(multi===1?44:46));warnings.push(multi===1?'Korg VPM replaced by native HYBRID FM; approximate.':'Korg user oscillator replaced by native HYBRID saw; approximate.');}
    else {p.osc[2].type=0;p.osc[2].level=0;
      if(ml)warnings.push(multi===2 ? `User oscillator slot ${b[41]+1} is unavailable; digital oscillator muted. A .mnlgxdunit binary cannot run on FM1.` : 'Korg VPM oscillator is unavailable in this translation; digital oscillator muted.');}
    if(b[34]===0) {p.osc[1].type=8;warnings.push('Oscillator 2 sync uses HYBRID hard sync.');}
    if(b[35]===0) {p.ring=127;warnings.push('Ring modulation balance is approximated.');}
    if(read(36))warnings.push('Cross-modulation depth is not translated.');
    p.flt[0]={type:1,cut:scale(read(60)),res:scale(read(62)),env:0,
      key:[0,32,64][choice(65,3)],vel:clamp(b[128]*127/127,0,127)};
    p.drive=[0,40,80][choice(64,3)];
    const amp=[66,68,70,72].map(i=>scale(read(i)));
    p.env2=[scale(read(74)),scale(read(76)),0,0];
    const eg=clamp((read(78)-512)*63/512,-64,63), et=choice(80,3);
    if(et===0)p.flt[0].env=eg;
    else p.mod[0]={src:1,via:0,dst:et===1?3:1,amt:eg};
    const mode=choice(82,3);
    p.lfo[0]={wave:[3,1,2][choice(81,3)],rate:scale(read(83)),key:!!b[125],sync:false,fade:0};
    const target=choice(87,3), oscTarget=choice(127,4), amount=clamp(read(85)*63/1023,0,63);
    const destinations=target===0?[12]:target===1?(oscTarget===0?[5,6,7]:oscTarget===1?[5,6]:oscTarget===2?[6]:[7]):
      (oscTarget===0?[1]:oscTarget===1?[2,3]:oscTarget===2?[3]:[4]);
    destinations.forEach((dst,i)=>{p.mod[1+i]={src:4,via:0,dst,amt:amount};});
    if(mode!==1)warnings.push(mode===0?'One-shot LFO approximated with a repeating LFO.':'Tempo-synced LFO rate approximated with a free-running LFO.');
    if(b[88]||b[99]||b[105])warnings.push('Korg modulation/delay/reverb algorithms are not reproduced; choose SynthMaven effects separately.');
    if(b[17])warnings.push('Portamento is not translated.');
    if(b[21]!==4)warnings.push('Voice-mode depth and non-poly modes are not translated.');
    warnings.push('Filter, pitch/shape response and envelope/LFO time scales are approximate. Sequencer, motion lanes, tuning and controller assignments are not imported.');
    return {name,patch:HY.sanitize(p),amp,warnings,multi,kind:'program'};
  }
  async function archive(bytes, HY, unzip, options = {}) {
    const b=new Uint8Array(bytes);
    if(b.length>1048576 || b.length<22)throw new Error('Invalid or oversized Korg archive.');
    /* Bound the declared ZIP output before invoking the existing unzip helper. */
    const v=new DataView(b.buffer,b.byteOffset,b.byteLength);let e=b.length-22;
    while(e>=0 && v.getUint32(e,true)!==0x06054b50)e--;
    if(e<0)throw new Error('Not a Korg ZIP archive.');
    const count=v.getUint16(e+10,true);let at=v.getUint32(e+16,true),total=0;
    if(count>2048)throw new Error('Too many Korg archive entries.');
    for(let i=0;i<count;i++) {
      if(at+46>b.length || v.getUint32(at,true)!==0x02014b50)throw new Error('Invalid ZIP directory.');
      total+=v.getUint32(at+24,true);
      if(total>1048576)throw new Error('Korg archive expands beyond 1 MB.');
      at+=46+v.getUint16(at+28,true)+v.getUint16(at+30,true)+v.getUint16(at+32,true);
    }
    const files=await unzip(b), decode=x=>new TextDecoder().decode(x.data);
    const info=files.find(x=>x.name==='FileInformation.xml');
    if(info) {
      if(!/<Product>\s*minilogue xd\s*<\/Product>/.test(decode(info)))throw new Error('This is not a minilogue xd program.');
      const records=files.filter(x=>/\.prog_bin$/i.test(x.name));
      if(!records.length)throw new Error('No program records in Korg archive.');
      if(records.length>1) {
        const programs=records.map(x=>{if(x.data.length!==1024 || String.fromCharCode(...x.data.slice(0,4))!=='PROG')throw new Error('Invalid program record in Korg library.');return {name:new TextDecoder().decode(x.data.slice(4,16)).replace(/\0.*$/s,'').trim(),entry:x.name,data:x.data};});
        return {kind:'library',name:`${programs.length} programs`,programs};
      }
      return program(records[0].data,HY,options);
    }
    const manifest=files.find(x=>/(^|\/)manifest\.json$/.test(x.name));
    if(manifest) {
      const h=JSON.parse(decode(manifest)).header;
      if(!h || h.platform!=='minilogue-xd' || !files.some(x=>/(^|\/)payload\.bin$/.test(x.name)))throw new Error('Invalid minilogue xd unit.');
      return {kind:'unit',name:String(h.name||'Unnamed'),warnings:[`${String(h.name||'This unit')} is compiled Korg ${String(h.module||'user')} code. FM1 cannot execute its ARM binary. Source code is needed for a native port.`],params:Array.isArray(h.params)?h.params.map(x=>String(x[0])):[]};
    }
    throw new Error('No minilogue xd program or user-unit manifest found.');
  }
  return {init,program,archive};
})();
