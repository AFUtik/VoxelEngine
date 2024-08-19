#include <iostream>
#include <random>
#include <cmath>

#include "noise/perlin_noise3d.hpp"

float PerlinGenerator3D::V[26][3] = {
        {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f},  {-1.0f, 1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 1.0f},  {1.0f, 0.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
        {0.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 1.0f},  {0.0f, 1.0f, -1.0f}, {0.0f, -1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f},  {1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
        {-1.0f,-1.0f,-1.0f},{-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f},{1.0f, -1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}
};

PerlinGenerator3D::PerlinGenerator3D(uint M, uint N, uint B, 
                                     uint w, uint h, uint l, 
                                     uint x_max, uint y_max, uint z_max,
                                     const long seed) : M(M), N(N), B(B), w(w), h(h), l(l), default_seed(seed) {
    if (x_max > M * w || y_max > N * h || z_max > B * l) {
        std::cerr << "Error with PerlinNoise3D -> Exception: Invalid size 'max_coords > specified_coords'" << std::endl;
    }

    R = new uint**[M];
    for (int i = 0; i < M; ++i) {
        R[i] = new uint*[N];
        for (int j = 0; j < N; ++j) {
            R[i][j] = new uint[B];
        }
    }
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(0, 26);

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int v = 0; v < B; ++v) {
                R[i][j][v] = dis(gen);
            }
        }
    }
}

inline float fade(float t) {
    return t * t * t * ((6 * t - 15) * t + 10);
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float grad(int hash, float x, float y, float z) {
    int h = hash & 15; // Берём последние 4 бита от hash
    float u = (h < 8) ? x : y; // Если h < 8, u = x, иначе u = y
    float v = (h < 4) ? y : ((h == 12 || h == 14) ? z : x); // Если h < 4, v = y; если h = 12 или 14, v = z; иначе v = x
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v); // Возвращаем u или -u, и добавляем v или -v
}

inline float dot(const float* a, const float* b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] + b[2];
}

float PerlinGenerator3D::noise(float x, float y, float z) {
    int xInd = x / w;
    int yInd = y / h;
    int zInd = z / l;

    float xl = x - xInd * w;
    float yl = y - yInd * h;
    float zl = z - zInd * l;
    float al = fade(xl / w);
    float be = fade(yl / h);
    float ga = fade(zl / l);

    float b000[] = { xl, yl, zl };
    float b001[] = { xl - w, yl, zl };
    float b010[] = { xl, yl - h, zl };
    float b011[] = { xl - w, yl - h, zl };
    float b100[] = { xl, yl, zl - l };
    float b101[] = { xl - w, yl, zl - l };
    float b110[] = { xl, yl - h, zl - l };
    float b111[] = { xl - w, yl - h, zl - l };

    float* e000 = V[R[xInd][yInd][zInd]];
    float* e001 = V[R[xInd + 1][yInd][zInd]];
    float* e010 = V[R[xInd][yInd + 1][zInd]];
    float* e011 = V[R[xInd + 1][yInd + 1][zInd]];
    float* e100 = V[R[xInd][yInd][zInd + 1]];
    float* e101 = V[R[xInd + 1][yInd][zInd + 1]];
    float* e110 = V[R[xInd][yInd + 1][zInd + 1]];
    float* e111 = V[R[xInd + 1][yInd + 1][zInd + 1]];

    float c000 = dot(b000, e000);
    float c001 = dot(b001, e001);
    float c010 = dot(b010, e010);
    float c011 = dot(b011, e011);
    float c100 = dot(b100, e100);
    float c101 = dot(b101, e101);
    float c110 = dot(b110, e110);
    float c111 = dot(b111, e111);

    float cx00 = lerp(c000, c001, al);
    float cx01 = lerp(c010, c011, al);
    float cx10 = lerp(c100, c101, al);
    float cx11 = lerp(c110, c111, al);
    float cxy0 = lerp(cx00, cx01, be);
    float cxy1 = lerp(cx10, cx11, be);
    return lerp(cxy0, cxy1, ga);
}

float*** PerlinGenerator3D::genDataset() {
    int MW = (M - 1) * w;
    int MH = (N - 1) * h;
    int ML = (B - 1) * l;
    float*** dataset = new float** [MW];
    for (int i = 0; i < MW; ++i) {
        dataset[i] = new float*[MH];
        for (int j = 0; j < MH; ++j) {
            dataset[i][j] = new float[ML];
        }
    }

    for (int i = 0; i < MW; ++i) {
        for (int j = 0; j < MH; ++j) {
            for (int v = 0; v < ML; ++v) {
                dataset[i][j][v] = noise(i, j, v);
            }
        }
    }
    return dataset;
}

PerlinGenerator3D::~PerlinGenerator3D() {
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            delete[] R[i][j];
        }
        delete[] R[i];
    }
    delete[] R;
}

void free_dataset(float*** dataset, int width, int height) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            delete[] dataset[i][j];
        }
        delete[] dataset[i];
    }
    delete[] dataset;
}