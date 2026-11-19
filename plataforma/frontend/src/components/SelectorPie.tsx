import clsx from 'clsx'
import { PIE_TXT, useApp, type Pie } from '../lib/store'

/** Aviso sobrio: con una sola plantilla no hay comparación entre pies. */
export function AvisoUnaPlantilla({ className }: { className?: string }) {
  return (
    <p className={clsx('rounded-xl border border-line bg-surface-2 px-3 py-2 text-sm text-ink-2', className)} role="note" data-testid="aviso-una-plantilla">
      Medición con una plantilla: sin comparación entre pies.
    </p>
  )
}

/** Etiqueta del pie medido en un salto (no muestra nada si no hay). */
export function EtiquetaPie({ pie, className }: { pie: Pie | null; className?: string }) {
  if (!pie) return null
  return (
    <span className={clsx('inline-flex items-center rounded-full border border-line bg-surface-2 px-2.5 py-0.5 text-xs font-semibold text-ink-2', className)} data-testid="etiqueta-pie">
      {PIE_TXT[pie]}
    </span>
  )
}

/** Selector "Pie medido": se cambia entre tomas sin reconectar. */
export function SelectorPie({ className, compacto }: { className?: string; compacto?: boolean }) {
  const pie = useApp((s) => s.pie)
  const setPie = useApp((s) => s.setPie)
  const op: [Pie, string][] = [
    ['izquierdo', 'Izquierdo'],
    ['derecho', 'Derecho'],
  ]
  return (
    <fieldset className={clsx('min-w-0', className)}>
      <legend className={clsx(compacto ? 'sr-only' : 'mb-2 text-sm font-semibold text-ink-2')}>Pie medido</legend>
      <div className="inline-flex gap-1 rounded-2xl border border-line bg-surface/75 p-1" role="radiogroup" aria-label="Pie medido">
        {op.map(([v, l]) => (
          <label key={v} className={clsx('relative flex min-h-11 min-w-28 cursor-pointer items-center justify-center gap-2 rounded-xl px-4 text-sm font-semibold transition focus-within:outline focus-within:outline-2 focus-within:outline-offset-2 focus-within:outline-accent', pie === v ? 'bg-ink text-bg' : 'text-ink-2 hover:text-ink')}>
            <input type="radio" name="pie-medido" value={v} checked={pie === v} onChange={() => setPie(v)} className="sr-only" data-testid={`pie-${v}`} />
            <span className={clsx('size-3.5 rounded-full border-2', pie === v ? 'border-bg bg-bg' : 'border-current')} aria-hidden />
            {l}
          </label>
        ))}
      </div>
      {!compacto && <p className="mt-2 max-w-md text-sm text-muted">La plantilla va siempre en el mismo lado del equipo; solo cambia de pie entre tomas.</p>}
    </fieldset>
  )
}
