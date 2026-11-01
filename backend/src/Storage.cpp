#include "Storage.h"
#include <fstream>

bool Storage::load(Graph& graph, std::string& warning) const {
    if (!std::filesystem::exists(path_)) return true;
    try { std::ifstream input(path_); nlohmann::json data; input >> data; graph.replace(data.value("locations", std::vector<Location>{}), data.value("routes", std::vector<Route>{}), data.value("nextLocationId", 1), data.value("nextRouteId", 1)); return true; }
    catch (const std::exception& e) { warning = std::string("Storage was not loaded: ") + e.what(); return false; }
}
bool Storage::save(const Graph& graph, std::string& error) const {
    try { std::filesystem::create_directories(path_.parent_path()); auto temporary=path_; temporary += ".tmp"; std::ofstream output(temporary); if(!output) throw std::runtime_error("could not open temporary storage file"); output << graph.snapshot().dump(2); output.close(); std::filesystem::copy_file(temporary, path_, std::filesystem::copy_options::overwrite_existing); std::filesystem::remove(temporary); return true; }
    catch (const std::exception& e) { error=e.what(); return false; }
}
