const baseUrl = import.meta.env.VITE_API_URL || localStorage.getItem('supply-api-url') || 'http://localhost:18080/api';

async function request(path, options = {}) {
  const response = await fetch(`${baseUrl}${path}`, {
    ...options,
    headers: { 'Content-Type': 'application/json', ...(options.headers || {}) },
  });
  const body = await response.json().catch(() => ({}));
  if (!response.ok || body.ok === false) throw new Error(body.error || body.route?.message || `Request failed (${response.status})`);
  return body;
}
const json = (method, value) => ({ method, body: JSON.stringify(value) });

export const api = {
  graph: () => request('/graph'), analytics: () => request('/analytics'), critical: () => request('/critical-infrastructure'),
  locations: {
    list: (filters = {}) => request(`/locations?${new URLSearchParams(Object.entries(filters).filter(([, value]) => value)).toString()}`),
    create: (value) => request('/locations', json('POST', value)), update: (id, value) => request(`/locations/${id}`, json('PUT', value)),
    remove: (id) => request(`/locations/${id}`, { method: 'DELETE' }), connections: (id) => request(`/locations/${id}/connections`),
  },
  routes: {
    list: () => request('/routes'), create: (value) => request('/routes', json('POST', value)),
    update: (id, value) => request(`/routes/${id}`, json('PUT', value)), remove: (id) => request(`/routes/${id}`, { method: 'DELETE' }),
  },
  optimize: (value) => request('/route/optimize', json('POST', value)),
  failure: (value) => request('/simulate/failure', json('POST', value)),
  restore: (value) => request('/simulate/restore', json('POST', value)),
};
