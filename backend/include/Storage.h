#pragma once
#include "Graph.h"
#include <filesystem>

class Storage {
public:
    explicit Storage(std::filesystem::path path) : path_(std::move(path)) {}
    bool load(Graph& graph, std::string& warning) const;
    bool save(const Graph& graph, std::string& error) const;
private:
    std::filesystem::path path_;
};
