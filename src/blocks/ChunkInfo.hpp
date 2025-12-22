//
// Created by 280325 on 8/11/2025.
//

#ifndef CHUNKINFO_HPP
#define CHUNKINFO_HPP

#include <cstdint>

struct ChunkInfo {
    static const inline uint32_t WIDTH  = 16;
    static const inline uint32_t HEIGHT = 256;
    static const inline uint32_t DEPTH  = 16;
    static const inline uint32_t VOLUME = 65536;
};

#endif //CHUNKINFO_HPP
