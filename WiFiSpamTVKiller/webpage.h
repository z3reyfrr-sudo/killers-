#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#0b0e14">
<title>WiFi Spammer + TV Killer</title>
<style>
:root{--bg:#0b0e14;--card:#151b26;--card2:#1e2632;--fg:#e7eef9;--dim:#8a97ab;
--acc:#3b82f6;--ok:#22c55e;--bad:#ef4444;--line:#26303f}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0 auto;max-width:560px;padding:14px 14px 50px;background:var(--bg);
color:var(--fg);font:15px/1.45 -apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif}
header{display:flex;align-items:center;gap:10px;padding:6px 0 14px}
h1{font-size:18px;margin:0;flex:1;font-weight:500}
.dot{width:9px;height:9px;border-radius:50%;background:var(--bad);transition:.2s}
.dot.on{background:var(--ok);box-shadow:0 0 10px var(--ok)}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;
padding:14px;margin-bottom:14px}
.card h2{font-size:12px;letter-spacing:.09em;text-transform:uppercase;color:var(--dim);
margin:0 0 12px;font-weight:500}
button{font:inherit;color:var(--fg);background:var(--card2);border:1px solid var(--line);
border-radius:10px;padding:11px 13px;cursor:pointer;transition:.12s}
button:active{transform:scale(.97)}
.big{width:100%;font-size:17px;font-weight:500;padding:16px;border-radius:12px;
background:var(--acc);border-color:var(--acc)}
.big.on{background:var(--bad);border-color:var(--bad)}
.seg{display:flex;gap:6px;flex-wrap:wrap;margin-bottom:12px}
.seg button{flex:1 1 30%;font-size:13px;padding:9px 6px}
.seg button.sel{background:var(--acc);border-color:var(--acc)}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.grid button{font-size:13px;padding:12px 6px}
.kill{grid-column:1/-1;background:var(--bad);border-color:var(--bad);font-weight:500}
label{display:flex;justify-content:space-between;font-size:13px;color:var(--dim);
margin:12px 0 5px}
label b{color:var(--fg);font-weight:500}
input[type=range]{width:100%;accent-color:var(--acc)}
input[type=text]{width:100%;background:var(--card2);border:1px solid var(--line);
color:var(--fg);border-radius:9px;padding:11px;font:inherit}
#chips{display:flex;flex-wrap:wrap;gap:5px;margin-top:12px;max-height:190px;overflow:auto}
.chip{background:var(--card2);border:1px solid var(--line);border-radius:7px;
padding:4px 8px;font-size:11.5px;color:var(--dim);white-space:nowrap}
#log{font:12px/1.5 ui-monospace,Menlo,monospace;color:var(--dim);margin-top:10px;
min-height:18px;word-break:break-word}
#log b{color:var(--ok);font-weight:400}
.stat{display:flex;justify-content:space-between;font-size:12.5px;color:var(--dim);
padding:5px 0;border-bottom:1px solid var(--line)}
.stat:last-child{border:0}.stat span:last-child{color:var(--fg)}
</style></head><body>

<header>
  <span class="dot" id="dot"></span>
  <h1>WiFi Spammer + TV Killer</h1>
</header>

<div class="card">
  <h2>Fake WiFi spam</h2>
  <button class="big" id="toggle">Spam: OFF</button>

  <div class="seg" id="modes" style="margin-top:12px">
    <button data-m="0">Random</button>
    <button data-m="1">Funny</button>
    <button data-m="2">Realistic</button>
    <button data-m="3">Rickroll</button>
    <button data-m="4" style="flex:1 1 100%">Custom message</button>
  </div>

  <input type="text" id="msg" maxlength="180"
         placeholder="Type a message, it gets split across networks">

  <label>Networks <b id="vCount">40</b></label>
  <input type="range" id="count" min="4" max="100" step="1" value="40">

  <label>Sweep delay <b id="vSpeed">120 ms</b></label>
  <input type="range" id="speed" min="30" max="500" step="10" value="120">

  <label>Channel <b id="vChan">1</b></label>
  <input type="range" id="chan" min="1" max="11" step="5" value="1">

  <div id="chips"></div>
  <div id="log"></div>
</div>

<div class="card">
  <h2>TV killer</h2>
  <div class="grid" id="brands"></div>
  <div id="log2" style="font:12px/1.5 ui-monospace,Menlo,monospace;color:var(--dim);margin-top:10px"></div>
</div>

<div class="card">
  <h2>Device</h2>
  <div class="stat"><span>Mode</span><span id="sMode">-</span></div>
  <div class="stat"><span>Networks broadcast</span><span id="sN">-</span></div>
  <div class="stat"><span>Channel</span><span id="sCh">-</span></div>
  <div class="stat"><span>Uptime</span><span id="sUp">-</span></div>
</div>

<script>
const $=s=>document.querySelector(s);
let state={spam:false,mode:0,count:40,speed:120,ch:1,msg:'',ssids:[],brands:[]};

function api(p){return fetch('/api/'+p).then(r=>r.json()).catch(()=>null);}
function log(el,t){el.innerHTML='<b>&gt;</b> '+t;}

function paint(){
  $('#toggle').textContent='Spam: '+(state.spam?'ON':'OFF');
  $('#toggle').classList.toggle('on',state.spam);
  $('#dot').classList.toggle('on',state.spam);
  document.querySelectorAll('#modes button').forEach(b=>
    b.classList.toggle('sel',+b.dataset.m===state.mode));
  $('#count').value=state.count; $('#vCount').textContent=state.count;
  $('#speed').value=state.speed; $('#vSpeed').textContent=state.speed+' ms';
  $('#chan').value=state.ch;     $('#vChan').textContent=state.ch;
  $('#msg').value=state.msg;
  $('#sMode').textContent=['Random','Funny','Realistic','Rickroll','Message'][state.mode];
  $('#sN').textContent=state.ssids.length;
  $('#sCh').textContent=state.ch;
  $('#chips').innerHTML=state.ssids.slice(0,80).map(s=>
    '<span class="chip">'+s.replace(/&/g,'&amp;').replace(/</g,'&lt;')+'</span>').join('');
}

function sync(){api('state').then(d=>{if(d){state=Object.assign(state,d);paint();}});}

$('#toggle').onclick=()=>api('spam?on='+(state.spam?0:1)).then(sync);
$('#modes').onclick=e=>{const b=e.target.closest('button');if(!b)return;
  api('mode?v='+b.dataset.m).then(sync);};
$('#msg').onchange=e=>api('msg?v='+encodeURIComponent(e.target.value)).then(sync);
$('#count').oninput=e=>{$('#vCount').textContent=e.target.value;};
$('#count').onchange=e=>api('count?v='+e.target.value).then(sync);
$('#speed').oninput=e=>{$('#vSpeed').textContent=e.target.value+' ms';};
$('#speed').onchange=e=>api('speed?v='+e.target.value).then(sync);
$('#chan').oninput=e=>{$('#vChan').textContent=e.target.value;};
$('#chan').onchange=e=>api('chan?v='+e.target.value).then(sync);

let brandsBuilt=false;
function buildBrands(){
  if(brandsBuilt||!state.brands.length)return; brandsBuilt=true;
  const g=$('#brands');
  state.brands.forEach((b,i)=>{
    const el=document.createElement('button'); el.textContent=b;
    el.onclick=()=>{api('tv?v='+i); log($('#log2'),'fired '+b);};
    g.appendChild(el);
  });
  const k=document.createElement('button'); k.className='kill';
  k.textContent='KILL ALL TVs';
  k.onclick=()=>{api('killall'); log($('#log2'),'firing every code...');};
  g.appendChild(k);
}

setInterval(()=>{api('state').then(d=>{if(d){state=Object.assign(state,d);paint();buildBrands();}});},1500);
api('state').then(d=>{if(d){state=Object.assign(state,d);paint();buildBrands();}});
api('uptime').then(d=>{if(d)$('#sUp').textContent=d.uptime;});
setInterval(()=>api('uptime').then(d=>{if(d)$('#sUp').textContent=d.uptime;}),5000);
</script>
</body></html>
)rawliteral";
