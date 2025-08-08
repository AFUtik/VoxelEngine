#include <iostream>
#include <numeric>
#include <algorithm>
#include <vector>

#include "PerlinNoise.hpp"

PerlinNoise::PerlinNoise(uint64_t seed): seed(seed), gen(seed) {
    std::iota(p, p+256, uint8_t{ 0 });
    std::shuffle(p, p+256, gen);

    for(size_t i = 255; i < 512; i++) p[i] = p[i-255];
}

float PerlinNoise::noise(double x, double y) {
    int X = static_cast<int>(floor(x)) & 255;
    int Y = static_cast<int>(floor(y)) & 255;

    x -= floor(x);
    y -= floor(y);

    double u = fade(x);
    double v = fade(y);

    const uint8_t aa = p[p[X] + Y];
    const uint8_t ab = p[p[X] + Y + 1];
    const uint8_t ba = p[p[X + 1] + Y];
    const uint8_t bb = p[p[X + 1] + Y + 1];

    return lerp(lerp(grad2(aa, x, y), grad2(ba, x - 1, y), u),
                lerp(grad2(ab, x, y - 1), grad2(bb, x - 1, y - 1), u), v);
}

float PerlinNoise::noise(double x, double y, double z) {
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
            lerp(grad3(p[AA], x, y, z), grad3(p[BA], x - 1, y, z), u),
            lerp(grad3(p[AB], x, y - 1, z), grad3(p[BB], x - 1, y - 1, z), u), v
        ),
        lerp(
            lerp(grad3(p[AA + 1], x, y, z - 1), grad3(p[BA + 1], x - 1, y, z - 1), u),
            lerp(grad3(p[AB + 1], x, y - 1, z - 1), grad3(p[BB + 1], x - 1, y - 1, z - 1), u), v
        ), w
    );
}

float PerlinNoise::detail(double x, double y, double z) {
    float total = 0.0;
    float max_amplitude = 0.0;
    float amplitude = 1.0;
    float frequency = base_freq;
    for (size_t i = 0; i < octaves; i++) {
        total += amplitude * noise(x*frequency, y*frequency, z*frequency);
        max_amplitude += amplitude;
        amplitude *= persistance;
        frequency *= lacunarity;
    }
    return total / max_amplitude;
}

float detail(double x, double y) {return 0.0f;}