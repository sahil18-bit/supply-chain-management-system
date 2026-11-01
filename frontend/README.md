# Flowline frontend

React/Vite dashboard for the C++ Crow supply-chain backend. It uses live API responses only: no mock locations, routes, analytics, or activity records are embedded in the application.

## Run

Start the C++ backend on port `18080`, then run:

```powershell
cd frontend
npm install
npm run dev
```

Open `http://localhost:5173`. The default API base is `http://localhost:18080/api`; change it in Settings or set `VITE_API_URL` before building.

## Included views

- Cytoscape explorer with live, draggable locations/routes, status styling, selection, filters, reset view, and highlighted algorithm paths.
- Location and route CRUD, validation, confirmations, connection inspection, and success/error feedback.
- Dijkstra optimizer and alternative route presentation from `POST /api/route/optimize`.
- Live failure/restore simulation with backend connectivity comparison and impact lists.
- Tarjan critical-infrastructure view, analytics charts, and API connection settings.

Run `npm run build` to create a verified production bundle in `dist/`.
