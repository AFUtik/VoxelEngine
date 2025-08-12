#include "BlockRegistry.hpp"

std::vector<BlockRegistry> BlockRegistry::registry;
std::unordered_map<std::string, uint32_t> BlockRegistry::umap;

void BlockRegistry::registerBlock(std::string name, BlockRegistry block) {
    
}