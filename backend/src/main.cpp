#include "Graph.h"
#include "Storage.h"
#include <crow.h>
#include <filesystem>
#include <iostream>

using json = nlohmann::json;

// Kept in PathEdge's (global) namespace so nlohmann/json finds it by ADL.
void to_json(json& j, const PathEdge& edge) {
    j = {{"routeId", edge.routeId}, {"from", edge.from}, {"to", edge.to},
         {"cost", edge.cost}, {"distance", edge.distance}, {"time", edge.time}};
}

namespace {
crow::response response(int status, const json& body) {
    crow::response out(status, body.dump());
    out.set_header("Content-Type", "application/json");
    out.set_header("Access-Control-Allow-Origin", "http://localhost:5173");
    out.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    out.set_header("Access-Control-Allow-Headers", "Content-Type");
    return out;
}
crow::response error(int status, const std::string& message) { return response(status, {{"ok", false}, {"error", message}}); }
json parseBody(const crow::request& request) { return json::parse(request.body); }
Metric metricFrom(const std::string& value) {
    if (value == "cost") return Metric::Cost;
    if (value == "distance") return Metric::Distance;
    if (value == "time") return Metric::Time;
    throw std::invalid_argument("metric must be cost, distance, or time");
}
json resultJson(const RouteResult& result) {
    return {{"found", result.found}, {"message", result.message}, {"locationIds", result.locationIds},
            {"edges", result.edges}, {"totals", {{"cost", result.totalCost}, {"distance", result.totalDistance}, {"time", result.totalTime}}},
            {"optimizedValue", result.optimizedValue}};
}
}

int main(int argc, char** argv) {
    const std::filesystem::path dataFile = argc > 1 ? argv[1] : "backend/data/network.json";
    Graph graph;
    Storage storage(dataFile);
    std::string loadWarning;
    if (!storage.load(graph, loadWarning)) std::cerr << loadWarning << '\n';
    auto persist = [&storage, &graph](std::string& failure) { return storage.save(graph, failure); };

    crow::SimpleApp app;
    CROW_ROUTE(app, "/api/<path>").methods(crow::HTTPMethod::Options)([](std::string) { return response(204, json::object()); });
    CROW_ROUTE(app, "/api").methods(crow::HTTPMethod::Options)([] { return response(204, json::object()); });

    CROW_ROUTE(app, "/api/locations").methods(crow::HTTPMethod::Get)([&](const crow::request& req) {
        const char* name = req.url_params.get("name"); const char* type = req.url_params.get("type");
        return response(200, {{"ok", true}, {"locations", graph.locations(name ? name : "", type ? type : "")}});
    });
    CROW_ROUTE(app, "/api/locations").methods(crow::HTTPMethod::Post)([&](const crow::request& req) {
        try { auto body=parseBody(req); std::string e; auto item=graph.addLocation(body.value("name", ""), body.value("type", ""), e); if(!item) return error(400,e); if(!persist(e)) return error(500,e); return response(201,{{"ok",true},{"location",*item}}); } catch(const std::exception& e) { return error(400,e.what()); }
    });
    CROW_ROUTE(app, "/api/locations/<int>").methods(crow::HTTPMethod::Put)([&](const crow::request& req,int id) {
        try { auto current=graph.getLocation(id); if(!current) return error(404,"location not found"); auto body=parseBody(req); std::string e; if(!graph.updateLocation(id,body.value("name",current->name),body.value("type",current->type),body.contains("working") ? std::optional<bool>(body.at("working").get<bool>()) : std::nullopt,e)) return error(400,e); if(!persist(e)) return error(500,e); return response(200,{{"ok",true},{"location",*graph.getLocation(id)}}); } catch(const std::exception& e) { return error(400,e.what()); }
    });
    CROW_ROUTE(app, "/api/locations/<int>").methods(crow::HTTPMethod::Delete)([&](int id) { std::string e; if(!graph.deleteLocation(id,e)) return error(404,e); if(!persist(e)) return error(500,e); return response(200,{{"ok",true},{"message","location and its associated routes deleted"}}); });
    CROW_ROUTE(app, "/api/locations/<int>/connections").methods(crow::HTTPMethod::Get)([&](int id) { std::string e; auto value=graph.connections(id,e); return e.empty() ? response(200,{{"ok",true},{"connections",value}}) : error(404,e); });

    CROW_ROUTE(app, "/api/routes").methods(crow::HTTPMethod::Get)([&] { return response(200,{{"ok",true},{"routes",graph.routes()}}); });
    CROW_ROUTE(app, "/api/routes").methods(crow::HTTPMethod::Post)([&](const crow::request& req) {
        try { auto b=parseBody(req); std::string e; auto item=graph.addRoute(b.at("from").get<int>(),b.at("to").get<int>(),b.at("cost").get<double>(),b.at("distance").get<double>(),b.at("time").get<double>(),b.value("bidirectional",false),e); if(!item) return error(400,e); if(!persist(e)) return error(500,e); return response(201,{{"ok",true},{"route",*item}}); } catch(const std::exception& e) { return error(400,e.what()); }
    });
    CROW_ROUTE(app, "/api/routes/<int>").methods(crow::HTTPMethod::Put)([&](const crow::request& req,int id) {
        try { auto current=graph.getRoute(id); if(!current) return error(404,"route not found"); auto b=parseBody(req); std::string e; auto optionalBool=[&](const char* key)->std::optional<bool>{return b.contains(key)?std::optional<bool>(b.at(key).get<bool>()):std::nullopt;}; if(!graph.updateRoute(id,b.value("cost",current->cost),b.value("distance",current->distance),b.value("time",current->time),optionalBool("working"),optionalBool("bidirectional"),e)) return error(400,e); if(!persist(e)) return error(500,e); return response(200,{{"ok",true},{"route",*graph.getRoute(id)}}); } catch(const std::exception& e) { return error(400,e.what()); }
    });
    CROW_ROUTE(app, "/api/routes/<int>").methods(crow::HTTPMethod::Delete)([&](int id) { std::string e; if(!graph.deleteRoute(id,e)) return error(404,e); if(!persist(e)) return error(500,e); return response(200,{{"ok",true},{"message","route deleted"}}); });

    CROW_ROUTE(app, "/api/graph").methods(crow::HTTPMethod::Get)([&] { return response(200,{{"ok",true},{"graph",graph.graphJson()}}); });
    CROW_ROUTE(app, "/api/route/optimize").methods(crow::HTTPMethod::Post)([&](const crow::request& req) {
        try { auto b=parseBody(req); int source=b.at("source").get<int>(), destination=b.at("destination").get<int>(); Metric metric=metricFrom(b.value("metric","cost")); auto primary=graph.optimize(source,destination,metric); json payload={{"ok",primary.found},{"route",resultJson(primary)}}; int desired=b.value("alternatives",0); if(desired>0) { json values=json::array(); for(auto& route:graph.alternatives(source,destination,metric,desired)) values.push_back(resultJson(route)); payload["alternatives"]=values; } return response(primary.found?200:404,payload); } catch(const std::exception& e) { return error(400,e.what()); }
    });
    CROW_ROUTE(app, "/api/analytics").methods(crow::HTTPMethod::Get)([&] { return response(200,{{"ok",true},{"analytics",graph.analytics()}}); });
    CROW_ROUTE(app, "/api/critical-infrastructure").methods(crow::HTTPMethod::Get)([&] { return response(200,{{"ok",true},{"criticalInfrastructure",graph.criticalInfrastructure()}}); });
    CROW_ROUTE(app, "/api/simulate/failure").methods(crow::HTTPMethod::Post)([&](const crow::request& req) {
        try { auto b=parseBody(req); json before=graph.analytics(); std::string e; const std::string kind=b.at("kind").get<std::string>(); const int id=b.at("id").get<int>(); bool success = kind=="location" ? graph.setLocationStatus(id,false,e) : kind=="route" ? graph.setRouteStatus(id,false,e) : false; if(kind!="location" && kind!="route") e="kind must be location or route"; if(!success) return error(e=="location not found"||e=="route not found"?404:400,e); if(!persist(e)) return error(500,e); json value={{"ok",true},{"connectivityBefore",before},{"connectivityAfter",graph.analytics()}}; if(b.contains("source")) value["impact"]=graph.failureImpact(b.at("source").get<int>()); if(b.contains("source") && b.contains("destination")) value["recalculatedRoute"]=resultJson(graph.optimize(b.at("source").get<int>(),b.at("destination").get<int>(),metricFrom(b.value("metric","cost")))); return response(200,value); } catch(const std::exception& e) { return error(400,e.what()); }
    });
    CROW_ROUTE(app, "/api/simulate/restore").methods(crow::HTTPMethod::Post)([&](const crow::request& req) {
        try { auto b=parseBody(req); json before=graph.analytics(); std::string e; const std::string kind=b.at("kind").get<std::string>(); const int id=b.at("id").get<int>(); bool success=kind=="location"?graph.setLocationStatus(id,true,e):kind=="route"?graph.setRouteStatus(id,true,e):false; if(kind!="location"&&kind!="route") e="kind must be location or route"; if(!success) return error(e=="location not found"||e=="route not found"?404:400,e); if(!persist(e)) return error(500,e); return response(200,{{"ok",true},{"connectivityBefore",before},{"connectivityAfter",graph.analytics()}}); } catch(const std::exception& e) { return error(400,e.what()); }
    });
    std::cout << "Supply-chain API listening on http://localhost:18080\n";
    app.port(18080).multithreaded().run();
}
