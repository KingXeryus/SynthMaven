// Runs inside the editor script: reuse its descriptors, history and coalesced SET queue.
window.SynthMavenPerform = {
  controls() {
    if (!dev?.dump || !dev.pdesc) return [];
    const pe0=dev.info.pe0, engine=dev.dump.engine;
    const cutoff=SM_FILTER_MAP.find(x=>x.engine===engine)?.cutoff;
    const tone=/^P_E\d+$/.test(cutoff||'') ? pe0+Number(cutoff.slice(3)) : -1;
    const macros=Array.from({length:4},(_,i)=>[dev.info.engines[engine]==='HYBRID'?pe0+4+i:96+i,`Macro ${i+1}`]);
    return [...macros,[tone,'Tone'],[33,'Distortion'],[35,'Delay'],[36,'Reverb'],[9,'LFO rate'],[14,'LFO → filter'],[1,'ENV attack'],[4,'ENV release']].map(([id,label])=>{
      const d=id>=0?dev.pdesc[id]:null, value=d?dev.dump.p[id]:0;
      return {id,label,min:d?.min??0,max:d?.max??127,value,enabled:!!d&&d.min!==d.max,text:d?fmtValue(d,value).filter(Boolean).join(' '):'Unavailable',context:`${engine}:${dev.dump.sel??''}`};
    });
  },
  set(id,value,final=true) {
    const c=this.controls().find(c=>c.id===id);
    if (!c?.enabled || !Number.isFinite(value)) return;
    userSet(0,id,Math.max(c.min,Math.min(c.max,Math.round(value))),final);
  }
};

window.SynthMavenMacroAssignments = {
  snapshot() {
    if(!dev?.dump || dev.info.pe0<118 || isDrum())return null;
    const hybrid=hyOn(), rows=[];
    for(let macro=0;macro<4;macro++){
      const source=(hybrid?14:16)+macro;
      const routes=hybrid?hyv.mod.map((m,slot)=>({...m,slot})).filter(m=>m.src===source):Array.from({length:12},(_,slot)=>{const id=mxId(slot+1);return {slot,src:dev.dump.p[id],dst:dev.dump.p[id+1],amt:dev.dump.p[id+2],via:0};}).filter(m=>m.src===source);
      const d=synDesc(61,dev.pdesc[61]);
      const destinations=hybrid?HY.DST:d.names;
      rows.push({macro,routes,destinations:destinations.map((name,id)=>({id,name})).filter(x=>x.id>0&&x.name&&!/^[-—]+$/.test(x.name))});
    }
    return {hybrid,rows,context:`${dev.dump.engine}:${hyTrack()}`,note:hybrid?'Assignments control the HYBRID patch’s modulation matrix.':'Assignments add to the native macro behavior: brightness, timbre, motion and space.'};
  },
  async assign(macro,slot,dst,amt,remove=false){
    if(!Number.isInteger(macro)||macro<0||macro>3)throw Error('Invalid macro');
    const state=this.snapshot();if(!state)throw Error('Connect a compatible sound first.');
    if(!remove&&!state.rows[macro].destinations.some(x=>x.id===dst))throw Error('Choose a destination.');
    amt=Math.max(-64,Math.min(63,Math.round(amt)));
    const source=(state.hybrid?14:16)+macro;
    if(state.hybrid){
      await hyReadTrack(true);
      if(slot<0)slot=hyv.mod.findIndex(m=>!m.src||!m.dst);
      if(slot<0)throw Error('The modulation matrix is full. Remove an unused route first.');
      const m=hyv.mod[slot];if(!m|| (m.src&&m.dst&&m.src!==source))throw Error('Assignment changed. Refresh and try again.');
      if(remove)hyv.mod[slot]={src:0,via:0,dst:0,amt:0};
      else hyv.mod[slot]={...m,src:source,dst,amt};
      await hySend(true);renderHy();
    }else{
      if(slot<0)slot=Array.from({length:12},(_,k)=>k).find(k=>!dev.dump.p[mxId(k+1)]||!dev.dump.p[mxId(k+1)+1])??-1;
      if(slot<0)throw Error('The modulation matrix is full. Remove an unused route first.');
      const id=mxId(slot+1);if(slot>11||(dev.dump.p[id]&&dev.dump.p[id+1]&&dev.dump.p[id]!==source))throw Error('Assignment changed. Refresh and try again.');
      // Disable the route while changing it so no intermediate destination sounds.
      for(const [pid,value] of [[id,0],[id+1,remove?0:dst],[id+2,remove?0:amt],[id,remove?0:source]]){
        const r=parse[CMD.SET](await rq(req.set(0,pid,value)));dev.dump.p[pid]=r.value;showValue(key(0,pid),r.value,false);
      }
    }
  }
};

window.SynthMavenPerformanceNavigation = {
  async read(action){
    if(!dev?.dump||dev.info.proto<25)return null;
    const r=await rq([120,action==null?[]:[action]]);
    if(r.length<7)throw Error('Update the FM-1 to firmware 3.0 for performance navigation.');
    if(r[0])throw Error(r[0]===2?'A program is already loading.':'No saved program or scene is available.');
    const end=r.indexOf(0,6),name=String.fromCharCode(...r.slice(6,end<0?r.length:end));
    return {program:r[1],scene:r[2],pending:r[3],result:r[4],mask:r[5],name};
  },
  async step(action){
    let state=await this.read(action);if(!state)throw Error('Connect firmware 3.0 first.');
    const deadline=Date.now()+7000;
    while(state.pending!==127){if(Date.now()>deadline)throw Error('Program load timed out.');await new Promise(r=>setTimeout(r,150));state=await this.read();}
    if((action===4||action===5)&&state.result)throw Error('Program could not be loaded.');
    await afterSoundChange();return state;
  }
};
