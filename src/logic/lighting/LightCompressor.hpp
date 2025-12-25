#ifndef LIGHTCOMPRESSOR_HPP
#define LIGHTCOMPRESSOR_HPP

#include <vector>
#include <cstdint>

#include "LightMap.hpp"
#include "../blocks/ChunkInfo.hpp"

struct LightCompression {
    uint8_t start;
    uint8_t end;
    uint16_t length;
};

using RGBS_compression = std::vector<LightCompression>[4];

class LightCompressor {
public:
    static void compress(Lightmap* lightmap, RGBS_compression &out);

    static void decompress(Lightmap* lightmap, const RGBS_compression &in);
};

#endif