#ifndef BLOCKREGISTER_HPP
#define BLOCKREGISTER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "../graphics/texture/TextureRegion.hpp"

struct Identifier {
    uint16_t id = 0;
    std::string name;
};

struct BlockState : public Identifier {
    TextureRegion textureRegion;
};

struct BlockPrototype : public Identifier {
    std::vector<Identifier> states;
    std::unordered_map<std::string, uint32_t> stateMap;
};

class BlockRegistry {
    static std::vector<BlockPrototype> registry;
    static std::unordered_map<std::string, uint32_t> registryMap;
public:
    static void registerBlock(std::string name, BlockPrototype block);
    static void registerState(std::string block_name, std::string state_name, Identifier state);
};

#endif