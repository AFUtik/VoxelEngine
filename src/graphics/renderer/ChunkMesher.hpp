//
// Created by 280325 on 8/8/2025.
//

#ifndef CUBEMESHER_HPP
#define CUBEMESHER_HPP

#include <glm/glm.hpp>

#include "Drawable.hpp"
#include "../../blocks/Block.hpp"
#include "../../blocks/Chunks.hpp"

using namespace glm;

enum class direction {
    UP, DOWN, EAST, WEST, NORTH, SOUTH
};

class ChunkMesher {
    Chunks* world;

    bool light_flag = true;

    inline void mix4_light(
        float &lr, float &lg, float &lb, float &ls,
        float cr, float cg, float cb, float cs,
        int x0, int y0, int z0,
        int x1, int y1, int z1,
        int x2, int y2, int z2
        ) {
        lr = (world->getLight(x0, y0, z0, 0) + cr * 30 + world->getLight(x1, y1, z1, 0) + world->getLight(x2, y2, z2, 0)) / 5.0f / 15.0f;
        lg = (world->getLight(x0, y0, z0, 1) + cg * 30 + world->getLight(x1, y1, z1, 1) + world->getLight(x2, y2, z2, 1)) / 5.0f / 15.0f;
        lb = (world->getLight(x0, y0, z0, 2) + cb * 30 + world->getLight(x1, y1, z1, 2) + world->getLight(x2, y2, z2, 2)) / 5.0f / 15.0f;
        ls = (world->getLight(x0, y0, z0, 3) + cs * 30 + world->getLight(x1, y1, z1, 3) + world->getLight(x2, y2, z2, 3)) / 5.0f / 15.0f;
    }
    template<direction DIR>
    inline void makeFace(
        VertexConsumer &consumer,
        int x, int y, int z,
        int lx, int ly, int lz,
        float u1, float v1,
        float u2, float v2) {
        constexpr int dx = direction::EAST  == DIR ? 1 : direction::WEST  == DIR ? -1 : 0;
        constexpr int dy = direction::UP    == DIR ? 1 : direction::DOWN  == DIR ? -1 : 0;
        constexpr int dz = direction::NORTH == DIR ? 1 : direction::SOUTH == DIR ? -1 : 0;



        float lr = world->getLight(x+dx, y+dy, z+dz, 0) / 15.0f;
        float lg = world->getLight(x+dx, y+dy, z+dz, 1) / 15.0f;
        float lb = world->getLight(x+dx, y+dy, z+dz, 2) / 15.0f;
        float ls = world->getLight(x+dx, y+dy, z+dz, 3) / 15.0f;

        float lr0 = 1.0f, lr1 = 1.0f, lr2 = 1.0f, lr3 = 1.0f;
        float lg0 = 1.0f, lg1 = 1.0f, lg2 = 1.0f, lg3 = 1.0f;
        float lb0 = 1.0f, lb1 = 1.0f, lb2 = 1.0f, lb3 = 1.0f;
        float ls0 = 1.0f, ls1 = 1.0f, ls2 = 1.0f, ls3 = 1.0f;


        if (light_flag) {
            if constexpr (DIR == direction::UP) {
                mix4_light(lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y+1,z, x-1,y+1,z-1, x,y+1,z-1);
                mix4_light(lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y+1,z, x-1,y+1,z+1, x,y+1,z+1);
                mix4_light(lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y+1,z, x+1,y+1,z+1, x,y+1,z+1);
                mix4_light(lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y+1,z, x+1,y+1,z-1, x,y+1,z-1);
            } else if constexpr (DIR == direction::DOWN) {
                mix4_light(lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y-1,z, x-1,y-1,z-1, x,y-1,z-1);
                mix4_light(lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y-1,z, x-1,y-1,z+1, x,y-1,z+1);
                mix4_light(lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y-1,z, x+1,y-1,z+1, x,y-1,z+1);
                mix4_light(lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y-1,z, x+1,y-1,z-1, x,y-1,z-1);
            } else if constexpr (DIR == direction::EAST) {
                mix4_light(lr0, lg0, lb0, ls0, lr, lg, lb, ls, x+1,y-1,z-1, x+1,y,z-1, x+1,y-1,z);
                mix4_light(lr1, lg1, lb1, ls1, lr, lg, lb, ls, x+1,y+1,z-1, x+1,y,z-1, x+1,y+1,z);
                mix4_light(lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y+1,z+1, x+1,y,z+1, x+1,y+1,z);
                mix4_light(lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y-1,z+1, x+1,y,z+1, x+1,y-1,z);
            } else if constexpr (DIR == direction::WEST) {
                mix4_light(lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y-1,z-1, x-1,y,z-1, x-1,y-1,z);
                mix4_light(lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y+1,z-1, x-1,y,z-1, x-1,y+1,z);
                mix4_light(lr2, lg2, lb2, ls2, lr, lg, lb, ls, x-1,y+1,z+1, x-1,y,z+1, x-1,y+1,z);
                mix4_light(lr3, lg3, lb3, ls3, lr, lg, lb, ls, x-1,y-1,z+1, x-1,y,z+1, x-1,y-1,z);
            } else if constexpr (DIR == direction::NORTH) {
                mix4_light(lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y-1,z+1, x,y-1,z+1, x-1,y,z+1);
                mix4_light(lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y+1,z+1, x,y+1,z+1, x-1,y,z+1);
                mix4_light(lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y+1,z+1, x,y+1,z+1, x+1,y,z+1);
                mix4_light(lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y-1,z+1, x,y-1,z+1, x+1,y,z+1);
            } else if constexpr (DIR == direction::SOUTH) {
                mix4_light(lr0, lg0, lb0, ls0, lr, lg, lb, ls, x-1,y-1,z-1, x,y-1,z-1, x-1,y,z-1);
                mix4_light(lr1, lg1, lb1, ls1, lr, lg, lb, ls, x-1,y+1,z-1, x,y+1,z-1, x-1,y,z-1);
                mix4_light(lr2, lg2, lb2, ls2, lr, lg, lb, ls, x+1,y+1,z-1, x,y+1,z-1, x+1,y,z-1);
                mix4_light(lr3, lg3, lb3, ls3, lr, lg, lb, ls, x+1,y-1,z-1, x,y-1,z-1, x+1,y,z-1);
            }
        }

        if constexpr (DIR == direction::UP) {
            consumer.vertex(Vertex{ lx-0.5f, ly+0.5f, lz-0.5f, u1, v2, lr0, lg0, lb0, ls0});
            consumer.vertex(Vertex{ lx-0.5f, ly+0.5f, lz+0.5f, u1, v1, lr1, lg1, lb1, ls1});
            consumer.vertex(Vertex{ lx+0.5f, ly+0.5f, lz+0.5f, u2, v1, lr2, lg2, lb2, ls2});
            consumer.vertex(Vertex{ lx+0.5f, ly+0.5f, lz-0.5f, u2, v2, lr3, lg3, lb3, ls3});
            consumer.index(0, 1, 3).index(1, 2, 3).endIndex();
        } else if constexpr (DIR == direction::DOWN) {
            consumer.vertex(Vertex{ lx-0.5f, ly-0.5f, lz-0.5f, u1, v1, lr0, lg0, lb0, ls0});
            consumer.vertex(Vertex{ lx+0.5f, ly-0.5f, lz-0.5f, u2, v1, lr3, lg3, lb3, ls3});
            consumer.vertex(Vertex{ lx+0.5f, ly-0.5f, lz+0.5f, u2, v2, lr2, lg2, lb2, ls2});
            consumer.vertex(Vertex{ lx-0.5f, ly-0.5f, lz+0.5f, u1, v2, lr1, lg1, lb1, ls1});
            consumer.index(0, 1, 3).index(1, 2, 3).endIndex();
        } else if constexpr (DIR == direction::EAST) {
            consumer.vertex(Vertex{ lx+0.5f, ly-0.5f, lz-0.5f, u1, v1, lr0, lg0, lb0, ls0});
            consumer.vertex(Vertex{ lx+0.5f, ly-0.5f, lz+0.5f, u1, v2, lr3, lg3, lb3, ls3});
            consumer.vertex(Vertex{ lx+0.5f, ly+0.5f, lz+0.5f, u2, v2, lr2, lg2, lb2, ls2});
            consumer.vertex(Vertex{ lx+0.5f, ly+0.5f, lz-0.5f, u2, v1, lr1, lg1, lb1, ls1});
            consumer.index(3, 1, 0).index(3, 2, 1).endIndex();
        }else if constexpr (DIR == direction::WEST) {
            consumer.vertex(Vertex{ lx-0.5f, ly-0.5f, lz-0.5f, u1, v1, lr0, lg0, lb0, ls0});
            consumer.vertex(Vertex{ lx-0.5f, ly-0.5f, lz+0.5f, u1, v2, lr3, lg3, lb3, ls3});
            consumer.vertex(Vertex{ lx-0.5f, ly+0.5f, lz+0.5f, u2, v2, lr2, lg2, lb2, ls2});
            consumer.vertex(Vertex{ lx-0.5f, ly+0.5f, lz-0.5f, u2, v1, lr1, lg1, lb1, ls1});
            consumer.index(0, 1, 3).index(1, 2, 3).endIndex();
        }else if constexpr (DIR == direction::NORTH) {
            consumer.vertex(Vertex{ lx-0.5f, ly-0.5f, lz+0.5f, u1, v1, lr0, lg0, lb0, ls0});
            consumer.vertex(Vertex{ lx+0.5f, ly-0.5f, lz+0.5f, u1, v2, lr3, lg3, lb3, ls3});
            consumer.vertex(Vertex{ lx+0.5f, ly+0.5f, lz+0.5f, u2, v2, lr2, lg2, lb2, ls2});
            consumer.vertex(Vertex{ lx-0.5f, ly+0.5f, lz+0.5f, u2, v1, lr1, lg1, lb1, ls1});
            consumer.index(0, 1, 3).index(1, 2, 3).endIndex();
        }else if constexpr (DIR == direction::SOUTH) {
            consumer.vertex(Vertex{ lx-0.5f, ly-0.5f, lz-0.5f, u1, v1, lr0, lg0, lb0, ls0});
            consumer.vertex(Vertex{ lx+0.5f, ly-0.5f, lz-0.5f, u1, v2, lr3, lg3, lb3, ls3});
            consumer.vertex(Vertex{ lx+0.5f, ly+0.5f, lz-0.5f, u2, v2, lr2, lg2, lb2, ls2});
            consumer.vertex(Vertex{ lx-0.5f, ly+0.5f, lz-0.5f, u2, v1, lr1, lg1, lb1, ls1});
            consumer.index(3, 1, 0).index(3, 2, 1).endIndex();
        }

    }
public:
    ChunkMesher(Chunks* world) : world(world) {}

    inline void makeCube(VertexConsumer &consumer,
        int gx, int gy, int gz,
        int lx, int ly, int lz, uint8_t id)
    {
        float uvsize = 1.0f / 16.0f;
        float u1 = (id % 16) * uvsize;
        float v1 = 1 - ((1 + id / 16) * uvsize);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;

        if (!world->getBlock(gx, gy+1, gz).id) makeFace<direction::UP>(consumer, gx, gy, gz, lx, ly, lz, u1, v1, u2, v2);  // UP
        if (!world->getBlock(gx, gy-1, gz).id) makeFace<direction::DOWN>(consumer, gx, gy, gz, lx, ly, lz, u1, v1, u2, v2); // DOWN
        if (!world->getBlock(gx+1, gy, gz).id) makeFace<direction::EAST>(consumer, gx, gy, gz, lx, ly, lz, u1, v1, u2, v2); // EAST
        if (!world->getBlock(gx-1, gy, gz).id) makeFace<direction::WEST>(consumer, gx, gy, gz, lx, ly, lz, u1, v1, u2, v2); // WEST
        if (!world->getBlock(gx, gy, gz+1).id) makeFace<direction::NORTH>(consumer, gx, gy, gz, lx, ly, lz, u1, v1, u2, v2); // NORTH
        if (!world->getBlock(gx, gy, gz-1).id) makeFace<direction::SOUTH>(consumer, gx, gy, gz, lx, ly, lz, u1, v1, u2, v2); // SOUTH
    }

    inline void makeChunk(Chunk* chunk) {
        VertexConsumer consumer = chunk->chunk_draw.mesh->getConsumer();
        for (int y = 0; y < Chunk::HEIGHT; y++) {
            for (int z = 0; z < Chunk::DEPTH; z++) {
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    block& vox = chunk->getBlock(x, y, z);
                    unsigned int id = vox.id;

                    if (!id) continue;

                    makeCube(consumer, x+chunk->x*Chunk::WIDTH, y+chunk->y*Chunk::HEIGHT, z+chunk->z*Chunk::DEPTH, x, y, z, vox.id);
                }
            }
        }
    }

    inline void modifyCube() {}

    inline void set_light(bool flag) {light_flag = flag;}
};

#endif //CUBEMESHER_HPP
