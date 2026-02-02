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
        meshInstance.aabb = AABB(Vector3(0.0f), Vector3(ChunkInfo::WIDTH, ChunkInfo::HEIGHT, ChunkInfo::DEPTH));
        meshInstance.setPosition(
            Vector3(
                double(chunk->pos.x) * double(ChunkInfo::WIDTH) ,
                double(chunk->pos.y) * double(ChunkInfo::HEIGHT) ,
                double(chunk->pos.z) * double(ChunkInfo::DEPTH)
            )
        );
    }
};

using ChunkClientPtr = std::shared_ptr<ChunkClient>;

#endif