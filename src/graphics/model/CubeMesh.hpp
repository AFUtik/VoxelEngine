#ifndef CUBEMESH_HPP
#define CUBEMESH_HPP

#include "../vertex/VertexConsumer.hpp"
#include "../vertex/VertexInfo.hpp"
#include "../texture/TextureRegion.hpp"
#include "structures/PackedFreelist.hpp"

#include <cstdint>
#include <array>

enum class direction : uint8_t {
    UP = 0, DOWN = 1, EAST = 2, WEST = 3, NORTH = 4, SOUTH = 5
};

// удобнее индексировать компоненты света
struct LightSample {
    float r[4];
    float g[4];
    float b[4];
    float s[4];
};

struct FaceInfo {
    float x, y, z;
    // у тебя в оригинале были u1,u2,v1,v2 — делаем их явными
    float u1, u2, v1, v2;
    LightSample lightSample;
};

struct FaceVertTpl {
    float ox, oy, oz;   // offset от центра блока
    float u, v;         // локальные UV (будут прибавлены к faceInfo.u/v)
    uint8_t cornerIdx;  // 0..3 индекс соответствующей компоненты в LightSample
};

struct CubeMesher {
private:
    static constexpr std::array<std::array<FaceVertTpl,6>,6> faceTemplates = {{
        // UP (Y+)
        std::array<FaceVertTpl,6>{{
            {-0.5f, +0.5f, -0.5f, /*u*/0.0f, /*v*/1.0f, 0},
            {-0.5f, +0.5f, +0.5f,          0.0f,        0.0f, 1},
            {+0.5f, +0.5f, -0.5f,          1.0f,        1.0f, 3},
            {-0.5f, +0.5f, +0.5f,          0.0f,        0.0f, 1},
            {+0.5f, +0.5f, +0.5f,          1.0f,        0.0f, 2},
            {+0.5f, +0.5f, -0.5f,          1.0f,        1.0f, 3}
        }},
        // DOWN (Y-)
        std::array<FaceVertTpl,6>{{
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0},
            {+0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 3},
            {-0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 1},
            {+0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 3},
            {+0.5f, -0.5f, +0.5f, 1.0f, 1.0f, 2},
            {-0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 1}
        }},
        // EAST (X+)
        std::array<FaceVertTpl,6>{{
            {+0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0},
            {+0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 2},
            {+0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 3},
            {+0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 2},
            {+0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0},
            {+0.5f, +0.5f, -0.5f, 1.0f, 0.0f, 1}
        }},
        // WEST (X-)
        std::array<FaceVertTpl,6>{{
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0},
            {-0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 2},
            {-0.5f, +0.5f, -0.5f, 1.0f, 0.0f, 1},
            {-0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 2},
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0},
            {-0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 3}
        }},
        // NORTH (Z+)
        std::array<FaceVertTpl,6>{{
            {-0.5f, -0.5f, +0.5f, 0.0f, 0.0f, 0},
            {+0.5f, -0.5f, +0.5f, 1.0f, 0.0f, 3},
            {+0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 2},
            {-0.5f, -0.5f, +0.5f, 0.0f, 0.0f, 0},
            {+0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 2},
            {-0.5f, +0.5f, +0.5f, 0.0f, 1.0f, 1}
        }},
        // SOUTH (Z-)
        std::array<FaceVertTpl,6>{{
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0},
            {+0.5f, +0.5f, -0.5f, 1.0f, 1.0f, 2},
            {+0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 3},
            {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0},
            {-0.5f, +0.5f, -0.5f, 0.0f, 1.0f, 1},
            {+0.5f, +0.5f, -0.5f, 1.0f, 1.0f, 2}
        }}
    }};

    static inline void applyLightFromSample(Vertex &v, const LightSample &ls, uint8_t corner) {
        v.lr = ls.r[corner];
        v.lg = ls.g[corner];
        v.lb = ls.b[corner];
        v.ls = ls.s[corner];
    }

public:
    template<direction DIR>
    void makeFace(VertexConsumer &consumer, const FaceInfo& info) {
        constexpr size_t dirIndex = static_cast<size_t>(DIR);
        const auto &tpl = faceTemplates[dirIndex];

        for (const auto &tv : tpl) {
            Vertex v;
            v.x = info.x + tv.ox;
            v.y = info.y + tv.oy;
            v.z = info.z + tv.oz;

            v.u = (tv.u == 0.0f) ? info.u1 : info.u2;
            v.v = (tv.v == 0.0f) ? info.v1 : info.v2;

            applyLightFromSample(v, info.lightSample, tv.cornerIdx);
            consumer.vertex(v);
        }
    }
};

#endif