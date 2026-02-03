#ifndef CHUNKCLIENT_HPP
#define CHUNKCLIENT_HPP

#include "../../../graphics/renderer/MeshInstance.hpp"
#include "../../server/blocks/Chunk.hpp"
#include <glm/glm.hpp>

using namespace glm;

class ChunkClient {
private:
    std::weak_ptr<ChunkClient> neighbours_safe[26];
    std::weak_ptr<ChunkClient> this_weak;
    ChunkClient* neighbours_raw[26] {nullptr};
public:
    std::unique_ptr<Lightmap> lightmap;
    std::unique_ptr<block[]>  blocks;
    
    Vector3I pos;
    uint32_t version;

    MeshInstance meshInstance;

    ChunkClient(Vector3I pos) : pos(pos), lightmap(new Lightmap), blocks(std::make_unique<block[]>(ChunkInfo::VOLUME))
    {
        meshInstance.aabb = AABB(Vector3(0.0f), Vector3(ChunkInfo::WIDTH, ChunkInfo::HEIGHT, ChunkInfo::DEPTH));
        meshInstance.setPosition(
            Vector3(
                double(pos.x) * double(ChunkInfo::WIDTH) ,
                double(pos.y) * double(ChunkInfo::HEIGHT) ,
                double(pos.z) * double(ChunkInfo::DEPTH)
            )
        );
    }

    inline void loadNeighbour(int ind, std::shared_ptr<ChunkClient> chunk) {
        neighbours_safe[ind] = chunk;
        neighbours_raw[ind]  = chunk.get();

        chunk->neighbours_safe[25 - ind] = this_weak;
        chunk->neighbours_raw[25  - ind]  = this;
    }

    inline ChunkClient* findNeighbourPtr(int x, int y, int z) {
        int dx = 0, dy = 0, dz = 0;
        if (x < 0)                   dx = -1;
        else if (x >= ChunkInfo::WIDTH)  dx = 1;
        if (y < 0)                   dy = -1;
        else if (y >= ChunkInfo::HEIGHT)  dy = 1;
        if (z < 0)                   dz = -1;
        else if (z >= ChunkInfo::DEPTH)  dz = 1;
        return neighbours_raw[INDEX(dx, dy, dz)];
    }

    inline block getBoundBlock(int x, int y, int z) {
        if (Chunk::INSIDE(x, y, z)) {
            return getBlock(x, y, z);
        }
        else {
            ChunkClient* chunk = findNeighbourPtr(x, y, z);
            if (chunk) {
                int32_t nx, ny, nz;
                Chunk::LOCAL(nx, ny, nz, x, y, z);
                return chunk->getBlock(nx, ny, nz);
            }
            else return block{};
        }
    }

    inline uint8_t getBoundLight(int x, int y, int z, int channel) {
        if (Chunk::INSIDE(x, y, z)) {
            return lightmap->get(x, y, z, channel);
        }
        else {
            ChunkClient* chunk = findNeighbourPtr(x, y, z);
            if (chunk) {
                int32_t nx, ny, nz;
                Chunk::LOCAL(nx, ny, nz, x, y, z);
                return chunk->lightmap->get(nx, ny, nz, channel);
            }
            else return 0;
        }
    }

    inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const { return blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx]; }
    inline void  setBlock(int32_t lx, int32_t ly, int32_t lz, uint8_t id) { blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx].id = id; }

    static inline int INDEX(int dx, int dy, int dz) {
        const int x = dx + 1;
        const int y = dy + 1;
        const int z = dz + 1;
        int idx = x + y * 3 + z * 9;
        if (idx > 13) idx--;
        return idx;
    }
};

using ChunkClientPtr = std::shared_ptr<ChunkClient>;
using ClientNeighboursLock = std::shared_ptr<ChunkClient>[26];

#endif