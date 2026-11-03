// SPDX-License-Identifier: GPL-3.0-only
// The connected editor (mock device): a Prophet's ENV edits its own envelopes, the Layers strips shift a layer's
// octave (TRN), and Device sounds filters as the Library does.
import {readFileSync} from 'node:fs';import assert from 'node:assert/strict';import {JSDOM,VirtualConsole} from 'jsdom';
let html=readFileSync(new URL('editor.html',import.meta.url),'utf8');
html=html.replace(/<script src="([^"?]+)[^"]*"><\/script>/g,(m,f)=>'<script>'+readFileSync(new URL(f,import.meta.url),'utf8')+'</script>');
html=html.replace('const QS = new URLSearchParams','window.envTest={get dev(){return dev;},get mock(){return mock;},rq:(r,o)=>rq(r,o),req:()=>req,parse:()=>parse,CMD:()=>CMD,renderPreferences:(f)=>renderPreferences(f)};const QS = new URLSearchParams');
const dom=new JSDOM(html,{runScripts:'dangerously',pretendToBeVisual:true,virtualConsole:new VirtualConsole(),url:'http://localhost/?mock=1&mockauto=0',
  beforeParse(w){w.ResizeObserver=class{observe(){}disconnect(){}};w.matchMedia=()=>({matches:false,addEventListener(){},removeEventListener(){}});}});
const w=dom.window,T=w.envTest,$=id=>w.document.getElementById(id),sleep=ms=>new Promise(r=>setTimeout(r,ms));
const until=async(f,what)=>{for(let i=0;i<1500;i++){if(f())return;await sleep(20);}throw Error('timed out: '+what);};
$('connect').click();
await until(()=>T.dev?.dump&&T.dev.mix,'connected');
const d=T.dev,req=T.req(),parse=T.parse(),CMD=T.CMD();
const p5=async()=>parse[CMD.PROPHET](await T.rq(req.prophetGet(d.sel||0))).raw;
assert.equal(d.info.engines[d.dump.engine],'PROPHET','the mock powers on with a Prophet on part 1');

/* Quick controls: ENV attack / release are the Prophet's amp envelope, sent as it moves */
const knob=[...$('perform-controls').querySelectorAll('input[type=range]')][0];
assert.equal(+knob.value,(await p5())[44],'ENV attack shows the Prophet amp attack');
knob.value='100';knob.dispatchEvent(new w.Event('input',{bubbles:true}));await sleep(200);
assert.equal((await p5())[44],100,'ENV attack reaches the Prophet amp attack');

/* Shape: its ENV group is the filter and amp envelopes, live */
const env=[...$('groups').querySelectorAll('.group')].find(g=>/Amplifier envelope/.test(g.textContent));
assert.ok(env&&/Amplifier envelope/.test(env.textContent)&&/Filter envelope/.test(env.textContent)&&env.querySelectorAll('.row').length===8,'ENV group: the Prophet envelopes');
const rel=[...env.querySelectorAll('.row')].filter(r=>r.textContent.startsWith('Release'))[1].querySelector('input');
rel.value='7';rel.dispatchEvent(new w.Event('input',{bubbles:true}));await sleep(200);
assert.equal((await p5())[50],7,'ENV group release reaches the device');

/* Layers: a layer's octave is its TRN, a whole octave a press, on any part */
const trn=d.pdesc.findIndex((x,k)=>x&&k<d.info.pe0&&x.label==='TRN');
const strip=$('mixer').querySelectorAll('.strip')[2],[dn,out,up]=strip.querySelector('.oct').children;
up.click();up.click();await sleep(200);
assert.equal(parse[CMD.TRACK_PARAM](await T.rq(req.trackParam(2,trn))).value,24,'Oct + twice: layer 3 two octaves up');
assert.equal(out.textContent,'Oct +2');
dn.click();await sleep(200);
assert.equal(parse[CMD.TRACK_PARAM](await T.rq(req.trackParam(2,trn))).value,12,'Oct -');

/* Device sounds: search, engine and source as the Library */
d.preferences={state:{caps:8,filter:0,palette:0},palettes:['RED'],favorites:[],slots:{slots:[{used:1,slot:0,name:'ZQX TEST',engine:d.dump.engine}]}};
T.renderPreferences(true);
{
  const rows=()=>[...$('device-preset-list').querySelectorAll('tr')].length;
  const all=rows();
  $('deveng').value=[...$('deveng').options][1].value;$('deveng').dispatchEvent(new w.Event('change'));
  assert.ok(rows()>0&&rows()<all,'engine filter narrows the device sounds');
  assert.match($('devcount').textContent,/^\d+ \/ \d+$/);
  $('deveng').value='';$('deveng').dispatchEvent(new w.Event('change'));
  $('devkind').value='u';$('devkind').dispatchEvent(new w.Event('change'));
  assert.ok(rows()<all,'source filter: user slots only');
  $('devkind').value='';$('devkind').dispatchEvent(new w.Event('change'));
  $('devq').value='zqx';$('devq').dispatchEvent(new w.Event('input'));
  assert.equal(rows(),1,'search finds the user sound');
}
console.log('Connected editor: Prophet ENV, layer octaves and Device sounds filters passed');
process.exit(0);
