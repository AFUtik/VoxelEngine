//
// Created by 280325 on 8/11/2025.
//

#ifndef CHUNKINFO_HPP
#define CHUNKINFO_HPP

#include <cstdint>

struct ChunkInfo {
    int x, y, z;

    static inline uint32_t WIDTH  = 16;
    static inline uint32_t HEIGHT = 256;
    static inline uint32_t DEPTH  = 16;
    static inline uint32_t VOLUME = 65536;

    inline static constexpr void setChunkSize(uint32_t w, uint32_t h, uint32_t d) {
        WIDTH = w;
        HEIGHT = h;
        DEPTH = d;
        VOLUME = w*h*d;
    }
};

struct ChunkProperties {
    bool lighting;
};

#endif //CHUNKINFO_HPP
