#ifndef RENDERINFO_HPP
#define RENDERINFO_HPP

#include <cstdint>

struct block_render {
	uint32_t offset      = 0; // 1 OFFSET = 6 VERTICES //
	uint8_t face_bitmask = 0;
}; 

struct ChunkRenderInfo {
    block_render blocks[16*16*256];
};

#endif