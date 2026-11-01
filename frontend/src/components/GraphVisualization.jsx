import { useEffect, useRef } from 'react';
import cytoscape from 'cytoscape';
import { Maximize2 } from 'lucide-react';

const typeColor = { factory: '#4f9eea', warehouse: '#0f9c9b', shop: '#7bbf8e', store: '#7bbf8e' };
export default function GraphVisualization({ graph, selectedPath = [], onSelect, height = 380 }) {
  const holder = useRef(null); const cyRef = useRef(null);
  useEffect(() => { if (!holder.current) return; const pathRoutes = new Set(selectedPath.filter((edge) => edge.routeId).map((edge) => `r${edge.routeId}`)); const pathLocations = new Set(selectedPath.filter((item) => item.locationId).map((item) => `l${item.locationId}`));
    const elements = [
      ...(graph?.locations || []).map((location) => ({ data: { id: `l${location.id}`, kind: 'location', location, label: location.name }, classes: `${location.working ? 'working' : 'failed'} ${location.type.toLowerCase()} ${pathLocations.has(`l${location.id}`) ? 'focus' : ''}` })),
      ...(graph?.routes || []).map((route) => ({ data: { id: `r${route.id}`, kind: 'route', route, source: `l${route.from}`, target: `l${route.to}`, label: route.bidirectional ? '↔' : '→' }, classes: `${route.working ? 'working' : 'failed'} ${pathRoutes.has(`r${route.id}`) ? 'path' : ''} ${route.bidirectional ? 'two-way' : ''}` })),
    ];
    cyRef.current?.destroy(); const cy = cytoscape({ container: holder.current, elements, layout: { name: 'cose', animate: false, padding: 28 }, style: [
      { selector: 'node', style: { label: 'data(label)', color: '#385168', 'font-size': 11, 'font-weight': 600, 'text-valign': 'bottom', 'text-margin-y': 7, width: 34, height: 34, 'background-color': '#7bbf8e', 'border-width': 3, 'border-color': '#e6f8f2' } },
      { selector: 'node.factory', style: { 'background-color': typeColor.factory } }, { selector: 'node.warehouse', style: { 'background-color': typeColor.warehouse } }, { selector: 'node.failed', style: { 'background-color': '#e45d67', 'border-color': '#ffe4e6' } }, { selector: 'node.focus', style: { width: 46, height: 46, 'border-width': 5, 'border-color': '#f3bd42', 'z-index': 10 } },
      { selector: 'edge', style: { width: 2, 'line-color': '#a9c6d7', 'target-arrow-color': '#a9c6d7', 'target-arrow-shape': 'triangle', 'curve-style': 'bezier', label: 'data(label)', color: '#71879a', 'font-size': 12 } }, { selector: 'edge.two-way', style: { 'source-arrow-color': '#a9c6d7', 'source-arrow-shape': 'triangle' } }, { selector: 'edge.failed', style: { 'line-color': '#e45d67', 'target-arrow-color': '#e45d67', 'line-style': 'dashed', opacity: 0.65 } }, { selector: 'edge.path', style: { width: 5, 'line-color': '#0f9c9b', 'target-arrow-color': '#0f9c9b', 'source-arrow-color': '#0f9c9b', 'z-index': 10 } },
    ] });
    cy.on('tap', 'node, edge', (event) => onSelect?.(event.target.data())); cyRef.current = cy; return () => cy.destroy();
  }, [graph, selectedPath, onSelect]);
  return <div className="relative overflow-hidden rounded-2xl border border-slate-100 bg-[#fcfeff]" style={{ height }}><div ref={holder} className="h-full w-full"/><button onClick={() => cyRef.current?.fit(undefined, 28)} className="absolute right-3 top-3 rounded-lg border border-slate-200 bg-white p-2 text-slate-500 shadow-sm hover:text-teal" title="Reset view"><Maximize2 size={17}/></button></div>;
}
