// SPDX-License-Identifier: GPL-3.0-only
(() => {
  'use strict';
  if (window.top !== window || !window.webkit?.messageHandlers?.midi) return;
  const native = body => window.webkit.messageHandlers.midi.postMessage(body);
  const access = {inputs:new Map(), outputs:new Map(), sysexEnabled:true, onstatechange:null};
  let input, output;
  function disconnected(message) {
    for (const port of [input,output]) if (port) {
      port.state='disconnected'; port.connection='closed';
      access.onstatechange?.({port});
    }
    access.inputs.clear();access.outputs.clear();input=output=null;
    if(message) window.dispatchEvent(new CustomEvent('synthmaven-native-error',{detail:message}));
  }
  function port(type) {
    return {id:'native-'+type, manufacturer:'SynthMaven',name:'SynthMaven FM-1',type,version:'1',state:'connected',connection:'open',onmidimessage:null,
      async open(){return this;}, async close(){this.connection='closed';return this;}};
  }
  window.SynthMavenNative = {
    state({connected,session}) {
      if (!connected) { disconnected();return; }
      if (input && input.session===session) return;
      if (input) disconnected();
      input=port('input');output=port('output');input.session=output.session=session;
      const selected=output;
      output.send=(data,timestamp) => {
        if(selected!==output || selected.state!=='connected')throw new Error('MIDI disconnected');
        if(timestamp && timestamp>performance.now()+10)throw new Error('Scheduled MIDI is not supported');
        const bytes=Array.from(data);
        native({action:'send',bytes,session}).catch(e=>disconnected(String(e)));
      };
      output.clear=()=>native({action:'disconnect'});
      access.inputs.set(input.id,input);access.outputs.set(output.id,output);
      access.onstatechange?.({port:input});access.onstatechange?.({port:output});
    },
    connect(){window.SynthMavenConnect?.();},
    receive(bytes){input?.onmidimessage?.({data:Uint8Array.from(bytes),receivedTime:performance.now(),target:input});},
    error(text){disconnected(text);}
  };
  Object.defineProperty(navigator,'requestMIDIAccess',{value:async()=>{window.SynthMavenNative.state(await native({action:'status'}));return access;}});
  // Keep native export in the same file workflows as the existing editor.
  document.addEventListener('click',async event=>{
    if(event.target.closest?.('#connect') && !input){event.preventDefault();event.stopImmediatePropagation();await native({action:'connect'});return;}
    const a=event.target.closest?.('a[download]');
    if(!a || !a.href.startsWith('blob:'))return;
    event.preventDefault();
    try {
      const blob=await (await fetch(a.href)).blob();
      if(blob.size>16*1024*1024)throw new Error('Export exceeds 16 MB');
      const data=await new Promise((resolve,reject)=>{const r=new FileReader();r.onload=()=>resolve(String(r.result).split(',')[1]);r.onerror=reject;r.readAsDataURL(blob);});
      await native({action:'export',name:a.download||'SynthMaven-export',data});
    }catch(e){await native({action:'error',message:String(e)});}
  },true);
})();
