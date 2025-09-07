#ifndef TILEMAPMANAGER_HPP
#define TILEMAPMANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

#include "Tilemap.hpp"

class TilemapManager {
    using TilemapPtr = std::unique_ptr<Tilemap>;

    std::vector<TilemapPtr> tilemaps;
    std::unordered_map<std::string, uint32_t> tilemapMap;
public:
    
};

#endif