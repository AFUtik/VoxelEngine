//
// Created by 280325 on 8/11/2025.
//

#ifndef CHUNKRLE_HPP
#define CHUNKRLE_HPP

#include <vector>

class Chunk;

struct RLE3D {
    int16_t w, h, d;
    int16_t id;
};

struct RLE2D {
    int16_t length;
    uint8_t id;
};

class ChunkRLE {
    std::vector<RLE2D> rle;
    int x, y, z;

    friend class Chunks;
public:
    ChunkRLE(int x, int y, int z) : x(x), y(y), z(z) {};

    static Chunk* decode(ChunkRLE* rle);
    static ChunkRLE* encode(Chunk *chunk);
};

#endif //INC_3DRLE_HPP
