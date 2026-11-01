# Supply Chain Management System Backend

This refactor retains the project’s core: dynamically allocated linked-list adjacency entries, Dijkstra with a priority queue, and BFS impact traversal. It removes the old fixed `MAX = 50` array and models routes as stable logical records so a two-way route can safely be edited, failed, restored, or deleted as one entity.

## Problems corrected from `final.cpp`

- `findPlace` was defined twice, preventing compilation.
- Menu condition `choice == 6` exited before optimal-route code could execute.
- Input and route weights were insufficiently validated; negative Dijkstra weights could be inserted.
- The fixed 50-place graph, raw global state, and no route IDs made deletion and restoration unreliable.
- Location deletion left inbound routes dangling; this backend deletes all associated logical routes transactionally.

## Build and run

Install Crow and nlohmann/json with your C++ package manager (for example vcpkg), then configure CMake:

```powershell
cmake -S backend -B build
cmake --build build
ctest --test-dir build --output-on-failure
.\build\supply_chain_api.exe
```

Pass a data-file path as the first argument to use a different JSON store. The server listens on `http://localhost:18080`; CORS permits the default Vite React origin `http://localhost:5173`.

## API

All responses are JSON and include `ok`; malformed bodies return 400, missing records 404, and storage failures 500.

| Endpoint | Purpose |
| --- | --- |
| `GET`, `POST /api/locations` | List/search (`?name=&type=`) or add locations |
| `PUT`, `DELETE /api/locations/{id}` | Edit/remove a location (removal also removes associated routes) |
| `GET /api/locations/{id}/connections` | Incoming and outgoing connections |
| `GET`, `POST /api/routes` | List or add a one-/two-way route |
| `PUT`, `DELETE /api/routes/{id}` | Edit/delete a route |
| `GET /api/graph` | Network snapshot |
| `POST /api/route/optimize` | Dijkstra optimization; body has `source`, `destination`, `metric` (`cost`, `distance`, `time`), optional `alternatives` |
| `GET /api/analytics` | Network counts plus weak connected-component membership |
| `POST /api/simulate/failure` | Body `{ "kind": "location|route", "id": 1, "source": 1 }`; returns connectivity before/after and, with `destination`/`metric`, a recalculated route |
| `POST /api/simulate/restore` | Body `{ "kind": "location|route", "id": 1 }` |
| `GET /api/critical-infrastructure` | Articulation points and bridges |

Example new route: `{"from":1,"to":2,"cost":30,"distance":12.5,"time":20,"bidirectional":true}`.

## Algorithms and complexity

Dijkstra ignores failed infrastructure and uses the selected non-negative cost, distance, or travel-time edge weight. With the binary priority queue it is `O((V + E) log V)` time and `O(V + E)` space. Alternatives are feasible paths made by temporarily excluding each primary-path route, then deduplicating candidates.

BFS reports locations reachable from an operational source in `O(V + E)` time and `O(V)` space. Connected-component counting is also `O(V + E)`.

Tarjan DFS computes discovery and low-link values in `O(V + E)` time and `O(V)` space. It intentionally uses *only operational two-way route records* to construct its undirected graph: a directed supply route is not mistakenly reported as an undirected bridge.

`backend/data/network.json` is loaded at startup and replaced after every mutation. Missing data starts an empty graph; malformed data logs a warning and leaves an empty, usable graph.
