//
// Created by 280325 on 8/11/2025.
//
#include "ChunkCompressor.hpp"
#include "Chunk.hpp"
#include "../lighting/LightCompressor.hpp"
#include <memory>
#include <shared_mutex>

std::shared_ptr<ChunkCompressed> ChunkCompressor::compress(const std::shared_ptr<Chunk> chunk) {
    auto chunkRLE = std::make_shared<ChunkCompressed>(chunk->x, chunk->y, chunk->z);
    int16_t count = 0;
    uint8_t cid = 0;
    {
        std::shared_lock<std::shared_mutex> wl(chunk->dataMutex);
        for (int i = 0; i < ChunkInfo::VOLUME; i++) {
            uint8_t id = chunk->blocks[i].id;
            if (id == cid) {
                count++;
            } else {
                chunkRLE->rle.push_back(BlockCompression{count, cid});
    
                count = 1;
                cid = id;
            }
        }
    }
    chunkRLE->rle.push_back(BlockCompression{count, cid});
    LightCompressor::compress(chunk->lightmap.get(), chunkRLE->lightCompression);
    return chunkRLE;
}

std::shared_ptr<Chunk> ChunkCompressor::decompress(const std::shared_ptr<ChunkCompressed> rle) {
    auto chunk = std::make_shared<Chunk>(rle->x, rle->y, rle->z);
    int16_t offset = 0;
    for (auto &[length, id] : rle->rle) {
        if (id==0) {
            offset += length;
            continue;
        }

        block *start = chunk->blocks.get()+offset;

        std::fill(start, start+length, block{id});
        offset += length;
    }
    LightCompressor::decompress(chunk->lightmap.get(), rle->lightCompression);
    return chunk;
}
