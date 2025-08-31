//
// Created by 280325 on 8/11/2025.
//

#ifndef CHUNKRLE_HPP
#define CHUNKRLE_HPP

#include <memory>
#include <vector>
#include "../lighting/LightCompressor.hpp"

class Chunk;

struct BlockCompression {
    int16_t length;
    uint8_t id;
};

struct ChunkCompressed {
    int x, y, z;

    std::vector<BlockCompression> rle;
    RGBS_compression lightCompression;
    
    ChunkCompressed(int x, int y, int z) : x(x), y(y), z(z) {};
};

class ChunkCompressor {
    friend class Chunks;
public:
    static std::shared_ptr<ChunkCompressed> compress(const std::shared_ptr<Chunk> chunk);
    static std::shared_ptr<Chunk> decompress(const std::shared_ptr<ChunkCompressed> rle);
};

#endif
