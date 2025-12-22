#include "Raycasting.hpp"
#include "../Chunks.hpp"

BlockHit raycastBlock(const glm::dvec3& origin,
                      const glm::vec3& dir,
                      float maxDistance,
                      Chunks* world)
{
    glm::dvec3 pos = glm::floor(origin); // стартовая клетка
    glm::ivec3 blockPos = glm::ivec3(pos);

    // направление шага (-1 или +1 по каждой оси)
    glm::ivec3 step;
    step.x = (dir.x > 0) ? 1 : -1;
    step.y = (dir.y > 0) ? 1 : -1;
    step.z = (dir.z > 0) ? 1 : -1;

    // расстояние до первой границы по каждой оси
    glm::dvec3 tMax;
    glm::dvec3 tDelta;

    auto intbound = [](double s, double ds) {
        if (ds > 0) {
            return (std::floor(s + 1.0) - s) / ds;
        } else {
            return (s - std::floor(s)) / -ds;
        }
    };

    tMax.x = intbound(origin.x, dir.x);
    tMax.y = intbound(origin.y, dir.y);
    tMax.z = intbound(origin.z, dir.z);

    tDelta.x = (dir.x != 0.0) ? std::abs(1.0 / dir.x) : DBL_MAX;
    tDelta.y = (dir.y != 0.0) ? std::abs(1.0 / dir.y) : DBL_MAX;
    tDelta.z = (dir.z != 0.0) ? std::abs(1.0 / dir.z) : DBL_MAX;

    double dist = 0.0;

    
    while (dist <= maxDistance) {
        glm::ivec3 chunkCoord = worldToChunk3(blockPos);
        std::shared_ptr<Chunk> chunk = world->getChunk(chunkCoord.x, chunkCoord.y, chunkCoord.z);
        if (chunk) {
            int lx, ly, lz;
            Chunk::local(lx, ly, lz, blockPos.x, blockPos.y, blockPos.z);

            if (lx >= 0 && lx < ChunkInfo::WIDTH &&
                ly >= 0 && ly < ChunkInfo::HEIGHT &&
                lz >= 0 && lz < ChunkInfo::DEPTH)
            {
                block b = chunk->getBlock(lx, ly, lz);
                if (b.id != 0) {
                    return BlockHit{true, blockPos.x, blockPos.y, blockPos.z};
                }
            }
        }

        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) {
                blockPos.x += step.x;
                dist = tMax.x;
                tMax.x += tDelta.x;
            } else {
                blockPos.z += step.z;
                dist = tMax.z;
                tMax.z += tDelta.z;
            }
        } else {
            if (tMax.y < tMax.z) {
                blockPos.y += step.y;
                dist = tMax.y;
                tMax.y += tDelta.y;
            } else {
                blockPos.z += step.z;
                dist = tMax.z;
                tMax.z += tDelta.z;
            }
        }
    }

    return {};
}