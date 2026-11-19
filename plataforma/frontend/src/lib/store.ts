import { create } from 'zustand'
import type { Atleta, SaltoEvento, Status } from './types'

const read = (k: string) => {
  try {
    return localStorage.getItem(k)
  } catch {
    return null
  }
}
const write = (k: string, v: string | null) => {
  try {
    if (v === null) localStorage.removeItem(k)
    else localStorage.setItem(k, v)
  } catch {
    /* almacenamiento no disponible */
  }
}

/** Pie medido en la toma (la plantilla física va siempre a los canales P1). Solo en el cliente: el servidor no lo guarda. */
export type Pie = 'izquierdo' | 'derecho'
export const PIE_TXT: Record<Pie, string> = { izquierdo: 'Pie izquierdo', derecho: 'Pie derecho' }
const leerPie = (): Pie => (read('pie-medido') === 'derecho' ? 'derecho' : 'izquierdo')

const MAPA_PIES = 'pies-por-salto'
const leerMapaPies = (): Record<string, Pie> => {
  try {
    const m = JSON.parse(read(MAPA_PIES) || '{}')
    return m && typeof m === 'object' ? m : {}
  } catch {
    return {}
  }
}
/** Reglas que comparan los dos pies: sin datos del otro pie (una sola plantilla) no tienen sentido. */
const REGLAS_BILATERALES = new Set(['simetria_contacto', 'simetria_carga'])
export const sinBilaterales = <T extends { id: string }>(xs: T[]): T[] => xs.filter((x) => !REGLAS_BILATERALES.has(x.id))
/** Etiqueta del salto registrada al recibirlo por el WebSocket; null si es anterior a este cambio. */
export const pieDeSalto = (id: number | null | undefined): Pie | null => {
  if (id == null) return null
  const p = leerMapaPies()[String(id)]
  return p === 'izquierdo' || p === 'derecho' ? p : null
}
export const registrarPieSalto = (id: number | null | undefined, pie: Pie) => {
  if (id == null) return
  write(MAPA_PIES, JSON.stringify({ ...leerMapaPies(), [String(id)]: pie }))
}

interface AppState {
  pie: Pie
  setPie: (p: Pie) => void
  atleta: Atleta | null
  setAtleta: (a: Atleta | null) => void
  tema: 'dark' | 'light'
  toggleTema: () => void
  proToken: string | null
  setProToken: (t: string | null) => void
}

export const useApp = create<AppState>((set, get) => ({
  pie: leerPie(),
  setPie: (p) => {
    write('pie-medido', p)
    set({ pie: p })
  },
  atleta: (() => {
    const s = read('atleta')
    return s ? (JSON.parse(s) as Atleta) : null
  })(),
  setAtleta: (a) => {
    write('atleta', a ? JSON.stringify(a) : null)
    set({ atleta: a })
  },
  tema: (document.documentElement.dataset.theme as 'dark' | 'light') || 'dark',
  toggleTema: () => {
    const t = get().tema === 'dark' ? 'light' : 'dark'
    document.documentElement.dataset.theme = t
    write('tema', t)
    set({ tema: t })
  },
  proToken: sessionStorage.getItem('pro') || null,
  setProToken: (t) => {
    if (t) sessionStorage.setItem('pro', t)
    else sessionStorage.removeItem('pro')
    set({ proToken: t })
  },
}))

interface LiveState {
  conectadoWs: boolean
  status: Status | null
  ultimoSalto: SaltoEvento | null
  set: (p: Partial<LiveState>) => void
}

export const useLive = create<LiveState>((set) => ({
  conectadoWs: false,
  status: null,
  ultimoSalto: null,
  set: (p) => set(p),
}))
