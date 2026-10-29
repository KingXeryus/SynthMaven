const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict');
(async()=>{const b=await chromium.launch({channel:'chrome',headless:true});try{
 const p=await b.newPage({viewport:{width:1366,height:900},colorScheme:'light'});
 await p.addInitScript(()=>localStorage.setItem('synthmaven-appearance','light'));
 await p.goto(require('node:url').pathToFileURL(require('node:path').resolve('web/editor.html')).href+'?mock=1&auto=0#sound');
 await p.locator('#connect').click();await p.waitForTimeout(5000);
 await p.locator('[data-sm-section="tuning"]').click();await p.waitForTimeout(1000);
 assert.equal(await p.locator('html').getAttribute('data-theme'),'dark');assert.equal(await p.getByLabel('Appearance',{exact:true}).count(),0);
 const w=p.getByLabel('Selected key tuning in cents',{exact:true});await w.waitFor({state:'visible'});
 const d=await p.evaluate(()=>{const w=document.querySelector('.sm-cent-wheel'),k=document.querySelector('.sm-tuning-graphic'),a=w.getBoundingClientRect(),b=k.getBoundingClientRect();return {below:a.top>=b.bottom,center:Math.abs((a.left+a.right-b.left-b.right)/2),border:getComputedStyle(w).borderTopWidth,range:[w.min,w.max,w.step]};});
 assert.equal(d.below,true);assert.ok(d.center<2,JSON.stringify(d));assert.equal(d.border,'0px');assert.deepEqual(d.range,['-100','100','1']);
 await w.evaluate(e=>{e.value='-37';e.dispatchEvent(new Event('input',{bubbles:true}));});
 assert.equal(await p.locator('.sm-key-selected .sm-key-cents').textContent(),'-37 ct');
 await p.screenshot({path:'/private/tmp/fm1-web-tuning-dark.png'});console.log('PASS: desktop dark-only, centered below-keyboard borderless wheel, one-cent range and selected-key preview');
 }finally{await b.close();}})();
