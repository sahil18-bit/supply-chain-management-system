import { X } from 'lucide-react';

export default function Modal({ title, children, onClose }) {
  return <div className="fixed inset-0 z-50 flex items-center justify-center bg-slate-900/25 p-4" onMouseDown={onClose}>
    <section className="max-h-[90vh] w-full max-w-lg overflow-y-auto rounded-2xl bg-white p-6 shadow-xl" onMouseDown={(event) => event.stopPropagation()}>
      <div className="mb-5 flex items-center justify-between"><h2 className="text-lg font-bold text-ink">{title}</h2><button className="rounded-lg p-1 text-slate-400 hover:bg-slate-100" onClick={onClose}><X size={20}/></button></div>
      {children}
    </section>
  </div>;
}
