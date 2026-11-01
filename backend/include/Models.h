#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Location {
    int id{};
    std::string name;
    std::string type;
    bool working{true};
};

// A Route is one logical connection.  When bidirectional is true it creates
// two linked-list adjacency entries, but remains one route for the API.
struct Route {
    int id{};
    int from{};
    int to{};
    double cost{};
    double distance{};
    double time{};
    bool working{true};
    bool bidirectional{false};
};

struct PathEdge {
    int routeId{};
    int from{};
    int to{};
    double cost{};
    double distance{};
    double time{};
};

struct RouteResult {
    bool found{false};
    std::string message;
    std::vector<int> locationIds;
    std::vector<PathEdge> edges;
    double totalCost{};
    double totalDistance{};
    double totalTime{};
    double optimizedValue{};
};

inline void to_json(nlohmann::json& j, const Location& value) {
    j = {{"id", value.id}, {"name", value.name}, {"type", value.type}, {"working", value.working}};
}
inline void from_json(const nlohmann::json& j, Location& value) {
    j.at("id").get_to(value.id); j.at("name").get_to(value.name); j.at("type").get_to(value.type);
    value.working = j.value("working", true);
}
inline void to_json(nlohmann::json& j, const Route& value) {
    j = {{"id", value.id}, {"from", value.from}, {"to", value.to}, {"cost", value.cost},
         {"distance", value.distance}, {"time", value.time}, {"working", value.working},
         {"bidirectional", value.bidirectional}};
}
inline void from_json(const nlohmann::json& j, Route& value) {
    j.at("id").get_to(value.id); j.at("from").get_to(value.from); j.at("to").get_to(value.to);
    j.at("cost").get_to(value.cost); j.at("distance").get_to(value.distance); j.at("time").get_to(value.time);
    value.working = j.value("working", true); value.bidirectional = j.value("bidirectional", false);
}
