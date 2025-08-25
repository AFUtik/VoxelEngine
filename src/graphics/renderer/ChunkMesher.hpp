//
// Created by 280325 on 8/8/2025.
//

#ifndef CUBEMESHER_HPP
#define CUBEMESHER_HPP

#include <glm/glm.hpp>

#include "Drawable.hpp"
#include "../../blocks/Block.hpp"
#include "../../blocks/Chunks.hpp"
#include "../../blocks/ChunkInfo.hpp"
#include "../../blocks/Chunk.hpp"
#include "structures/ThreadPool.hpp"

#include <memory>
#include <shared_mutex>
#include <mutex>

using namespace glm;

enum class direction {
    UP, DOWN, EAST, WEST, NORTH, SOUTH
};

static const int Dirs[6][3] = {{0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{-1,0,0},{1,0,0}};

static const float Normals[6][3] = {{0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{-1,0,0},{1,0,0}};

class ChunkMesher {
    Chunks* world; 

    std::mutex meshUploadMutex;
    std::queue<std::weak_ptr<Chunk>> meshUploadQueue;
    std::condition_variable meshUploadCv;

    std::thread worker;
    bool stop = false;

    bool light_flag = true;

    inline void mix4_light(
        Chunk* cur_chunk,
        float &lr, float &lg, float &lb, float &ls,
        float cr, float cg, float cb, float cs,
        int x0, int y0, int z0,
        int x1, int y1, int z1,
        int x2, int y2, int z2
        ) {
        lr = (cur_chunk->getBoundLight(x0, y0, z0, 0) + cr * 30 + cur_chunk->getBoundLight(x1, y1, z1, 0) + cur_chunk->getBoundLight(x2, y2, z2, 0)) / 5.0f / 15.0f;
        lg = (cur_chunk->getBoundLight(x0, y0, z0, 1) + cg * 30 + cur_chunk->getBoundLight(x1, y1, z1, 1) + cur_chunk->getBoundLight(x2, y2, z2, 1)) / 5.0f / 15.0f;
        lb = (cur_chunk->getBoundLight(x0, y0, z0, 2) + cb * 30 + cur_chunk->getBoundLight(x1, y1, z1, 2) + cur_chunk->getBoundLight(x2, y2, z2, 2)) / 5.0f / 15.0f;
        ls = (cur_chunk->getBoundLight(x0, y0, z0, 3) + cs * 30 + cur_chunk->getBoundLight(x1, y1, z1, 3) + cur_chunk->getBoundLight(x2, y2, z2, 3)) / 5.0f / 15.0f;
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

        float lr0 = 1.0f, lr1 = 1.0f, lr2 = 1.0f, lr3 = 1.0f;
        float lg0 = 1.0f, lg1 = 1.0f, lg2 = 1.0f, lg3 = 1.0f;
        float lb0 = 1.0f, lb1 = 1.0f, lb2 = 1.0f, lb3 = 1.0f;
        float ls0 = 1.0f, ls1 = 1.0f, ls2 = 1.0f, ls3 = 1.0f;


        if (light_flag) {
            if constexpr (DIR == direction::UP) {
                mix4_light(cur_chunk, lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y+1,z, x-1,y+1,z-1, x,y+1,z-1);
                mix4_light(cur_chunk, lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y+1,z, x-1,y+1,z+1, x,y+1,z+1);
                mix4_light(cur_chunk, lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y+1,z, x+1,y+1,z+1, x,y+1,z+1);
                mix4_light(cur_chunk, lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y+1,z, x+1,y+1,z-1, x,y+1,z-1);
            } else if constexpr (DIR == direction::DOWN) {
                mix4_light(cur_chunk, lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y-1,z, x-1,y-1,z-1, x,y-1,z-1);
                mix4_light(cur_chunk, lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y-1,z, x-1,y-1,z+1, x,y-1,z+1);
                mix4_light(cur_chunk, lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y-1,z, x+1,y-1,z+1, x,y-1,z+1);
                mix4_light(cur_chunk, lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y-1,z, x+1,y-1,z-1, x,y-1,z-1);
            } else if constexpr (DIR == direction::EAST) {
                mix4_light(cur_chunk, lr0, lg0, lb0, ls0, lr, lg, lb, ls, x+1,y-1,z-1, x+1,y,z-1, x+1,y-1,z);
                mix4_light(cur_chunk, lr1, lg1, lb1, ls1, lr, lg, lb, ls, x+1,y+1,z-1, x+1,y,z-1, x+1,y+1,z);
                mix4_light(cur_chunk, lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y+1,z+1, x+1,y,z+1, x+1,y+1,z);
                mix4_light(cur_chunk, lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y-1,z+1, x+1,y,z+1, x+1,y-1,z);
            } else if constexpr (DIR == direction::WEST) {
                mix4_light(cur_chunk, lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y-1,z-1, x-1,y,z-1, x-1,y-1,z);
                mix4_light(cur_chunk, lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y+1,z-1, x-1,y,z-1, x-1,y+1,z);
                mix4_light(cur_chunk, lr2, lg2, lb2, ls2, lr, lg, lb, ls, x-1,y+1,z+1, x-1,y,z+1, x-1,y+1,z);
                mix4_light(cur_chunk, lr3, lg3, lb3, ls3, lr, lg, lb, ls, x-1,y-1,z+1, x-1,y,z+1, x-1,y-1,z);
            } else if constexpr (DIR == direction::NORTH) {
                mix4_light(cur_chunk, lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y-1,z+1, x,y-1,z+1, x-1,y,z+1);
                mix4_light(cur_chunk, lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y+1,z+1, x,y+1,z+1, x-1,y,z+1);
                mix4_light(cur_chunk, lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y+1,z+1, x,y+1,z+1, x+1,y,z+1);
                mix4_light(cur_chunk, lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y-1,z+1, x,y-1,z+1, x+1,y,z+1);
            } else if constexpr (DIR == direction::SOUTH) {
                mix4_light(cur_chunk, lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y-1,z-1, x,y-1,z-1, x-1,y,z-1);
                mix4_light(cur_chunk, lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y+1,z-1, x,y+1,z-1, x-1,y,z-1);
                mix4_light(cur_chunk, lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y+1,z-1, x,y+1,z-1, x+1,y,z-1);
                mix4_light(cur_chunk, lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y-1,z-1, x,y-1,z-1, x+1,y,z-1);
            }
        }

        if constexpr (DIR == direction::UP) {
            consumer.vertex(Vertex{ x-0.5f, y+0.5f, z-0.5f, u1, v2, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x-0.5f, y+0.5f, z+0.5f, u1, v1, lr1, lg1, lb1, ls1}); // 1
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z-0.5f, u2, v2, lr3, lg3, lb3, ls3}); // 3

            consumer.vertex(Vertex{ x-0.5f, y+0.5f, z+0.5f, u1, v1, lr1, lg1, lb1, ls1}); // 1
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z+0.5f, u2, v1, lr2, lg2, lb2, ls2}); // 2
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z-0.5f, u2, v2, lr3, lg3, lb3, ls3}); // 3
        }
        else if constexpr (DIR == direction::DOWN) {
            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z-0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x+0.5f, y-0.5f, z-0.5f, u2, v1, lr3, lg3, lb3, ls3}); // 1
            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z+0.5f, u1, v2, lr1, lg1, lb1, ls1}); // 3

            consumer.vertex(Vertex{ x+0.5f, y-0.5f, z-0.5f, u2, v1, lr3, lg3, lb3, ls3}); // 1
            consumer.vertex(Vertex{ x+0.5f, y-0.5f, z+0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2
            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z+0.5f, u1, v2, lr1, lg1, lb1, ls1}); // 3
        }
        else if constexpr (DIR == direction::EAST) {
            consumer.vertex(Vertex{ x+0.5f, y-0.5f, z-0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z+0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2
            consumer.vertex(Vertex{ x+0.5f, y-0.5f, z+0.5f, u1, v2, lr3, lg3, lb3, ls3}); // 1

            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z+0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2
            consumer.vertex(Vertex{ x+0.5f, y-0.5f, z-0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z-0.5f, u2, v1, lr1, lg1, lb1, ls1}); // 3
        }
        else if constexpr (DIR == direction::WEST) {
            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z-0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x-0.5f, y+0.5f, z+0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2
            consumer.vertex(Vertex{ x-0.5f, y+0.5f, z-0.5f, u2, v1, lr1, lg1, lb1, ls1}); // 3

            consumer.vertex(Vertex{ x-0.5f, y+0.5f, z+0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2
            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z-0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z+0.5f, u1, v2, lr3, lg3, lb3, ls3}); // 1
        }
        else if constexpr (DIR == direction::NORTH) {
            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z+0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x+0.5f, y-0.5f, z+0.5f, u2, v1, lr3, lg3, lb3, ls3}); // 1
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z+0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2

            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z+0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z+0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2
            consumer.vertex(Vertex{ x-0.5f, y+0.5f, z+0.5f, u1, v2, lr1, lg1, lb1, ls1}); // 3
        }
        else if constexpr (DIR == direction::SOUTH) {
            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z-0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z-0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2
            consumer.vertex(Vertex{ x+0.5f, y-0.5f, z-0.5f, u2, v1, lr3, lg3, lb3, ls3}); // 1

            consumer.vertex(Vertex{ x-0.5f, y-0.5f, z-0.5f, u1, v1, lr0, lg0, lb0, ls0}); // 0
            consumer.vertex(Vertex{ x-0.5f, y+0.5f, z-0.5f, u1, v2, lr1, lg1, lb1, ls1}); // 3
            consumer.vertex(Vertex{ x+0.5f, y+0.5f, z-0.5f, u2, v2, lr2, lg2, lb2, ls2}); // 2
        }

    }

    friend class BlockRenderer; 
public:
    ChunkMesher(Chunks* world) : world(world) {
        worker = std::thread([this, world] { meshWorkerThread(); });
    }

    ~ChunkMesher() {
        // сигнализируем потоку остановиться
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
        
        if (!cur_chunk->getBoundBlock(lx, ly+1, lz).id) makeFace<direction::UP>(cur_chunk, consumer, lx, ly, lz, u1, v1, u2, v2);  // UP
        if (!cur_chunk->getBoundBlock(lx, ly-1, lz).id) makeFace<direction::DOWN>(cur_chunk, consumer,  lx, ly, lz, u1, v1, u2, v2); // DOWN
        if (!cur_chunk->getBoundBlock(lx+1, ly, lz).id) makeFace<direction::EAST>(cur_chunk, consumer, lx, ly, lz, u1, v1, u2, v2); // EAST
        if (!cur_chunk->getBoundBlock(lx-1, ly, lz).id) makeFace<direction::WEST>(cur_chunk, consumer,  lx, ly, lz, u1, v1, u2, v2); // WEST
        if (!cur_chunk->getBoundBlock(lx, ly, lz+1).id) makeFace<direction::NORTH>(cur_chunk, consumer,  lx, ly, lz, u1, v1, u2, v2); // NORTH
        if (!cur_chunk->getBoundBlock(lx, ly, lz-1).id) makeFace<direction::SOUTH>(cur_chunk, consumer, lx, ly, lz, u1, v1, u2, v2); // SOUTH
    }

    inline Mesh* makeChunk(Chunk* chunk) {
        Mesh* mesh = new Mesh;
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
        
        
        return mesh;
    }

    void meshWorkerThread() {
        while (true) {
            std::weak_ptr<Chunk> cd;
            {
                std::unique_lock lk(world->readyQueueMutex);
                world->readyCv.wait(lk, [&] {
                    return stop || !world->readyChunks.empty();
                });
                if (stop && world->readyChunks.empty())
                    break;


                cd = world->readyChunks.front();
                world->readyChunks.pop();
            }
            

            if (auto sp = cd.lock()) {
                Mesh* mesh;
                {
                    std::shared_lock<std::shared_mutex> wlock(sp->dataMutex);
                    mesh = makeChunk(sp.get());
                }
                {
                    std::unique_lock<std::shared_mutex> wl(sp->dataMutex);
                    sp->chunk_draw.loadMesh(mesh);
                }
                {
                    std::lock_guard lk(meshUploadMutex);
                    meshUploadQueue.push(sp);
                }
                meshUploadCv.notify_one();   
            }
        }
    }

    inline void modifyCube() {}

    inline void set_light(bool flag) {light_flag = flag;}
};

#endif //CUBEMESHER_HPP
