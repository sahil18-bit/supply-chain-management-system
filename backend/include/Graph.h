#pragma once

#include "Models.h"
#include <optional>
#include <unordered_map>
#include <unordered_set>

enum class Metric { Cost, Distance, Time };

class Graph {
public:
    Graph() = default;
    ~Graph();
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    std::optional<Location> addLocation(const std::string& name, const std::string& type, std::string& error);
    bool updateLocation(int id, const std::string& name, const std::string& type, std::optional<bool> working, std::string& error);
    bool deleteLocation(int id, std::string& error);
    std::optional<Location> getLocation(int id) const;
    std::vector<Location> locations(const std::string& name = "", const std::string& type = "") const;

    std::optional<Route> addRoute(int from, int to, double cost, double distance, double time, bool bidirectional, std::string& error);
    bool updateRoute(int id, double cost, double distance, double time, std::optional<bool> working, std::optional<bool> bidirectional, std::string& error);
    bool deleteRoute(int id, std::string& error);
    std::optional<Route> getRoute(int id) const;
    std::vector<Route> routes() const;
    nlohmann::json connections(int locationId, std::string& error) const;

    RouteResult optimize(int source, int destination, Metric metric, int alternatives = 0) const;
    std::vector<RouteResult> alternatives(int source, int destination, Metric metric, int count) const;
    nlohmann::json failureImpact(int source) const;
    nlohmann::json analytics() const;
    nlohmann::json criticalInfrastructure() const;
    nlohmann::json graphJson() const;

    bool setLocationStatus(int id, bool working, std::string& error);
    bool setRouteStatus(int id, bool working, std::string& error);
    void replace(std::vector<Location> locations, std::vector<Route> routes, int nextLocationId, int nextRouteId);
    nlohmann::json snapshot() const;

private:
    struct Road { int to; int routeId; Road* next; };
    std::unordered_map<int, Location> locations_;
    std::unordered_map<int, Route> routes_;
    std::unordered_map<int, Road*> adjacency_;
    int nextLocationId_{1};
    int nextRouteId_{1};

    void clearAdjacency();
    void rebuildAdjacency();
    void addAdjacency(int from, int to, int routeId);
    RouteResult shortestPath(int source, int destination, Metric metric, const std::unordered_set<int>& excluded) const;
    static bool validWeight(double value);
    int componentCount() const;
    nlohmann::json components() const;
};
