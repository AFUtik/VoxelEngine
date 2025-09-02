#include "BlockRegistry.hpp"
#include <cstdint>
#include <iostream>

std::vector<BlockPrototype> BlockRegistry::registry;
std::unordered_map<std::string, uint32_t> BlockRegistry::registryMap;

void BlockRegistry::registerBlock(std::string name, BlockPrototype block) {
    block.id   = static_cast<uint16_t>(registry.size());
    block.name = name;
    block.states.push_back(block);

    registry.push_back(block);
    registryMap.emplace(block.name, block.id);
}

void BlockRegistry::registerState(std::string block_name, std::string state_name, Identifier state) {
    auto it = registryMap.find(block_name);
    if(it != registryMap.end()) {
        BlockPrototype& block = registry[it->second];
        block.stateMap[state_name] = block.states.size();
        block.states.push_back(state);
    } else {
        std::cerr <<"Block not found: " << block_name <<  std::endl;
    }
}