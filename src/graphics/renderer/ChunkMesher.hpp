#ifndef CUBEMESHER_HPP
#define CUBEMESHER_HPP

#include <glm/glm.hpp>

#include "Drawable.hpp"
#include "../../logic/blocks/Block.hpp"
#include "../../logic/blocks/ChunkInfo.hpp"
#include "../../logic/blocks/Chunk.hpp"
#include "../../logic/LogicSystem.hpp"
#include "../model/Mesh.hpp"
#include "../model/CubeMesh.hpp"

#include "GLController.hpp"

#include <memory>
#include <mutex>

class Mesher;

using namespace glm;

static const int Dirs[6][3] = {{0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{-1,0,0},{1,0,0}};

static const float Normals[6][3] = {{0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{-1,0,0},{1,0,0}};

class ChunkMesher {
    std::unique_ptr<GlController> glController;
    LogicSystem* world;

    std::mutex meshUploadMutex;
    std::queue<ChunkPtr> meshUploadQueue;
    std::condition_variable meshUploadCv;
    std::thread worker;
    bool stop = false;
    bool light_flag = true;

    CubeMesher cubeMesher;

    template<size_t Corner>
    inline void mix4_light(
        Chunk* cur_chunk,
        LightSample &lightSample,
        float cr, float cg, float cb, float cs,
        int x0, int y0, int z0,
        int x1, int y1, int z1,
        int x2, int y2, int z2
        ) {
        lightSample.r[Corner] = (cur_chunk->getBoundLight(x0, y0, z0, 0) + cr * 30 + cur_chunk->getBoundLight(x1, y1, z1, 0) + cur_chunk->getBoundLight(x2, y2, z2, 0)) / 5.0f / 15.0f;
        lightSample.g[Corner] = (cur_chunk->getBoundLight(x0, y0, z0, 1) + cg * 30 + cur_chunk->getBoundLight(x1, y1, z1, 1) + cur_chunk->getBoundLight(x2, y2, z2, 1)) / 5.0f / 15.0f;
        lightSample.b[Corner] = (cur_chunk->getBoundLight(x0, y0, z0, 2) + cb * 30 + cur_chunk->getBoundLight(x1, y1, z1, 2) + cur_chunk->getBoundLight(x2, y2, z2, 2)) / 5.0f / 15.0f;
        lightSample.s[Corner] = (cur_chunk->getBoundLight(x0, y0, z0, 3) + cs * 30 + cur_chunk->getBoundLight(x1, y1, z1, 3) + cur_chunk->getBoundLight(x2, y2, z2, 3)) / 5.0f / 15.0f;
    }

    template<direction DIR>
    inline void makeFace(
        Chunk* cur_chunk,
        VertexConsumer &consumer,
        int x, int y, int z,
        float u1, float v1,
        float u2, float v2) {
        constexpr int dx = direction::EAST  == DIR ? 1 : direction::WEST  == DIR ? -1 : 0;
        constexpr int dy = direction::UP    == DIR ? 1 : direction::DOWN  == DIR ? -1 : 0;
        constexpr int dz = direction::NORTH == DIR ? 1 : direction::SOUTH == DIR ? -1 : 0;

        float lr = cur_chunk->getBoundLight(x+dx, y+dy, z+dz, 0) / 15.0f;
        float lg = cur_chunk->getBoundLight(x+dx, y+dy, z+dz, 1) / 15.0f;
        float lb = cur_chunk->getBoundLight(x+dx, y+dy, z+dz, 2) / 15.0f;
        float ls = cur_chunk->getBoundLight(x+dx, y+dy, z+dz, 3) / 15.0f;

        FaceInfo faceInfo;
        LightSample &lightSample = faceInfo.lightSample;

        faceInfo.x = x;
        faceInfo.y = y;
        faceInfo.z = z;

        faceInfo.u1 = u1;
        faceInfo.u2 = u2;
        faceInfo.v1 = v1;
        faceInfo.v2 = v2;

        if constexpr (DIR == direction::UP) {
            mix4_light<0>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y+1,z, x-1,y+1,z-1, x,y+1,z-1);
            mix4_light<1>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y+1,z, x-1,y+1,z+1, x,y+1,z+1);
            mix4_light<2>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y+1,z, x+1,y+1,z+1, x,y+1,z+1);
            mix4_light<3>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y+1,z, x+1,y+1,z-1, x,y+1,z-1);
        } else if constexpr (DIR == direction::DOWN) {
            mix4_light<0>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y-1,z, x-1,y-1,z-1, x,y-1,z-1);
            mix4_light<1>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y-1,z, x-1,y-1,z+1, x,y-1,z+1);
            mix4_light<2>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y-1,z, x+1,y-1,z+1, x,y-1,z+1);
            mix4_light<3>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y-1,z, x+1,y-1,z-1, x,y-1,z-1);
        } else if constexpr (DIR == direction::EAST) {
            mix4_light<0>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y-1,z-1, x+1,y,z-1, x+1,y-1,z);
            mix4_light<1>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y+1,z-1, x+1,y,z-1, x+1,y+1,z);
            mix4_light<2>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y+1,z+1, x+1,y,z+1, x+1,y+1,z);
            mix4_light<3>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y-1,z+1, x+1,y,z+1, x+1,y-1,z);
        } else if constexpr (DIR == direction::WEST) {
            mix4_light<0>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y-1,z-1, x-1,y,z-1, x-1,y-1,z);
            mix4_light<1>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y+1,z-1, x-1,y,z-1, x-1,y+1,z);
            mix4_light<2>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y+1,z+1, x-1,y,z+1, x-1,y+1,z);
            mix4_light<3>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y-1,z+1, x-1,y,z+1, x-1,y-1,z);
        } else if constexpr (DIR == direction::NORTH) {
            mix4_light<0>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y-1,z+1, x,y-1,z+1, x-1,y,z+1);
            mix4_light<1>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y+1,z+1, x,y+1,z+1, x-1,y,z+1);
            mix4_light<2>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y+1,z+1, x,y+1,z+1, x+1,y,z+1);
            mix4_light<3>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y-1,z+1, x,y-1,z+1, x+1,y,z+1);
        } else if constexpr (DIR == direction::SOUTH) {
            mix4_light<0>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y-1,z-1, x,y-1,z-1, x-1,y,z-1);
            mix4_light<1>(cur_chunk, lightSample, lr, lg, lb, ls, x-1,y+1,z-1, x,y+1,z-1, x-1,y,z-1);
            mix4_light<2>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y+1,z-1, x,y+1,z-1, x+1,y,z-1);
            mix4_light<3>(cur_chunk, lightSample, lr, lg, lb, ls, x+1,y-1,z-1, x,y-1,z-1, x+1,y,z-1);
        }
        cubeMesher.makeFace<DIR>(consumer, faceInfo);
    }

    friend class BlockRenderer; 
public:
    ChunkMesher(LogicSystem* world) : glController(new GlController), world(world) {
        worker = std::thread([this, world] { meshWorkerThread(); });
    }

    ~ChunkMesher() {
        {
            std::lock_guard<std::mutex> lk(world->readyQueueMutex);
            stop = true;
        }
        world->readyCv.notify_all();
        if (worker.joinable())
            worker.join();
    }

    inline void makeCube(Chunk* cur_chunk, VertexConsumer &consumer, int lx, int ly, int lz, uint8_t id)
    {
        float uvsize = 1.0f / 16.0f;
        float u1 = (id % 16) * uvsize;
        float v1 = 1 - ((1 + id / 16) * uvsize);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;
        
        if (!cur_chunk->getBoundBlock(lx, ly+1, lz).isOpaque()) makeFace<direction::UP>(cur_chunk, consumer, lx, ly, lz, u1, v1, u2, v2);  // UP
        if (!cur_chunk->getBoundBlock(lx, ly-1, lz).isOpaque()) makeFace<direction::DOWN>(cur_chunk, consumer,  lx, ly, lz, u1, v1, u2, v2); // DOWN
        if (!cur_chunk->getBoundBlock(lx+1, ly, lz).isOpaque()) makeFace<direction::EAST>(cur_chunk, consumer, lx, ly, lz, u1, v1, u2, v2); // EAST
        if (!cur_chunk->getBoundBlock(lx-1, ly, lz).isOpaque()) makeFace<direction::WEST>(cur_chunk, consumer,  lx, ly, lz, u1, v1, u2, v2); // WEST
        if (!cur_chunk->getBoundBlock(lx, ly, lz+1).isOpaque()) makeFace<direction::NORTH>(cur_chunk, consumer,  lx, ly, lz, u1, v1, u2, v2); // NORTH
        if (!cur_chunk->getBoundBlock(lx, ly, lz-1).isOpaque()) makeFace<direction::SOUTH>(cur_chunk, consumer, lx, ly, lz, u1, v1, u2, v2); // SOUTH
    }

    inline std::shared_ptr<Mesh> makeChunk(Chunk* chunk) {
        auto mesh = std::make_shared<Mesh>(glController.get());
        VertexConsumer consumer = mesh->getConsumer();
        for (int y = 0; y < ChunkInfo::HEIGHT; y++) {
            for (int z = 0; z < ChunkInfo::DEPTH; z++) {
                for (int x = 0; x < ChunkInfo::WIDTH; x++) {
                    block vox = chunk->getBlock(x, y, z);
                    unsigned int id = vox.id;

                    if (!id) continue;
                    makeCube(chunk, consumer, x, y, z, vox.id);
                }
            }
        }
        {
            std::lock_guard<std::mutex> lk(glController->meshUploadMutex);
            glController->glUpload.push(mesh);
        }
        return mesh;
    }

    inline void updateChunk(Chunk* chunk) {
        auto mesh = chunk->drawable.getSharedMesh();
        if(!mesh) return; 
        VertexConsumer consumer = mesh->getConsumer();
        for (int y = 0; y < ChunkInfo::HEIGHT; y++) {
            for (int z = 0; z < ChunkInfo::DEPTH; z++) {
                for (int x = 0; x < ChunkInfo::WIDTH; x++) {
                    block vox = chunk->getBlock(x, y, z);
                    unsigned int id = vox.id;
                    if (!id) continue;
                    makeCube(chunk, consumer, x, y, z, vox.id);
                }
            }
        }
        mesh->verticesUpdated = consumer.getIndex();
        {
            std::lock_guard<std::mutex> lk(glController->meshUpdateMutex);
            glController->glUpdate.push(mesh);
        }
    }

    void meshWorkerThread();

    inline void modifyCube() {}

    inline void set_light(bool flag) {light_flag = flag;}
};

#endif //CUBEMESHER_HPP
