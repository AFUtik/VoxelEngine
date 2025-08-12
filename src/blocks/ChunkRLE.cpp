//
// Created by 280325 on 8/11/2025.
//
#include "ChunkRLE.hpp"
#include "Chunk.hpp"

Chunk *ChunkRLE::decode(ChunkRLE *rle) {
    Chunk* chunk = new Chunk(rle->x, rle->y, rle->z);
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
    return chunk;
}

ChunkRLE* ChunkRLE::encode(Chunk *chunk) {
    ChunkRLE *chunkRLE = new ChunkRLE(chunk->x, chunk->y, chunk->z);

    int16_t count = 0;
    uint8_t cid = 0;
    for (int i = 0; i < ChunkInfo::VOLUME; i++) {
        uint8_t id = chunk->blocks[i].id;
        if (id == cid) {
            count++;
        } else {
            chunkRLE->rle.push_back(RLE2D{count, cid});

            count = 1;
            cid = id;
        }
    }
    chunkRLE->rle.push_back(RLE2D{count, cid});
    return chunkRLE;
}