// Páginas del nodo web, embebidas en la flash. Sin recursos externos (el punto de
// acceso del dispositivo no tiene internet): fuentes del sistema y todo en línea.
// Los textos dinámicos se insertan con textContent, nunca como HTML.
#pragma once
#include <pgmspace.h>

static const char STYLE_HTML[] PROGMEM = R"CSS(
:root{--bg:#f3f4f1;--surface:#fff;--line:#d9ddd5;--ink:#0d1014;--ink2:#3a424c;--muted:#5d6672;--accent:#0089a8;--ok:#0f8a4c;--mid:#a86400;--bad:#c42a3c}
@media (prefers-color-scheme:dark){:root{--bg:#0a0c0f;--surface:#121519;--line:#2e353e;--ink:#eef1f4;--ink2:#c3cad3;--muted:#8c96a3;--accent:#3ee0ff;--ok:#3ddc84;--mid:#ffb340;--bad:#ff5a6a}}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--ink);font:16px/1.5 system-ui,-apple-system,"Segoe UI",sans-serif}
.mono{font-family:ui-monospace,Menlo,Consolas,monospace;font-variant-numeric:tabular-nums}
h1{font-size:1.5rem;margin:0;letter-spacing:-.02em}
h2{font-size:1.15rem;margin:0}
.wrap{max-width:1100px;margin:0 auto;padding:20px 16px 32px}
:focus-visible{outline:2px solid var(--accent);outline-offset:3px}
::selection{background:var(--accent);color:var(--bg)}
)CSS";

static const char DASHBOARD_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Aterriza · Dispositivo</title>
<link rel="stylesheet" href="/style.css">
<style>
header{display:flex;flex-wrap:wrap;gap:16px;align-items:center;justify-content:space-between;margin-bottom:24px}
.net{display:flex;align-items:center;gap:12px;border:1px solid var(--line);background:var(--surface);border-radius:14px;padding:10px 16px}
.net b{display:block;line-height:1.2}
.net small{color:var(--muted)}
.dot{flex:none;width:12px;height:12px;border-radius:50%;background:var(--muted)}
.ok .dot{background:var(--ok)}.mid .dot{background:var(--mid)}.bad .dot{background:var(--bad)}
.off .dot{background:none;border:2px solid var(--muted)}
.grid{display:grid;gap:16px;grid-template-columns:repeat(auto-fit,minmax(290px,1fr))}
.imu{background:var(--surface);border:1px solid var(--line);border-radius:18px;padding:18px}
.imu header{margin:0 0 4px}
.badge{display:inline-flex;align-items:center;gap:8px;font-weight:600;font-size:.9rem}
.badge.ok{color:var(--ok)}.badge.mid{color:var(--mid)}.badge.bad{color:var(--bad)}.badge.off{color:var(--muted)}
.meta{margin:0 0 14px;color:var(--muted);font-size:.85rem}
dl{margin:0;display:grid;grid-template-columns:auto 1fr;gap:8px 14px;align-items:baseline}
dt{color:var(--ink2)}dt small{color:var(--muted)}
dd{margin:0;text-align:right;font-size:.95rem}
.u{color:var(--muted)}
.imu.nodata dd{color:var(--muted)}
footer{margin-top:24px;color:var(--muted);font-size:.85rem;display:flex;flex-wrap:wrap;gap:6px 24px}
@media (max-width:420px){.wrap{padding:16px 16px 24px}}
</style></head><body><div class="wrap">
<header>
 <h1>Aterriza</h1>
 <div id="net" class="net off" role="status" aria-live="polite"><span class="dot"></span><span><b id="netT">Conectando…</b><small id="netS" class="mono"></small></span></div>
</header>
<main id="imus" class="grid" aria-label="Sensores de movimiento"></main>
<footer id="foot" class="mono"></footer>
</div>
<script>
const NOMBRE={ok:'OK',sin_respuesta:'Sin respuesta',no_conectada:'No conectada'};
const CLASE={ok:'ok',sin_respuesta:'mid',no_conectada:'off'};
const FILAS=[['Acelerómetro','acel_g','g',2],['Giroscopio','giro_dps','°/s',1],['Magnetómetro','mag_uT','µT',1]];
const f=(v,d)=>v==null?'—':v.toFixed(d);
const el=(t,c,x)=>{const e=document.createElement(t);if(c)e.className=c;if(x!=null)e.textContent=x;return e};
const $=id=>document.getElementById(id);
let panels=null;
function build(imus){
 const root=$('imus');root.textContent='';panels=imus.map(()=>{
  const s=el('section','imu'),h=el('header'),t=el('h2'),b=el('span','badge'),m=el('p','meta mono'),d=el('dl'),rows={};
  b.append(el('span','dot'),el('span'));h.append(t,b);
  FILAS.forEach(([n,k,u])=>{const dt=el('dt',null,n+' ');dt.append(el('small',null,'('+u+')'));const dd=el('dd','mono');d.append(dt,dd);rows[k]=dd});
  const dt1=el('dt',null,'Inclinación '),d1=el('dd','mono');dt1.append(el('small',null,'(°)'));d.append(dt1,d1);rows.inc=d1;
  const dt2=el('dt',null,'Errores I2C '),d2=el('dd','mono');dt2.append(el('small',null,'(cuenta)'));d.append(dt2,d2);rows.err=d2;
  s.append(h,m,d);root.append(s);return{s,t,b,m,rows}});
}
function vec(o,d,u){return o?`x ${f(o.x,d)}  y ${f(o.y,d)}  z ${f(o.z,d)}  |${f(o.mag,d)}| ${u}`:'—'}
function paint(j){
 if(!panels)build(j.imu);
 j.imu.forEach((m,i)=>{const p=panels[i],c=CLASE[m.estado]||'off';
  p.t.textContent=m.nombre;p.b.className='badge '+c;p.b.lastChild.textContent=NOMBRE[m.estado]||m.estado;
  p.s.classList.toggle('nodata',m.estado==='no_conectada');
  p.m.textContent=m.modelo?`${m.modelo} · mag ${m.magnetometro} · ${m.direccion_i2c}`:'Sin detectar en su bus I2C';
  FILAS.forEach(([n,k,u,d])=>p.rows[k].textContent=vec(m[k],d,u));
  p.rows.inc.textContent=m.inclinacion_deg==null?'—':f(m.inclinacion_deg,1)+' °';
  p.rows.err.textContent=m.errores_i2c});
 const w=j.wifi,n=$('net');let cls='off',t='Sin red',s='';
 const cal=r=>r>-55?'excelente':r>-67?'buena':r>-80?'débil':'muy débil';
 if(w.estado==='conectado'){cls=w.rssi_dbm>-80?'ok':'mid';t='WiFi conectado';s=`${w.ssid} · ${w.ip} · ${w.rssi_dbm} dBm (${cal(w.rssi_dbm)})`}
 else if(w.estado==='punto_acceso'){cls='mid';t='Punto de acceso';s=`${w.ap_ssid} · ${w.ap_ip}`+(w.ssid?` · buscando «${w.ssid}»`:'')}
 else if(w.estado==='conectando'){cls='mid';t='Conectando…';s=w.ssid}
 n.className='net '+cls;$('netT').textContent=t;$('netS').textContent=s;
 $('foot').textContent='';
 [`Muestreo ${f(j.muestreo.hz,1)} Hz`,`Tara ${j.muestreo.tara?'hecha':'pendiente'}`,`Encendido ${Math.floor(j.uptime_s/60)} min`,`Firmware ${j.fw}`,`Flujo TCP ${w.cliente_tcp?'con cliente':'libre'}`]
  .forEach(x=>$('foot').append(el('span',null,x)));
}
function lost(msg){const n=$('net');n.className='net bad';$('netT').textContent=msg;$('netS').textContent='';}
async function tick(){
 try{const r=await fetch('/api/status',{cache:'no-store'});
  if(r.status===401)lost('Acceso denegado');else if(r.status===429)lost('Demasiados intentos: espera 30 s');
  else if(!r.ok)lost('Error '+r.status);else paint(await r.json());
 }catch(e){lost('Sin conexión con el dispositivo')}
 setTimeout(tick,document.hidden?3000:500);
}
tick();
</script></body></html>)HTML";

static const char SETUP_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Configurar · Aterriza</title><link rel="stylesheet" href="/style.css">
<style>
form{max-width:420px;display:grid;gap:14px}
fieldset{border:1px solid var(--line);border-radius:14px;padding:14px 16px;display:grid;gap:12px;background:var(--surface)}
legend{font-weight:600;padding:0 6px}
label{display:grid;gap:4px;color:var(--ink2);font-size:.9rem}
input{font:inherit;color:var(--ink);background:var(--bg);border:1px solid var(--line);border-radius:10px;padding:10px 12px;min-height:44px}
input:focus{border-color:var(--accent)}
small{color:var(--muted)}
button{font:inherit;font-weight:600;border:0;border-radius:999px;min-height:48px;padding:0 24px;background:var(--ink);color:var(--bg);cursor:pointer}
button:hover{box-shadow:0 2px 10px rgba(0,0,0,.25)}
</style></head><body><div class="wrap">
<h1>Configurar el dispositivo</h1>
<p style="color:var(--ink2);max-width:46ch">Define con qué usuario y contraseña se consultan las lecturas. No son las del WiFi. La contraseña no se guarda: solo un resumen cifrado.</p>
<form method="post" action="/setup" autocomplete="off">
<fieldset><legend>Acceso al dispositivo</legend>
<label>Usuario<input name="user" required minlength="3" maxlength="32" pattern="[A-Za-z0-9._\-]+" autocapitalize="off" spellcheck="false"><small>3 a 32 caracteres: letras, números, punto, guion.</small></label>
<label>Contraseña<input name="pass" type="password" required minlength="8" maxlength="64"><small>Mínimo 8 caracteres.</small></label>
</fieldset>
<fieldset><legend>Red WiFi (opcional)</legend>
<label>Nombre de la red<input name="ssid" maxlength="32" autocapitalize="off" spellcheck="false"><small>Solo redes de 2,4 GHz. Vacío = dejar la configuración actual.</small></label>
<label>Clave de la red<input name="wpass" type="password" maxlength="63"></label>
</fieldset>
<button type="submit">Guardar</button>
</form></div></body></html>)HTML";
