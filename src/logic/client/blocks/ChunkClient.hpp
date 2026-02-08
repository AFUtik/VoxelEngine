#ifndef CHUNKCLIENT_HPP
#define CHUNKCLIENT_HPP

#include "../../../graphics/renderer/MeshInstance.hpp"
#include "../../server/blocks/Chunk.hpp"
#include <glm/glm.hpp>

using namespace glm;

class ChunkClient : public std::enable_shared_from_this<ChunkClient> {
private:
    std::array<std::weak_ptr<ChunkClient>, 26> neighbours_safe;
    std::mutex neighbours_mtx;
    std::mutex content_mtx;

    friend class ChunkLock;
    friend class ChunkProto;
    friend class Client;
public:
    std::unique_ptr<Lightmap> lightmap;
    std::unique_ptr<block[]>  blocks;
    
    Vector3I pos;
    uint32_t version;

    MeshGroup meshInstance;

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

    inline void loadNeighbour(int ind, const std::shared_ptr<ChunkClient> &neigh) {
        std::scoped_lock slock(neighbours_mtx, neigh->neighbours_mtx);

        if (neighbours_safe[ind].lock() || neigh->neighbours_safe[25 - ind].lock()) {
            std::cout << "okay" << std::endl;
            return;
        }

        neighbours_safe[ind] = neigh;
        neigh->neighbours_safe[25-ind] = weak_from_this();
    }

    inline bool checkNeighbours() {
        std::lock_guard<std::mutex> lock(neighbours_mtx);
        return neighbours_safe[3].lock() && neighbours_safe[4].lock() && neighbours_safe[5].lock() &&
               neighbours_safe[20].lock() && neighbours_safe[21].lock() && neighbours_safe[22].lock() &&
               neighbours_safe[12].lock() && neighbours_safe[13].lock();
    }

    inline void removeNeighbour(int ind) {
        auto neigh = neighbours_safe[ind].lock();
        if (!neigh) return;

        //std::scoped_lock slock(neighbours_mtx, neigh->neighbours_mtx);
        neighbours_safe[ind].reset();
        neigh->neighbours_safe[25 - ind].reset();
    }

    inline void unlinkAll() {
        for (int i = 0; i < 26; i++) removeNeighbour(i);
    }

    static inline int INDEX(int dx, int dy, int dz) {
        const int x = dx + 1;
        const int y = dy + 1;
        const int z = dz + 1;
        int idx = x + y * 3 + z * 9;
        if (idx > 13) idx--;
        return idx;
    }

    inline uint8_t getLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel) const { return lightmap->get(lx, ly, lz, channel); }
    inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const { return blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx]; }
};

// Safe mechanism for chunk locking //
struct ChunkLock {
    std::shared_ptr<ChunkClient> chunk;
    //std::unique_lock<std::mutex> lock;

    ChunkLock(std::weak_ptr<ChunkClient> w)
        : chunk(w.lock()) {}
        //, lock(chunk ? std::unique_lock<std::mutex>(chunk->content_mtx)
        //    : std::unique_lock<std::mutex>())

    ChunkLock(std::shared_ptr<ChunkClient> chunk) : 
        chunk(chunk)//,
        //lock(chunk->content_mtx) 
    {}

    ChunkLock() {}

    //ChunkLock(const ChunkLock&) = delete;
    //ChunkLock& operator=(const ChunkLock&) = delete;
};

// Points to neighbours and locks them for safe use //
class ChunkProto : public ChunkLock {
    std::array<std::shared_ptr<ChunkClient>, 26> onLock;
    std::array<ChunkClient*, 26> neighbours_raw{};

    ChunkClient* this_ptr = nullptr;

    friend class Mesher;
public:
    ChunkProto(std::shared_ptr<ChunkClient> chunk) : ChunkLock(chunk), this_ptr(chunk.get())
    {
        std::lock_guard<std::mutex> glock(chunk->neighbours_mtx);

        for (int i = 0; i < 26; i++) {
            auto n = chunk->neighbours_safe[i].lock();
            onLock[i] = n;
            neighbours_raw[i] = n.get();
        }
    };

    ~ChunkProto() {}

    inline ChunkClient* findNeighbourPtr(int x, int y, int z) {
        int dx = 0, dy = 0, dz = 0;
        if (x < 0)                   dx = -1;
        else if (x >= ChunkInfo::WIDTH)  dx = 1;
        if (y < 0)                   dy = -1;
        else if (y >= ChunkInfo::HEIGHT)  dy = 1;
        if (z < 0)                   dz = -1;
        else if (z >= ChunkInfo::DEPTH)  dz = 1;
        return onLock[ChunkClient::INDEX(dx, dy, dz)].get();
    }

    inline block getBoundBlock(int x, int y, int z) {
        if (Chunk::INSIDE(x, y, z)) {
            return this_ptr->getBlock(x, y, z);
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
            return this_ptr->lightmap->get(x, y, z, channel);
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

    inline uint8_t getLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel) const { return this_ptr->lightmap->get(lx, ly, lz, channel); }
    inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const { return this_ptr->blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx]; }
};

using ChunkClientPtr = std::shared_ptr<ChunkClient>;

#endif