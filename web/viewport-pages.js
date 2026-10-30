/* SPDX-License-Identifier: GPL-3.0-only
 * Viewport pages, shared by web and iOS. Navigation changes presentation only.
 * Keep whole control rows on a page; never scale touch targets down to fit.
 */
(() => {
 const main=document.querySelector('main'),panels=[...document.querySelectorAll('.panel')];
 const viewport=document.createElement('div'),content=document.createElement('div'),nav=document.createElement('nav');
 viewport.id='viewport-pages';content.id='viewport-content';nav.id='viewport-navigation';nav.setAttribute('aria-label','Screen pages');
 panels[0].before(viewport);viewport.append(content);content.append(...panels);
 const prev=document.createElement('button'),next=document.createElement('button'),label=document.createElement('span');
 prev.textContent='Previous page';next.textContent='Next page';label.setAttribute('aria-live','polite');nav.append(prev,label,next);main.append(nav);
 const play=document.createElement('button'),stop=document.createElement('button');play.textContent='▶ Play';stop.textContent='■ Stop';nav.prepend(play,stop);
 const action=name=>document.getElementById((key==='p-drums'?'drum-':'seq-')+name)?.click();play.onclick=()=>action('play');stop.onclick=()=>action('stop');
 let page=0,cuts=[0],key='',scheduled=false;
 function layout(){
  scheduled=false;const active=panels.find(p=>!p.hidden),newKey=active?.id||'';
  if(newKey!==key){page=0;key=newKey;}
  content.style.transform='';viewport.style.height='';
  const available=Math.max(100,window.innerHeight-viewport.getBoundingClientRect().top-nav.getBoundingClientRect().height-12);
  viewport.style.height=available+'px';
  const total=content.scrollHeight,origin=content.getBoundingClientRect().top;
  const boxes=[...content.querySelectorAll('button,input,select,textarea,summary,tr,.row,.drum-grid>button')].filter(n=>n.getClientRects().length).map(n=>{const r=n.getBoundingClientRect();return [r.top-origin,r.bottom-origin];}).filter(([a,b])=>b-a<=available);
  cuts=[0];let start=0;
  while(total-start>available+1){
   let end=start+available;
   for(let attempt=0;attempt<100;attempt++){
    const overlaps=boxes.filter(([a,b])=>a<end-1&&b>end+1&&a>start+8);
    if(!overlaps.length)break;
    const proposed=Math.min(...overlaps.map(x=>x[0]))-4;
    if(proposed<=start+available*.35)break;
    end=proposed;
   }
   if(end<=start+8)end=start+available;
   cuts.push(end);start=end;if(cuts.length>500)break;
  }
  play.hidden=stop.hidden=!(['p-drums','p-sequencer'].includes(key));
  for(const [button,name]of [[play,'play'],[stop,'stop']]){const source=document.getElementById((key==='p-drums'?'drum-':'seq-')+name);button.disabled=!source||source.disabled;button.setAttribute('aria-pressed',source?.getAttribute('aria-pressed')||'false');}
  page=Math.min(page,cuts.length-1);draw();
 }
 function draw(){
  const end=cuts[page+1],height=parseFloat(viewport.style.height);
  content.style.transform=`translateY(${-cuts[page]}px)`;
  viewport.style.clipPath=`inset(0 0 ${end?Math.max(0,height-(end-cuts[page])):0}px 0)`;
  prev.disabled=page===0;next.disabled=page===cuts.length-1;label.textContent=`${page+1} / ${cuts.length}`;
  // Off-page widgets cannot trap keyboard focus in invisible content.
  for(const n of content.querySelectorAll('button,input,select,textarea,a,summary')){
   const r=n.getBoundingClientRect(),v=viewport.getBoundingClientRect();
   const visible=r.height&&r.top>=v.top-1&&r.bottom<=v.top+(end?end-cuts[page]:height)+1;
   if(!visible){if(!n.hasAttribute('data-page-tab'))n.dataset.pageTab=n.getAttribute('tabindex')??'';n.setAttribute('tabindex','-1');}
   else if(n.hasAttribute('data-page-tab')){const old=n.dataset.pageTab;if(old==='')n.removeAttribute('tabindex');else n.setAttribute('tabindex',old);delete n.dataset.pageTab;}
  }
 }
 function schedule(){if(!scheduled){scheduled=true;requestAnimationFrame(layout);}}
 prev.onclick=()=>{page=Math.max(0,page-1);draw();};next.onclick=()=>{page=Math.min(cuts.length-1,page+1);draw();};
 const observer=new MutationObserver(schedule);observer.observe(content,{childList:true,subtree:true,attributes:true,attributeFilter:['hidden','open','data-shape-hidden','data-seq-view','class']});
 window.addEventListener('resize',schedule);window.visualViewport?.addEventListener('resize',schedule);document.addEventListener('toggle',schedule,true);
 new ResizeObserver(schedule).observe(document.querySelector('header.top'));
 schedule();
})();
