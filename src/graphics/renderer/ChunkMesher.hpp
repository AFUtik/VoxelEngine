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

using namespace glm;

enum class direction {
    UP, DOWN, EAST, WEST, NORTH, SOUTH
};

static const int Dirs[6][3] = {{0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{-1,0,0},{1,0,0}};

static const float Normals[6][3] = {{0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{-1,0,0},{1,0,0}};

class ChunkMesher {
    Chunk* cur_chunk;

    bool light_flag = true;

    inline void mix4_light(
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

    /*
    Mesh generateGreedyMesh(VertexConsumer &consumer, Chunk *chunk) {
        Mesh mesh;
 
        // Буфер маски и вспомогательных данных
        struct MaskCell { uint8_t id; uint8_t light; };
        std::vector<MaskCell> mask(std::max({ChunkInfo::WIDTH, ChunkInfo::HEIGHT, ChunkInfo::DEPTH}) * std::max({ChunkInfo::WIDTH, ChunkInfo::HEIGHT, ChunkInfo::DEPTH}));
    
        // Проходим по 6 направлениям
        for (int d = 0; d < 6; ++d) {
            int dx = Dirs[d][0], dy = Dirs[d][1], dz = Dirs[d][2];
            int u = (d==0||d==1) ? 0 : (d==2||d==3) ? 0 : 1; // индексы осей для проекции (упрощение)
            int v = (d==0||d==1) ? 1 : (d==2||d==3) ? 2 : 2; // упрощенно
    
            // Шаги по основной оси
            int xLen = ChunkInfo::WIDTH, yLen = ChunkInfo::HEIGHT, zLen = ChunkInfo::DEPTH;
    
            // Размеры проекции
            int iMax = (d < 2) ? ChunkInfo::WIDTH : (d < 4) ? ChunkInfo::WIDTH : ChunkInfo::HEIGHT;
            int jMax = (d < 2) ? ChunkInfo::HEIGHT : (d < 4) ? ChunkInfo::DEPTH : ChunkInfo::DEPTH;
    
            for (int slice = -1; slice < zLen; ++slice) {
                // Build mask
                int n = 0;
                for (int j = 0; j < jMax; ++j) {
                    for (int i = 0; i < iMax; ++i) {
                        int x = i, y = j, z = slice;
                        // transform depending on direction
                        int ax = x, ay = y, az = z;
                        if (d==0) { az = z; }
                        // Get current and neighbor block
                        Block cur {0};
                        Block neigh {0};
                        if (inBounds(ax, ay, az)) cur = getBlock(chunk, ax, ay, az);
                        int nx = ax + dx, ny = ay + dy, nz = az + dz;
                        if (inBounds(nx, ny, nz)) neigh = getBlock(chunk, nx, ny, nz);
    
                        // If current is opaque and neighbor is not -> face
                        if (cur.isOpaque() && !neigh.isOpaque()) {
                            mask[n++] = {cur.id, cur.light};
                        } else {
                            mask[n++] = {0,0};
                        }
                    }
                }
    
                // Greedy merge mask
                int w = iMax;
                int h = jMax;
                for (int j = 0; j < h; ++j) {
                    for (int i = 0; i < w;) {
                        auto mc = mask[j*w + i];
                        if (mc.id == 0) { ++i; continue; }
                        // find width
                        int width = 1;
                        while (i + width < w && mask[j*w + (i+width)].id == mc.id && mask[j*w + (i+width)].light == mc.light) ++width;
                        // find height
                        int height = 1;
                        bool done = false;
                        while (j + height < h && !done) {
                            for (int k = 0; k < width; ++k) {
                                auto mc2 = mask[(j+height)*w + (i+k)];
                                if (mc2.id != mc.id || mc2.light != mc.light) { done = true; break; }
                            }
                            if (!done) ++height;
                        }
    
                        // Create quad for the rectangle (i,j)-(i+width,j+height)
                        // Compute 3D coordinates для вершин в зависимости от d
                        float x0 = i, y0 = j;
                        float x1 = i + width, y1 = j + height;
    
                        // Convert to world coords (упрощённо)
                        Vertex v0, v1, v2, v3;
                        // set positions depending on face direction (упрощение)
                        // TODO: тут нужна точная трансформация по оси
                        v0.x = x0; v0.y = y0; v0.z = slice + 1;
                        v1.x = x1; v1.y = y0; v1.z = slice + 1;
                        v2.x = x1; v2.y = y1; v2.z = slice + 1;
                        v3.x = x0; v3.y = y1; v3.z = slice + 1;
                        v0.u = 0; v0.v = 0; v1.u = 1; v1.v = 0; v2.u = 1; v2.v = 1; v3.u = 0; v3.v = 1;
                        v0.nx = Normals[d][0]; v0.ny = Normals[d][1]; v0.nz = Normals[d][2];
                        v1 = v0; v2 = v0; v3 = v0;
                        v0.light = mc.light; v1.light = mc.light; v2.light = mc.light; v3.light = mc.light;
    
                        uint32_t baseIndex = (uint32_t)mesh.vertices.size();
                        mesh.vertices.push_back(v0);
                        mesh.vertices.push_back(v1);
                        mesh.vertices.push_back(v2);
                        mesh.vertices.push_back(v3);
                        mesh.indices.push_back(baseIndex + 0);
                        mesh.indices.push_back(baseIndex + 1);
                        mesh.indices.push_back(baseIndex + 2);
                        mesh.indices.push_back(baseIndex + 2);
                        mesh.indices.push_back(baseIndex + 3);
                        mesh.indices.push_back(baseIndex + 0);
    
                        // zero out mask
                        for (int yy = 0; yy < height; ++yy) for (int xx = 0; xx < width; ++xx) mask[(j+yy)*w + (i+xx)] = {0,0};
    
                        i += width;
                    }
                }
            }
        }
        
        return mesh;
    }
    */
    

    template<direction DIR>
    inline void makeFace(
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
public:
    inline void makeCube(VertexConsumer &consumer, int lx, int ly, int lz, uint8_t id)
    {
        float uvsize = 1.0f / 16.0f;
        float u1 = (id % 16) * uvsize;
        float v1 = 1 - ((1 + id / 16) * uvsize);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;

        if (!cur_chunk->getBoundBlock(lx, ly+1, lz).id) makeFace<direction::UP>(consumer, lx, ly, lz, u1, v1, u2, v2);  // UP
        if (!cur_chunk->getBoundBlock(lx, ly-1, lz).id) makeFace<direction::DOWN>(consumer,  lx, ly, lz, u1, v1, u2, v2); // DOWN
        if (!cur_chunk->getBoundBlock(lx+1, ly, lz).id) makeFace<direction::EAST>(consumer, lx, ly, lz, u1, v1, u2, v2); // EAST
        if (!cur_chunk->getBoundBlock(lx-1, ly, lz).id) makeFace<direction::WEST>(consumer,  lx, ly, lz, u1, v1, u2, v2); // WEST
        if (!cur_chunk->getBoundBlock(lx, ly, lz+1).id) makeFace<direction::NORTH>(consumer,  lx, ly, lz, u1, v1, u2, v2); // NORTH
        if (!cur_chunk->getBoundBlock(lx, ly, lz-1).id) makeFace<direction::SOUTH>(consumer, lx, ly, lz, u1, v1, u2, v2); // SOUTH
    }

    inline void makeChunk(Chunk* chunk) {
        cur_chunk = chunk;

        VertexConsumer consumer = chunk->chunk_draw.getMesh()->getConsumer();
        for (int y = 0; y < ChunkInfo::HEIGHT; y++) {
            for (int z = 0; z < ChunkInfo::DEPTH; z++) {
                for (int x = 0; x < ChunkInfo::WIDTH; x++) {
                    block vox = chunk->getBlock(x, y, z);
                    unsigned int id = vox.id;

                    if (!id) continue;

                    makeCube(consumer, x, y, z, vox.id);
                }
            }
        }
    }

    inline void modifyCube() {}

    inline void set_light(bool flag) {light_flag = flag;}
};

#endif //CUBEMESHER_HPP
