//
// Created by 280325 on 8/11/2025.
//

#ifndef CHUNKINFO_HPP
#define CHUNKINFO_HPP

#include <cstdint>

struct ChunkInfo {
    static const inline int32_t SECTION_WIDTH  = 32;
    static const inline int32_t SECTION_HEIGHT = 32;
    static const inline int32_t SECTION_DEPTH  = 32;
    static const inline int32_t SECTION_VOLUME = SECTION_WIDTH * SECTION_DEPTH * SECTION_HEIGHT;

    static const inline int32_t WIDTH  = SECTION_WIDTH;
    static const inline int32_t HEIGHT = 256;
    static const inline int32_t DEPTH  = SECTION_DEPTH;
    static const inline int32_t VOLUME = WIDTH * HEIGHT * DEPTH;
};

#endif //CHUNKINFO_HPP
