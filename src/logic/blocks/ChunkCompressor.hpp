//
// Created by 280325 on 8/11/2025.
//

#ifndef CHUNKRLE_HPP
#define CHUNKRLE_HPP

#include <memory>
#include <vector>
#include "../lighting/LightCompressor.hpp"

class Lightmap;
class Chunk;
struct block;

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

struct DataCompressedRLE {
    std::vector<BlockCompression> rle;
    RGBS_compression lightCompression;
};

class ChunkCompressor {
    friend class Chunks;
public:
    static std::shared_ptr<ChunkCompressed> compress(const std::shared_ptr<Chunk> chunk);
    static std::shared_ptr<Chunk> decompress(const std::shared_ptr<ChunkCompressed> rle);

    static std::shared_ptr<DataCompressedRLE> compress(block* blocks, Lightmap* lightmap);
    static void decompress(DataCompressedRLE *dataRLE, block* blocks, Lightmap* lightmap);
};

#endif
