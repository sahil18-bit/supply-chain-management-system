#include "Graph.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <iterator>
#include <limits>
#include <queue>
#include <set>
#include <stdexcept>

using json = nlohmann::json;

Graph::~Graph() { clearAdjacency(); }

void Graph::clearAdjacency() {
    for (auto& [_, head] : adjacency_) while (head) { Road* next = head->next; delete head; head = next; }
    adjacency_.clear();
}
void Graph::addAdjacency(int from, int to, int routeId) { adjacency_[from] = new Road{to, routeId, adjacency_[from]}; }
void Graph::rebuildAdjacency() {
    clearAdjacency();
    for (const auto& [id, route] : routes_) { addAdjacency(route.from, route.to, id); if (route.bidirectional) addAdjacency(route.to, route.from, id); }
}
bool Graph::validWeight(double value) { return std::isfinite(value) && value >= 0; }

std::optional<Location> Graph::addLocation(const std::string& name, const std::string& type, std::string& error) {
    if (name.empty() || type.empty()) { error = "name and type are required"; return std::nullopt; }
    for (const auto& [_, location] : locations_) if (location.name == name) { error = "a location with this name already exists"; return std::nullopt; }
    Location location{nextLocationId_++, name, type, true}; locations_.emplace(location.id, location); return location;
}
bool Graph::updateLocation(int id, const std::string& name, const std::string& type, std::optional<bool> working, std::string& error) {
    auto it = locations_.find(id); if (it == locations_.end()) { error = "location not found"; return false; }
    if (name.empty() || type.empty()) { error = "name and type are required"; return false; }
    for (const auto& [otherId, item] : locations_) if (otherId != id && item.name == name) { error = "a location with this name already exists"; return false; }
    it->second.name = name; it->second.type = type; if (working) it->second.working = *working; return true;
}
bool Graph::deleteLocation(int id, std::string& error) {
    if (!locations_.erase(id)) { error = "location not found"; return false; }
    for (auto it = routes_.begin(); it != routes_.end();) it = (it->second.from == id || it->second.to == id) ? routes_.erase(it) : std::next(it);
    rebuildAdjacency(); return true;
}
std::optional<Location> Graph::getLocation(int id) const { auto it = locations_.find(id); return it == locations_.end() ? std::nullopt : std::optional<Location>(it->second); }
std::vector<Location> Graph::locations(const std::string& name, const std::string& type) const {
    std::vector<Location> result; for (const auto& [_, item] : locations_) if ((name.empty() || item.name.find(name) != std::string::npos) && (type.empty() || item.type == type)) result.push_back(item);
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b){ return a.id < b.id; }); return result;
}
std::optional<Route> Graph::addRoute(int from, int to, double cost, double distance, double time, bool bidirectional, std::string& error) {
    if (!locations_.count(from) || !locations_.count(to)) { error = "source or destination location not found"; return std::nullopt; }
    if (from == to) { error = "a route cannot connect a location to itself"; return std::nullopt; }
    if (!validWeight(cost) || !validWeight(distance) || !validWeight(time)) { error = "cost, distance, and time must be finite non-negative values"; return std::nullopt; }
    for (const auto& [_, r] : routes_) {
        // Do not permit two logical records that would create the same directed arc.
        const bool sameDirection = r.from == from && r.to == to;
        const bool reverseDirectionShared = (r.bidirectional || bidirectional) && r.from == to && r.to == from;
        if (sameDirection || reverseDirectionShared) { error = "an equivalent or overlapping route already exists"; return std::nullopt; }
    }
    Route route{nextRouteId_++, from, to, cost, distance, time, true, bidirectional}; routes_.emplace(route.id, route); rebuildAdjacency(); return route;
}
bool Graph::updateRoute(int id, double cost, double distance, double time, std::optional<bool> working, std::optional<bool> bidirectional, std::string& error) {
    auto it = routes_.find(id); if (it == routes_.end()) { error = "route not found"; return false; }
    if (!validWeight(cost) || !validWeight(distance) || !validWeight(time)) { error = "cost, distance, and time must be finite non-negative values"; return false; }
    const bool proposedBidirectional = bidirectional.value_or(it->second.bidirectional);
    for (const auto& [otherId, other] : routes_) if (otherId != id) {
        const bool sameDirection = other.from == it->second.from && other.to == it->second.to;
        const bool reverseDirectionShared = (other.bidirectional || proposedBidirectional) && other.from == it->second.to && other.to == it->second.from;
        if (sameDirection || reverseDirectionShared) { error = "route update would overlap an existing route"; return false; }
    }
    it->second.cost = cost; it->second.distance = distance; it->second.time = time; if (working) it->second.working = *working; if (bidirectional) it->second.bidirectional = *bidirectional; rebuildAdjacency(); return true;
}
bool Graph::deleteRoute(int id, std::string& error) { if (!routes_.erase(id)) { error = "route not found"; return false; } rebuildAdjacency(); return true; }
std::optional<Route> Graph::getRoute(int id) const { auto it = routes_.find(id); return it == routes_.end() ? std::nullopt : std::optional<Route>(it->second); }
std::vector<Route> Graph::routes() const { std::vector<Route> result; for (auto& [_, r] : routes_) result.push_back(r); std::sort(result.begin(), result.end(), [](const auto& a, const auto& b){return a.id < b.id;}); return result; }
bool Graph::setLocationStatus(int id, bool working, std::string& error) { auto it = locations_.find(id); if (it == locations_.end()) { error = "location not found"; return false; } it->second.working = working; return true; }
bool Graph::setRouteStatus(int id, bool working, std::string& error) { auto it = routes_.find(id); if (it == routes_.end()) { error = "route not found"; return false; } it->second.working = working; return true; }

json Graph::connections(int locationId, std::string& error) const {
    if (!locations_.count(locationId)) { error = "location not found"; return {}; }
    json incoming = json::array(), outgoing = json::array();
    for (const auto& [_, r] : routes_) { if (r.from == locationId) outgoing.push_back(r); if (r.to == locationId) incoming.push_back(r); if (r.bidirectional && r.from == locationId) incoming.push_back(r); if (r.bidirectional && r.to == locationId) outgoing.push_back(r); }
    return {{"locationId", locationId}, {"incoming", incoming}, {"outgoing", outgoing}};
}

RouteResult Graph::shortestPath(int source, int destination, Metric metric, const std::unordered_set<int>& excluded) const {
    RouteResult out;
    if (!locations_.count(source) || !locations_.count(destination)) { out.message = "source or destination location not found"; return out; }
    if (!locations_.at(source).working || !locations_.at(destination).working) { out.message = "source or destination location is failed"; return out; }
    using State = std::pair<double, int>; std::priority_queue<State, std::vector<State>, std::greater<State>> pq;
    std::unordered_map<int, double> distances; std::unordered_map<int, std::pair<int, int>> parent;
    for (auto& [id, _] : locations_) distances[id] = std::numeric_limits<double>::infinity(); distances[source] = 0; pq.push({0, source});
    auto weight = [metric](const Route& r) { return metric == Metric::Cost ? r.cost : (metric == Metric::Distance ? r.distance : r.time); };
    while (!pq.empty()) { auto [current, u] = pq.top(); pq.pop(); if (current != distances[u]) continue; if (u == destination) break;
        auto found = adjacency_.find(u); if (found == adjacency_.end()) continue;
        for (Road* edge = found->second; edge; edge = edge->next) { const auto& r = routes_.at(edge->routeId); const auto& v = locations_.at(edge->to);
            if (excluded.count(r.id) || !r.working || !v.working) continue; const double candidate = current + weight(r);
            if (candidate < distances[edge->to]) { distances[edge->to] = candidate; parent[edge->to] = {u, r.id}; pq.push({candidate, edge->to}); }
        }
    }
    if (!std::isfinite(distances[destination])) { out.message = "no operational route exists"; return out; }
    out.found = true; out.message = "route found"; out.optimizedValue = distances[destination]; int at = destination; out.locationIds.push_back(at);
    while (at != source) { auto [previous, routeId] = parent.at(at); const auto& r = routes_.at(routeId); out.edges.push_back({routeId, previous, at, r.cost, r.distance, r.time}); out.totalCost += r.cost; out.totalDistance += r.distance; out.totalTime += r.time; at = previous; out.locationIds.push_back(at); }
    std::reverse(out.locationIds.begin(), out.locationIds.end()); std::reverse(out.edges.begin(), out.edges.end()); return out;
}
RouteResult Graph::optimize(int source, int destination, Metric metric, int) const { return shortestPath(source, destination, metric, {}); }
std::vector<RouteResult> Graph::alternatives(int source, int destination, Metric metric, int count) const {
    std::vector<RouteResult> result; RouteResult primary = optimize(source, destination, metric); if (!primary.found || count <= 0) return result;
    std::set<std::vector<int>> uniquePaths; uniquePaths.insert(primary.locationIds);
    for (const auto& edge : primary.edges) { auto candidate = shortestPath(source, destination, metric, {edge.routeId}); if (candidate.found && uniquePaths.insert(candidate.locationIds).second) result.push_back(candidate); }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b){ return a.optimizedValue < b.optimizedValue; }); if ((int)result.size() > count) result.resize(count); return result;
}

json Graph::failureImpact(int source) const {
    if (!locations_.count(source) || !locations_.at(source).working) return {{"error", "source not found or is failed"}};
    std::unordered_set<int> seen{source}; std::queue<int> q; q.push(source); while (!q.empty()) { int u = q.front(); q.pop(); auto found = adjacency_.find(u); if (found == adjacency_.end()) continue; for (Road* e = found->second; e; e = e->next) { const auto& r = routes_.at(e->routeId); if (r.working && locations_.at(e->to).working && seen.insert(e->to).second) q.push(e->to); } }
    json reachable = json::array(), unreachable = json::array(), failed = json::array(); for (const auto& [id, l] : locations_) { if (!l.working) failed.push_back(l); else if (seen.count(id)) reachable.push_back(l); else unreachable.push_back(l); }
    return {{"sourceId", source}, {"reachable", reachable}, {"unreachableOperational", unreachable}, {"failedLocations", failed}};
}
json Graph::components() const {
    // Connectivity is weak connectivity: each usable directed supply link joins
    // the two facilities for resilience analysis, regardless of its direction.
    std::unordered_set<int> seen; std::unordered_map<int, std::vector<int>> undirected;
    for (auto& [id, location] : locations_) if (location.working) undirected[id];
    for (auto& [_, route] : routes_) if (route.working && locations_.at(route.from).working && locations_.at(route.to).working) { undirected[route.from].push_back(route.to); undirected[route.to].push_back(route.from); }
    json result=json::array();
    for (auto& [start, _] : undirected) if (seen.insert(start).second) {
        json part=json::array(); std::queue<int> q; q.push(start);
        while(!q.empty()) { int u=q.front(); q.pop(); part.push_back(locations_.at(u)); for(int v:undirected[u]) if(seen.insert(v).second) q.push(v); }
        result.push_back(part);
    }
    return result;
}
int Graph::componentCount() const { return static_cast<int>(components().size()); }
json Graph::analytics() const { int operationalLocations=0, operationalRoutes=0; for(auto& [_,l]:locations_) operationalLocations+=l.working; for(auto& [_,r]:routes_) operationalRoutes+=r.working; const json groups=components(); const int componentCount=static_cast<int>(groups.size()); return {{"totalLocations",locations_.size()},{"totalRoutes",routes_.size()},{"operationalLocations",operationalLocations},{"failedLocations",(int)locations_.size()-operationalLocations},{"operationalRoutes",operationalRoutes},{"failedRoutes",(int)routes_.size()-operationalRoutes},{"connectedComponents",componentCount},{"components",groups},{"isConnected",operationalLocations <= 1 || componentCount == 1}}; }

json Graph::criticalInfrastructure() const {
    // Tarjan runs on only genuinely bidirectional routes. Directed arcs are not undirected roads and cannot be bridges here.
    std::unordered_map<int, std::vector<std::pair<int,int>>> g; for(auto& [id,l]:locations_) if(l.working) g[id]; for(auto& [id,r]:routes_) if(r.working && r.bidirectional && locations_.at(r.from).working && locations_.at(r.to).working) { g[r.from].push_back({r.to,id}); g[r.to].push_back({r.from,id}); }
    std::unordered_map<int,int> disc, low; std::unordered_set<int> points; std::vector<int> bridges; int clock=0;
    std::function<void(int,int)> dfs = [&](int u,int parentRoute) { disc[u]=low[u]=++clock; int children=0; for(auto [v,rid]:g[u]) { if(rid==parentRoute) continue; if(!disc[v]) { ++children; dfs(v,rid); low[u]=std::min(low[u],low[v]); if(parentRoute!=-1 && low[v]>=disc[u]) points.insert(u); if(low[v]>disc[u]) bridges.push_back(rid); } else low[u]=std::min(low[u],disc[v]); } if(parentRoute==-1 && children>1) points.insert(u); };
    for(auto& [id,_]:g) if(!disc[id]) dfs(id,-1); json pointValues=json::array(), bridgeValues=json::array(); for(int id:points) pointValues.push_back(locations_.at(id)); for(int id:bridges) bridgeValues.push_back(routes_.at(id)); return {{"articulationPoints",pointValues},{"bridges",bridgeValues},{"note","Analysis includes operational two-way routes only; directed routes are excluded from undirected bridge semantics."},{"timeComplexity","O(V + E)"}};
}
json Graph::graphJson() const { return {{"locations", locations()}, {"routes", routes()}}; }
json Graph::snapshot() const { return {{"version",1},{"nextLocationId",nextLocationId_},{"nextRouteId",nextRouteId_},{"locations",locations()},{"routes",routes()}}; }
void Graph::replace(std::vector<Location> locations, std::vector<Route> routes, int nextLocationId, int nextRouteId) {
    std::unordered_set<int> locationIds, routeIds; std::unordered_set<std::string> names; int maxLocationId=0, maxRouteId=0;
    for (const auto& location : locations) {
        if (location.id <= 0 || location.name.empty() || location.type.empty() || !locationIds.insert(location.id).second || !names.insert(location.name).second) throw std::invalid_argument("invalid or duplicate location in storage");
        maxLocationId = std::max(maxLocationId, location.id);
    }
    for (const auto& route : routes) {
        if (route.id <= 0 || !routeIds.insert(route.id).second || !locationIds.count(route.from) || !locationIds.count(route.to) || route.from == route.to || !validWeight(route.cost) || !validWeight(route.distance) || !validWeight(route.time)) throw std::invalid_argument("invalid route in storage");
        maxRouteId = std::max(maxRouteId, route.id);
    }
    locations_.clear(); routes_.clear();
    for(auto& location:locations) locations_.emplace(location.id,location);
    for(auto& route:routes) routes_.emplace(route.id,route);
    nextLocationId_=std::max(maxLocationId + 1, std::max(1,nextLocationId)); nextRouteId_=std::max(maxRouteId + 1, std::max(1,nextRouteId)); rebuildAdjacency();
}
