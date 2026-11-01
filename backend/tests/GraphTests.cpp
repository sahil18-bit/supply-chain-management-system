#include "Graph.h"
#include <cassert>
#include <iostream>

int main() {
    Graph graph; std::string error;
    auto a=graph.addLocation("Factory","factory",error); auto b=graph.addLocation("Warehouse","warehouse",error); auto c=graph.addLocation("Store","store",error); assert(a&&b&&c);
    assert(!graph.addLocation("Factory","other",error)); // duplicate names are rejected
    auto ab=graph.addRoute(a->id,b->id,4,10,8,true,error); auto bc=graph.addRoute(b->id,c->id,3,5,4,true,error); auto ac=graph.addRoute(a->id,c->id,20,1,2,false,error); assert(ab&&bc&&ac);
    assert(!graph.addRoute(a->id,c->id,-1,1,1,false,error)); // Dijkstra-safe weights
    auto cheapest=graph.optimize(a->id,c->id,Metric::Cost); assert(cheapest.found && cheapest.totalCost==7 && cheapest.locationIds.size()==3);
    auto shortest=graph.optimize(a->id,c->id,Metric::Distance); assert(shortest.found && shortest.totalDistance==1);
    assert(graph.criticalInfrastructure()["articulationPoints"].size()==1); // Warehouse is a cut point for two-way roads
    assert(graph.setRouteStatus(bc->id,false,error)); assert(!graph.optimize(a->id,c->id,Metric::Cost).found || graph.optimize(a->id,c->id,Metric::Cost).totalCost==20);
    assert(graph.setRouteStatus(bc->id,true,error)); assert(graph.deleteLocation(b->id,error)); assert(graph.routes().size()==1); // associated routes removed safely
    std::cout << "All graph tests passed\n";
}
