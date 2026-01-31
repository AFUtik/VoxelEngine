#ifndef CHUNKCLIENT_HPP
#define CHUNKCLIENT_HPP

#include "../../../graphics/renderer/MeshInstance.hpp"
#include "../../server/blocks/Chunk.hpp"

#include <glm/glm.hpp>
#include <memory>

using namespace glm;

// CHUNK WRAPPER WITH RENDER DATA //
class ChunkClient {
private:
    ChunkPtr chunk;
public:
    MeshInstance meshInstance;
    inline ChunkPtr getChunk() { return chunk; }

    ChunkClient(ChunkPtr chunk) : chunk(chunk) 
    {
        std::cout << chunk->pos.x << ' ' << chunk->pos.z << std::endl;
        meshInstance.getTransform().setPosition(
            Vector3(
                double(chunk->pos.x) * double(ChunkInfo::WIDTH) ,
                double(chunk->pos.y) * double(ChunkInfo::HEIGHT) ,
                double(chunk->pos.z) * double(ChunkInfo::DEPTH)
            )
        );
        meshInstance.aabb = AABB(
            Vector3(double(chunk->pos.x) * ChunkInfo::WIDTH, double(chunk->pos.y) * ChunkInfo::HEIGHT, double(chunk->pos.z) * ChunkInfo::DEPTH),
            Vector3((double(chunk->pos.x) + 1) * ChunkInfo::WIDTH, (double(chunk->pos.y) + 1) * ChunkInfo::HEIGHT, (double(chunk->pos.z) + 1) * ChunkInfo::DEPTH)
        );
    }
};

using ChunkClientPtr = std::shared_ptr<ChunkClient>;

#endif