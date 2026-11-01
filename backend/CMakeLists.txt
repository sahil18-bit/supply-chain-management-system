cmake_minimum_required(VERSION 3.20)
project(supply_chain_backend LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(supply_chain_api src/main.cpp src/Graph.cpp src/Storage.cpp)
target_include_directories(supply_chain_api PRIVATE include)
target_link_libraries(supply_chain_api PRIVATE Crow::Crow nlohmann_json::nlohmann_json)

enable_testing()
add_executable(graph_tests tests/GraphTests.cpp src/Graph.cpp)
target_include_directories(graph_tests PRIVATE include)
target_link_libraries(graph_tests PRIVATE nlohmann_json::nlohmann_json)
add_test(NAME graph_tests COMMAND graph_tests)
