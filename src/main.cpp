// builtin
#include <iostream>
#include <optional>
#include <utility>
#include <algorithm>

// local
#include "map.hpp"
#include "pathfinder.hpp"



void display_map(Map const& map, std::optional<std::pair<IdxVec2, IdxVec2>> const targets = std::nullopt, std::optional<std::vector<IdxVec2>> const& path = std::nullopt) {

    auto const map_size = map.get_size();

    std::cout << "map size: " << map_size.x << "x" << map_size.y << "\n";
    for (int32_t y = 0; y < map_size.y; y++) {
        std::cout << '|';
        for (int32_t x = 0; x < map_size.x; x++) {

            if (targets.has_value()) {
                if (targets.value().first == IdxVec2{x, y}) {
                    std::cout << 'O';
                    continue;
                }
                if (targets.value().second == IdxVec2{x, y}) {
                    std::cout << 'D';
                    continue;
                }
            }

            if (path.has_value()) {
                if (std::find(path.value().begin(), path.value().end(), IdxVec2{x, y}) != path.value().end()) {
                    std::cout << '*';
                    continue;
                }
            }
            std::cout << (map.is_walkable({x, y}) ? '_' : '#');
        }
        std::cout << "|\n";
    }
    std::cout << std::flush;
}

bool validate_point(Map const& map, IdxVec2 const& point) {
    return map.has_tile(point) && map.is_walkable(point);
}


int main() {

    /* ---------------------------------- setup --------------------------------- */
    
    std::cout << "loading map from file 'map.txt'" << std::endl;
    auto map = Map::from_file("map.txt");
    display_map(map);
    std::cout << std::endl;

    // receive origin point
    IdxVec2 origin = {-1, -1};
    std::cout << "Enter the x and y coordinates of the origin point: ";
    std::cin >> origin.x >> origin.y;
    if (!validate_point(map, origin)) {
        std::cout << "Invalid origin point" << std::endl;
        return 1;
    }

    // receive destination point
    IdxVec2 destination = {-1, -1};
    std::cout << "Enter the x and y coordinates of the destination point: ";
    std::cin >> destination.x >> destination.y;
    if (!validate_point(map, destination)) {
        std::cout << "Invalid destination point" << std::endl;
        return 1;
    }

    // the origin and destination points shall not be the same
    if (origin == destination) {
        std::cout << "Origin and destination points cannot be the same" << std::endl;
        return 1;
    }

    // display map with origin and destination points
    std::cout << std::endl;
    display_map(map, std::make_pair(origin, destination));
    std::cout << std::endl;

    size_t run_count;
    std::cout << "Enter the number of times to run the pathfinding algorithms: ";
    std::cin >> run_count;
    std::cout << std::endl;


    /* ------------------------------- benchmarks ------------------------------- */

    std::cout << "------------------------ benchmarks ------------------------" << std::endl;

    std::vector<std::pair<std::string, std::function<std::optional<Path>(IdxVec2, IdxVec2, std::function<bool(IdxVec2)>, std::function<bool(IdxVec2)>)>>> pathfinders = {
        {"A*", a_star},
        {"Djikstra", djikstra}
    };

    for (auto const pathfinder: pathfinders) {

        std::cout << "benchmarking " << pathfinder.first << " algorithm" << std::endl;

        uint64_t total_time_us = 0;
        std::optional<Path> path = std::nullopt;

        for (size_t i = 0; i < run_count; i++) {
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result_path = pathfinder.second(origin, destination, [&map](IdxVec2 point) { return map.is_walkable(point); }, [&map](IdxVec2 point) { return map.has_tile(point); });
            auto end = std::chrono::high_resolution_clock::now();
            
            if (path.has_value()) {
                if (path.value() != result_path)
                    std::cout << pathfinder.first << " path mismatch" << std::endl;
            }
            else
                path = result_path;

            total_time_us += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }

        if (path.has_value()) {

            assert(is_path_valid(path.value()) && "invalid path");
            std::cout << "path found: ";
            for (auto const& point: path.value())
                std::cout << "(" << point.x << ", " << point.y << ") ";
            std::cout << std::endl;

            display_map(map, std::make_pair(origin, destination), path.value());

            std::cout << std::endl;
        }

        std::cout << "average time: " << total_time_us / run_count << " us" << std::endl;

        std::cout << std::endl;
    }
}
