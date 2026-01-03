#ifndef BLOCKREGISTER_HPP
#define BLOCKREGISTER_HPP

#include "AbstractBlock.hpp"
#include <cstdint>
#include <unordered_map>

constexpr int MAX_BLOCKS = 4096;

struct FaceInfo {
    uint16_t texture;
};

struct BlockModel {
    uint16_t id;
    bool opaque;
    bool fullCube;
    FaceInfo faces[6];
};

class BlockModelRegistry {
    static inline BlockModel models[MAX_BLOCKS];
public:
    static inline const BlockModel& get(uint16_t id) {
        return models[id];
    }

    static inline void registerBlockModel(BlockModel& model) {
        models[model.id] = model;
    }
};

class BlockRegistry {
    static inline AbstractBlock blocks[MAX_BLOCKS];
    static inline std::unordered_map<std::string, uint16_t> blockMap;
public:
    static inline const AbstractBlock& get(uint16_t id) {
        return blocks[id];
    }

    static inline void registerBlock(AbstractBlock& block) {
        std::string literal = block.getLiteralId();
        if(!literal.empty()) {
            blockMap[literal] = block.getId();
        }
        blocks[block.getId()] = block;
    }
};

#endif