/* SPDX-License-Identifier: GPL-3.0-only */
/* Vital JSON -> HYBRID approximation. Format/units verified against mtytel/vital
 * src/common/load_save.cpp, synth_parameters.cpp, wavetable/wave_source.cpp.
 * No Vital DSP, factory sounds or proprietary preset content is bundled. */
const VITAL = (() => {
  const MAX_BYTES=16*1024*1024, clamp=(x,a,b)=>Math.max(a,Math.min(b,Math.round(x)));
  function read(text) {
    if(typeof text!=='string'||text.length>MAX_BYTES)throw Error('Vital file exceeds 16 MB.');
    const j=JSON.parse(text);
    if(!j||Array.isArray(j)||!j.settings||typeof j.settings!=='object'||Array.isArray(j.settings))throw Error('Not a Vital preset: settings object missing.');
    if(!Object.keys(j.settings).some(k=>/^(osc_[123]_|env_[123]_)/.test(k)))throw Error('No Vital oscillator/envelope controls found.');
    for(const [k,v] of Object.entries(j.settings))if(typeof v==='number'&&!Number.isFinite(v))throw Error('Non-finite control: '+k);
    return j;
  }
  function frames(table,decode) {
    if(!table||!Array.isArray(table.groups)||table.groups.length!==1)throw Error('Only a single unmodified Wave Source group can be extracted.');
    const c=table.groups[0].components;
    if(!Array.isArray(c)||c.length!==1||!Array.isArray(c[0].keyframes)||!c[0].keyframes.length||c[0].keyframes.length>256)throw Error('Wavetable generators/modifiers are not supported for extraction.');
    return c[0].keyframes.map(k=>{
      if(typeof k.wave_data!=='string'||k.wave_data.length>12000||!Number.isFinite(k.position)||k.position<0||k.position>256)throw Error('Invalid Wave Source keyframe.');
      const bytes=decode(k.wave_data);if(bytes.length!==8192)throw Error('Only 2048-point float Wave Source frames are supported.');
      const view=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength),samples=new Float32Array(2048);
      for(let i=0;i<2048;i++){let v=view.getFloat32(i*4,true);if(!Number.isFinite(v)||Math.abs(v)>64)throw Error('Invalid wavetable sample.');samples[i]=v;}
      return {position:k.position,samples};
    }).sort((a,b)=>a.position-b.position);
  }
  function identify(samples) {
    // Match harmonics' magnitudes, independent of starting phase and amplitude.
    const h=[];for(let k=1;k<=12;k++){let re=0,im=0;for(let i=0;i<2048;i++){const ph=2*Math.PI*k*i/2048;re+=samples[i]*Math.cos(ph);im+=samples[i]*Math.sin(ph);}h.push(Math.hypot(re,im));}
    if(h[0]<1e-5)return 0;
    const templates=[{type:4,h:k=>k===1?1:0},{type:1,h:k=>1/k},{type:2,h:k=>k%2?1/k:0},{type:3,h:k=>k%2?1/(k*k):0}];
    let best={type:0,error:Infinity};for(const t of templates){let err=0;for(let k=1;k<=12;k++)err+=(h[k-1]/h[0]-t.h(k))**2;if(err<best.error)best={type:t.type,error:err};}
    return best.error<0.02?best.type:0;
  }
  const time=v=>clamp(Math.log(Math.max(1,1000*v**4))/Math.log(10000)*127,0,127);
  function convert(j,HY,{approximate=false,decode}={}) {
    if(!j||!j.settings)throw Error('Read a Vital preset first.');
    const s=j.settings,n=(k,d=0)=>{const v=s[k];if(v===undefined)return d;if(typeof v!=='number'||!Number.isFinite(v))throw Error('Invalid numeric control: '+k);return v;};
    const warnings=[],mapped=[],tables=[],p=HY.init();p.name=String(j.preset_name||j.name||'VITAL IMPORT').slice(0,12);p.level=90;p.drive=0;p.ring=0;p.route=0;p.fbal=64;p.mod=Array.from({length:16},()=>({src:0,via:0,dst:0,amt:0}));
    let missing=0;
    for(let i=0;i<3;i++) {
      const prefix=`osc_${i+1}_`,on=n(prefix+'on',i===0?1:0),level=n(prefix+'level',Math.SQRT1_2),table=(s.wavetables||[])[i];let type=0;
      if(on&&level>0){
        if(!table)warnings.push(`Oscillator ${i+1}: missing wavetable definition.`);
        if(table&&decode)try{const f=frames(table,decode);tables.push({oscillator:i+1,name:String(table.name||'Wavetable').slice(0,64),frames:f});const at=n(prefix+'wave_frame');const chosen=f.reduce((a,b)=>Math.abs(a.position-at)<=Math.abs(b.position-at)?a:b);if(Math.abs(chosen.position-at)<0.51)type=identify(chosen.samples);}catch(e){warnings.push(`Oscillator ${i+1}: ${e.message}`);}
        if(!type){if(approximate){type=1;warnings.push(`Oscillator ${i+1}: custom/missing wavetable replaced with a native saw.`);}else{missing++;warnings.push(`Oscillator ${i+1}: no matching native waveform. Enable explicit approximation or extract its wavetable for WAVE.`);}}
        if(n(prefix+'unison_voices',1)>1)warnings.push(`Oscillator ${i+1}: unison voices/stereo spread are not reproduced.`);
        if(n(prefix+'spectral_morph_type')||n(prefix+'distortion_type'))warnings.push(`Oscillator ${i+1}: spectral/phase warping is not reproduced.`);
        if(n(prefix+'pan')||n(prefix+'destination'))warnings.push(`Oscillator ${i+1}: per-oscillator pan/output routing is not reproduced.`);
        if(n(prefix+'tune')<-.64||n(prefix+'tune')>.63)warnings.push(`Oscillator ${i+1}: fine tuning limited to -64..63 cents.`);
        mapped.push(`Oscillator ${i+1}: ${type?HY.OSC[type]:'unmapped'}, level and pitch.`);
      }
      p.osc[i]={type:on?type:0,level:on?clamp(level*127,0,127):0,coarse:clamp(n(prefix+'transpose'),-48,48),fine:clamp(n(prefix+'tune')*100,-64,63),shape:type===2?64:0,reset:false,fixed:false};
    }
    const env=i=>[time(n(`env_${i}_attack`,.1495)),time(n(`env_${i}_decay`,1)),clamp(n(`env_${i}_sustain`,1)*127,0,127),time(n(`env_${i}_release`,.5476))];
    const amp=env(1);p.env2=env(2);p.env3=env(3);mapped.push('Amp envelope and envelopes 2/3 (quartic Vital times converted approximately).');
    for(let i=0;i<2;i++){
      const pre=`filter_${i+1}_`;let type=0;
      if(n(pre+'on')){const blend=n(pre+'blend');type=blend<.5?1:blend<1.5?3:4;if(n(pre+'model')||n(pre+'style'))warnings.push(`Filter ${i+1}: model/style replaced with a native LP/BP/HP filter.`);mapped.push(`Filter ${i+1}: cutoff, resonance, key tracking.`);}
      const hz=440*2**((n(pre+'cutoff',60)-69)/12);p.flt[i]={type,cut:clamp(Math.log(hz/30)/Math.log(16000/30)*127,0,127),res:clamp(n(pre+'resonance',.5)*127,0,127),env:0,key:clamp(n(pre+'keytrack')*63,-64,63),vel:0};
      if(n(pre+'drive'))warnings.push(`Filter ${i+1}: drive omitted.`);
    }
    if(n('filter_1_on')||n('filter_2_on'))warnings.push('HYBRID filters use a serial approximation; Vital input routing and filter response differ.');
    for(let i=0;i<2;i++){const pre=`lfo_${i+1}_`,hz=2**n(pre+'frequency',1);p.lfo[i]={wave:0,rate:clamp(Math.log(hz/.05)/Math.log(800)*127,0,127),key:true,sync:false,fade:0};}
    const sources={env_1:3,env_2:1,env_3:2,lfo_1:4,lfo_2:5,velocity:7,note:8,mod_wheel:9,aftertouch:10,pitch_wheel:11,macro_control_1:14,macro_control_2:15,macro_control_3:16,macro_control_4:17};
    const destinations={osc_1_transpose:2,osc_2_transpose:3,osc_3_transpose:4,osc_1_level:8,osc_2_level:9,osc_3_level:10,filter_1_cutoff:13,filter_2_cutoff:14,filter_1_resonance:15,filter_2_resonance:16,volume:18};
    let slot=0,dropped=0;const mods=s.modulations||[];if(!Array.isArray(mods)||mods.length>256)throw Error('Invalid modulation list.');
    mods.forEach((m,i)=>{if(!m||!m.source||!m.destination)return;const amount=n(`modulation_${i+1}_amount`);if(!amount)return;const src=sources[m.source],dst=destinations[m.destination];if(!src||!dst||slot>=16||m.line_mapping||n(`modulation_${i+1}_power`)||n(`modulation_${i+1}_stereo`)||n(`modulation_${i+1}_bipolar`)){dropped++;return;}p.mod[slot++]={src,via:0,dst,amt:clamp(amount*63,-64,63)};});
    if(slot)mapped.push(`${slot} modulation routes (depths approximate).`);if(dropped)warnings.push(`${dropped} unsupported/nonlinear modulation routes omitted.`);
    if(mods.some(m=>m&&/^lfo_/.test(m.source)))warnings.push('Custom/synced LFO shapes are approximated by free-running sine LFOs; LFO3/4 routes are omitted.');
    for(const fx of ['chorus','delay','reverb','distortion','phaser','flanger','compressor','eq'])if(n(fx+'_on'))warnings.push(`${fx}: Vital effect omitted; choose SynthMaven effects separately.`);
    if(n('sample_on'))warnings.push('Sample oscillator is not imported.');
    warnings.push('Approximate translation, not Vital playback. Master volume, macros, tuning, voice modes, portamento and unsupported controls are not imported.');
    return {format:'synthmaven-vital-translation',version:1,name:String(j.preset_name||j.name||'VITAL IMPORT').slice(0,64),author:String(j.author||'').slice(0,128),sourceVersion:String(j.synth_version||''),patch:HY.sanitize(p),amp,mapped,warnings,tables,canSend:missing===0&&p.osc.some(o=>o.type&&o.level)};
  }
  function wave(table){
    // Export the actual stored keyframes; no claim to render Vital modifiers/interpolation.
    const fs=table.frames;if(!fs||!fs.length)throw Error('No extractable frames.');const b=new Uint8Array(44+fs.length*2048*2),v=new DataView(b.buffer),text=(at,s)=>[...s].forEach((c,i)=>v.setUint8(at+i,c.charCodeAt(0)));
    text(0,'RIFF');v.setUint32(4,b.length-8,true);text(8,'WAVE');text(12,'fmt ');v.setUint32(16,16,true);v.setUint16(20,1,true);v.setUint16(22,1,true);v.setUint32(24,44100,true);v.setUint32(28,88200,true);v.setUint16(32,2,true);v.setUint16(34,16,true);text(36,'data');v.setUint32(40,b.length-44,true);
    let peak=1;for(const f of fs)for(const x of f.samples)peak=Math.max(peak,Math.abs(x));fs.forEach((f,k)=>f.samples.forEach((x,i)=>v.setInt16(44+2*(k*2048+i),clamp(x/peak*32767,-32768,32767),true)));return b;
  }
  return {read,convert,frames,wave,identify,time,MAX_BYTES};
})();
