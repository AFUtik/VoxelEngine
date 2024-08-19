#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <cmath>

#include "noise/perlin_noise3d.hpp"

PerlinGenerator3D::PerlinGenerator3D(const unsigned long seed) : default_seed(seed) {
    p.resize(256);
    std::iota(p.begin(), p.end(), uint8_t{ 0 });
    std::mt19937 gen(seed);
    std::shuffle(p.begin(), p.end(), gen);
    p.insert(p.end(), p.begin(), p.end());
}

inline constexpr static float fade(float t) noexcept {
    return t * t * t * ((6 * t - 15) * t + 10);
}

inline constexpr static double lerp(double a, double b, double t) noexcept {
    return a + (b - a) * t;
}

inline constexpr double grad(int hash, double x, double y, double z) noexcept {
    const std::uint8_t h = hash & 15;
    const float u = h < 8 ? x : y;
    const float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}


float PerlinGenerator3D::noise(double x, double y, double z) {
    const int xInd = static_cast<int>(std::floor(x)) & 255;
    const int yInd = static_cast<int>(std::floor(y)) & 255;
    const int zInd = static_cast<int>(std::floor(z)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    const float u = fade(x);
    const float v = fade(y);
    const float w = fade(z);

    const std::uint8_t A =  p[xInd] + yInd;
    const std::uint8_t AA = p[A] + zInd;
    const std::uint8_t AB = p[A + 1] + zInd;
    const std::uint8_t B =  p[xInd + 1] + yInd;
    const std::uint8_t BA = p[B] + zInd;
    const std::uint8_t BB = p[B + 1] + zInd;

    return lerp(
        lerp(
            lerp(grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z), u),
            lerp(grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z), u) , v
        ),
        lerp(
            lerp(grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1), u),
            lerp(grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1), u), v
        ), w
    );
}
