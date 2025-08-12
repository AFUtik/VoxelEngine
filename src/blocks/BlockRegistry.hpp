#ifndef BLOCKREGISTER_HPP
#define BLOCKREGISTER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

struct TextureRegion;

struct BlockPrototype {
	uint16_t id;
	std::string name;

	virtual bool isOpaque() = 0;
};

class BlockRegistry {
    static std::vector<BlockRegistry> registry;
    static std::unordered_map<std::string, uint32_t> umap;
public:
    static void registerBlock(std::string name, BlockRegistry block);
};

#endif