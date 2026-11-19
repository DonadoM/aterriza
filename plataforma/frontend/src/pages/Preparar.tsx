import clsx from 'clsx'
import { ArrowRight } from 'lucide-react'
import { ModoSaltoToggle, useModoSalto } from '../components/ModoSalto'
import { Cargando } from '../components/marks'
import { AnimatePresence, motion } from 'motion/react'
import { useEffect, useRef, useState } from 'react'
import { Navigate, useNavigate } from 'react-router-dom'
import { AvisoUnaPlantilla, SelectorPie } from '../components/SelectorPie'
import { Button, inputCls } from '../components/ui'
import { api } from '../lib/api'
import { frame } from '../lib/live'
import { useApp, useLive } from '../lib/store'
import { useFrame } from '@react-three/fiber'
import { Athlete, newJoints } from '../three/Athlete'
import { emptyPose, poseFromFrame } from '../three/pose'
import { FloorPressure, Stage } from '../three/Stage'

const FUENTES = [
  { id: 'demo', titulo: 'Demostración', desc: 'Gemelo digital con el firmware real. Sin hardware.', tag: 'gemelo · 100 Hz' },
  { id: 'usb', titulo: 'ESP32 por USB', desc: 'Plantillas e IMU reales del prototipo.', tag: 'USB · 921 600 baud' },
  { id: 'wifi', titulo: 'ESP32 por WiFi', desc: 'Sin cable de datos: el equipo funciona con su cargador.', tag: 'WiFi · TCP :3333' },
  { id: 'wokwi', titulo: 'Wokwi', desc: 'Simulación del circuito en VS Code.', tag: 'rfc2217 · :4000' },
] as const

function LivePose({ scan }: { scan: React.MutableRefObject<number | null> }) {
  const pose = useRef(emptyPose())
  const joints = useRef(newJoints())
  const loads = useRef<number[]>([0, 0, 0, 0])
  useFrame(() => {
    if (frame.current) {
      poseFromFrame(frame.current, pose.current)
      loads.current = frame.current.fsr
    }
  })
  return (
    <>
      <Athlete pose={pose} joints={joints} scan={scan} />
      <FloorPressure loads={loads} joints={joints} />
    </>
  )
}

function Paso({ n, titulo, activo, hecho, children }: { n: number; titulo: string; activo: boolean; hecho: boolean; children?: React.ReactNode }) {
  return (
    <section className={clsx('relative border-l-2 pb-10 pl-8 transition-colors', hecho ? 'border-ok' : activo ? 'border-accent' : 'border-line')}>
      <span className={clsx('absolute -left-[17px] top-0 grid size-8 place-items-center rounded-full border-2 text-sm font-bold transition', hecho ? 'border-ok bg-ok text-bg' : activo ? 'border-accent bg-bg text-accent' : 'border-line bg-bg text-muted')}>
        {n}
      </span>
      <h2 className={clsx('display-cond text-[28px] leading-8', !activo && !hecho && 'text-muted')}>{titulo}</h2>
      <AnimatePresence initial={false}>
        {activo && (
          <motion.div initial={{ opacity: 0, height: 0 }} animate={{ opacity: 1, height: 'auto' }} exit={{ opacity: 0, height: 0 }} transition={{ duration: 0.35, ease: [0.16, 1, 0.3, 1] }} className="overflow-hidden">
            <div className="pt-5">{children}</div>
          </motion.div>
        )}
      </AnimatePresence>
    </section>
  )
}

const HOST_WIFI = 'aterriza.local'
const HOST_AP = '192.168.4.1'
const leerHost = () => {
  try {
    return localStorage.getItem('wifi-host') || HOST_WIFI
  } catch {
    return HOST_WIFI
  }
}
const guardarHost = (h: string) => {
  try {
    localStorage.setItem('wifi-host', h)
  } catch {
    /* almacenamiento no disponible */
  }
}

export default function Preparar() {
  const modo = useModoSalto()
  const nav = useNavigate()
  const atleta = useApp((s) => s.atleta)
  const st = useLive((s) => s.status)
  const [fuente, setFuente] = useState<string>('demo')
  const [puertos, setPuertos] = useState<string[]>([])
  const [puerto, setPuerto] = useState('')
  const [url, setUrl] = useState('rfc2217://localhost:4000')
  const [host, setHost] = useState(leerHost)
  const [busy, setBusy] = useState(false)
  const [err, setErr] = useState('')
  const [cuenta, setCuenta] = useState<number | null>(null)
  const [taraOk, setTaraOk] = useState(false)
  const scan = useRef<number | null>(null)

  const conectado = st?.estado === 'conectado' && st.fresco
  const paso = !conectado ? 1 : !taraOk ? 2 : 3

  useEffect(() => {
    if (fuente === 'usb')
      api.get<{ puertos: string[]; sugerido: string | null }>('/api/puertos').then((r) => {
        setPuertos(r.puertos)
        setPuerto(r.sugerido ?? r.puertos[0] ?? '')
      })
  }, [fuente])

  if (!atleta) return <Navigate to="/" replace />

  const conectar = async () => {
    setBusy(true)
    setErr('')
    setTaraOk(false)
    try {
      const dir = host.trim() || HOST_WIFI
      await api.post('/api/conectar', { fuente, puerto: fuente === 'usb' ? puerto : undefined, url: fuente === 'wokwi' ? url : undefined, host: fuente === 'wifi' ? dir : undefined })
      if (fuente === 'wifi') guardarHost(dir)
    } catch (e) {
      setErr((e as Error).message)
    } finally {
      setBusy(false)
    }
  }

  const tara = async () => {
    setErr('')
    for (const n of [3, 2, 1]) {
      setCuenta(n)
      await new Promise((r) => setTimeout(r, 900))
    }
    setCuenta(0)
    try {
      const r = await api.post<{ ok: boolean }>('/api/tara')
      if (!r.ok) throw new Error('No se detectó quietud. Quédate quieto y repite.')
      setTaraOk(true)
    } catch (e) {
      setErr((e as Error).message)
    } finally {
      setCuenta(null)
    }
  }

  const empezar = async () => {
    await api.post('/api/sesion', { atleta_id: atleta.id })
    nav('/evaluar')
  }

  return (
    <div className="grid min-h-[calc(100vh-4rem)] lg:grid-cols-[minmax(0,1fr)_minmax(0,1.1fr)]">
      <div className="px-6 py-10 lg:px-16">
        <p className="eyebrow">Preparación guiada · {atleta.nombre}</p>
        <h1 className="display mb-6 mt-3 text-5xl">Listos en 3 pasos</h1>
        <AvisoUnaPlantilla className="mb-8 max-w-lg" />

        <Paso n={1} titulo="Conecta el dispositivo" activo={paso === 1} hecho={paso > 1}>
          <div className="grid gap-3 sm:grid-cols-2">
            {FUENTES.map((f) => (
              <button key={f.id} onClick={() => setFuente(f.id)} className={clsx('flex flex-col items-start gap-3 rounded-2xl border p-4 text-left transition', fuente === f.id ? 'border-accent bg-accent-soft' : 'border-line hover:border-line-2')} data-testid={`fuente-${f.id}`}>
                <span className={clsx('font-mono text-[11px] uppercase tracking-[0.12em]', fuente === f.id ? 'text-accent' : 'text-muted')}>{f.tag}</span>
                <span>
                  <span className="block font-semibold">{f.titulo}</span>
                  <span className="text-sm leading-snug text-muted">{f.desc}</span>
                </span>
              </button>
            ))}
          </div>
          {fuente === 'usb' && (
            <select className={clsx(inputCls, 'mt-4 w-full')} value={puerto} onChange={(e) => setPuerto(e.target.value)} aria-label="Puerto USB">
              {puertos.length === 0 && <option value="">No se encontró ningún ESP32 (¿cable de datos?)</option>}
              {puertos.map((p) => (
                <option key={p}>{p}</option>
              ))}
            </select>
          )}
          {fuente === 'wifi' && (
            <div className="mt-4">
              <input className={clsx(inputCls, 'w-full font-mono text-sm')} value={host} onChange={(e) => setHost(e.target.value)} list="hosts-wifi" placeholder={HOST_WIFI} spellCheck={false} autoCapitalize="off" autoCorrect="off" aria-label="Dirección del equipo por WiFi" aria-describedby="ayuda-wifi" data-testid="wifi-host" />
              <datalist id="hosts-wifi">
                <option value={HOST_WIFI} />
                <option value={HOST_AP} />
              </datalist>
              <p id="ayuda-wifi" className="mt-2 max-w-lg text-sm leading-snug text-muted">
                Enciende el equipo con su cargador. Si está en la red del laboratorio, usa <span className="font-mono text-ink-2">{HOST_WIFI}</span>. Si no, crea la red «Aterriza-ESP32»: únete a ella desde este computador y usa <span className="font-mono text-ink-2">{HOST_AP}</span>.
              </p>
            </div>
          )}
          {fuente === 'wokwi' && <input className={clsx(inputCls, 'mt-4 w-full font-mono text-sm')} value={url} onChange={(e) => setUrl(e.target.value)} aria-label="URL de Wokwi" />}
          <Button size="lg" className="mt-5" onClick={conectar} disabled={busy} data-testid="conectar">
            {busy ? <Cargando /> : null} Conectar
          </Button>
        </Paso>

        <Paso n={2} titulo="Tara de pie" activo={paso === 2} hecho={paso > 2}>
          <p className="max-w-md text-lg text-ink-2">Párate derecho con el pie medido sobre la plantilla, apoyando el peso en él, y quédate quieto. Esa postura será el cero de los ángulos.</p>
          <p className="mt-2 max-w-md text-sm text-muted">Sin atleta: deja las IMU quietas en su posición de referencia (eje X hacia arriba, como de pie). Puedes volver a tarar cuando quieras con el botón «Tarar» de arriba.</p>
          <Button size="lg" className="mt-5" onClick={tara} disabled={cuenta !== null} data-testid="tara">
            Iniciar tara
          </Button>
        </Paso>

        <Paso n={3} titulo="¡Todo listo!" activo={paso === 3} hecho={false}>
          <p className="max-w-md text-lg text-ink-2">{modo === 'cajon' ? 'Protocolo: 3 aterrizajes desde el cajón de 30 – 40 cm.' : 'Protocolo: 3 saltos verticales en el sitio, aterrizando en el mismo lugar.'} El sistema analiza cada salto solo.</p>
          <ModoSaltoToggle className="mt-4 self-start !bg-surface-2" />
          <SelectorPie className="mt-5" />
          <Button size="xl" className="mt-5" onClick={empezar} data-testid="empezar">
            Empezar evaluación <ArrowRight className="size-6" />
          </Button>
        </Paso>
        {err && <p className="mt-2 rounded-xl bg-bad-soft p-3 text-bad">{err}</p>}
      </div>

      <div className="relative min-h-[460px] overflow-hidden lg:sticky lg:top-16 lg:h-[calc(100vh-4rem)]">
        <Stage vista="frontal" zoom={1.05} focusY={0.9}>
          <LivePose scan={scan} />
        </Stage>
        <div className="pointer-events-none absolute inset-x-0 top-0 h-24 bg-gradient-to-b from-bg to-transparent" />
        <AnimatePresence>
          {cuenta !== null && (
            <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }} className="absolute inset-0 grid place-items-center bg-bg/40 backdrop-blur-[2px]">
              <div className="text-center">
                <p className="display-cond text-3xl">Quédate de pie, quieto</p>
                <AnimatePresence mode="popLayout">
                  <motion.p key={cuenta} initial={{ scale: 1.6, opacity: 0 }} animate={{ scale: 1, opacity: 1 }} exit={{ scale: 0.6, opacity: 0 }} transition={{ duration: 0.45, ease: [0.16, 1, 0.3, 1] }} className="display mt-2 text-[180px] text-accent">
                    {cuenta || '·'}
                  </motion.p>
                </AnimatePresence>
                {cuenta === 0 && <p className="text-lg text-ink-2">Midiendo quietud…</p>}
              </div>
            </motion.div>
          )}
        </AnimatePresence>
        <div className="absolute bottom-6 left-6 flex items-center gap-2 rounded-full border border-line bg-surface/70 px-3 py-1.5 text-sm text-ink-2 backdrop-blur">
          <span className="inline-block h-2.5 w-4 rounded-sm" style={{ background: 'repeating-linear-gradient(45deg, var(--accent) 0 2px, transparent 2px 5px)' }} />
          Pierna izquierda estimada (sin IMU, en espejo)
        </div>
      </div>
    </div>
  )
}
