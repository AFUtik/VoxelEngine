//
// Created by 280325 on 8/11/2025.
//

#ifndef CHUNKINFO_HPP
#define CHUNKINFO_HPP

#include <cstdint>

struct ChunkInfo {
    static const inline uint32_t WIDTH  = 32;
    static const inline uint32_t HEIGHT = 256;
    static const inline uint32_t DEPTH  = 32;
    static const inline uint32_t VOLUME = WIDTH * HEIGHT * DEPTH;
};

#endif //CHUNKINFO_HPP
