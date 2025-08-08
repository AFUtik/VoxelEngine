//
// Created by 280325 on 8/8/2025.
//

#ifndef CHUNKDRAWBUILDER_HPP
#define CHUNKDRAWBUILDER_HPP

#include "../../blocks/Chunks.hpp"
#include "../../lighting/LightMap.hpp"

/*
 * ChunkDraw Builder
 */
class ChunkDrawBuilder {
    VertexConsumer consumer;
    Lightmap* lightmap = nullptr;

    Chunk* chunk = nullptr;
    Chunks* world;

    int bound_x0, bound_x1;
    int bound_y0, bound_y1;
    int bound_z0, bound_z1;

    inline bool is_blocked(int x, int y, int z) {
        return world->get(chunk->x*CHUNK_W+x, chunk->y*CHUNK_H+y, chunk->z*CHUNK_D+z).id;
    }

    inline void emit_face(float x, float y, float z, const glm::vec3 offsets[4], float u, float v, float uvsize, bool flip = false)
    {
        for (int i = 0; i < 4; i++) {
            const glm::vec3 &offset = offsets[i];

            consumer.vertex(Vertex{x+offset.x, y+offset.y, z+offset.z,
                i == 0 || i == 1 ? u + uvsize : u,
                i == 0 || i == 3 ? v : v + uvsize,
                1, 1, 1, 1});
        }

        if (flip) consumer.index(3, 1, 0).index(3, 2, 1).endIndex();
        else      consumer.index(0, 1, 3).index(1, 2, 3).endIndex();
    }
public:
    ChunkDrawBuilder(Chunks* world) : world(world) {};

    void build(Chunk* chunk);
};


#endif //CHUNKDRAWBUILDER_HPP
