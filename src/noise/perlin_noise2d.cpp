#include <iostream>
#include <random>
#include <cmath>

#include "noise/perlin_noise2d.hpp"

float PerlinGenerator::V[8][2] = {
    {0.0f, 1.0f}, {0.0f, -1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f},
    {1.0f, 1.0f}, {-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}
};

PerlinGenerator::PerlinGenerator(uint M, uint N, 
                                 uint w, uint h, const long seed) : M(M), N(N), w(w), h(h) {
    R = new uint * [M];
    for (int i = 0; i < M; ++i) {
        R[i] = new uint[N];
    }
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(0, 7);

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            R[i][j] = dis(gen);
        }
    }
}

inline float f(float t) {
    return t * t * t * ((6 * t - 15) * t + 10);
}

inline float g(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float dot(const float* a, const float* b) {
    return a[0] * b[0] + a[1] * b[1];
}

float PerlinGenerator::noise(int x, int y) {
    int xInd = (int) floor(x / w);
    int yInd = (int) floor(y / h);

    float xl = x - xInd * w;
    float yl = y - yInd * h;
    float al = f(xl / w);
    float be = f(yl / h);

    float b10[] = { xl, yl - h };
    float b11[] = { xl - w, yl - h };
    float b00[] = { xl, yl };
    float b01[] = { xl - w, yl};

    float* e00 = V[R[xInd][yInd]];
    float* e01 = V[R[yInd][xInd + 1]];
    float* e10 = V[R[yInd + 1][xInd]];
    float* e11 = V[R[yInd + 1][xInd + 1]];

    float c00 = dot(b00, e00);
    float c01 = dot(b01, e01);
    float c10 = dot(b10, e10);
    float c11 = dot(b11, e11);

    float cx1 = g(c00, c01, al);
    float cx2 = g(c10, c11, al);
    return g(cx1, cx2, be);
}

float** PerlinGenerator::genDataset() {
    int MW = (M - 1) * w;
    int MH = (N - 1) * h;
    float **dataset = new float*[MW];
    for (int i = 0; i < MH; i++) {
        dataset[i] = new float[MH];
    }

    for (int i = 0; i < MW; ++i) {
        for (int j = 0; j < MH; ++j) {
            dataset[i][j] = noise(i, j);
        }
    }
    return dataset;
}

PerlinGenerator::~PerlinGenerator() {
    for (int i = 0; i < M; ++i) {
        delete[] R[i];
    }
    delete[] R;
}

void free_dataset(float** dataset, size_t rows) {
    for (size_t i = 0; i < rows; ++i) {
        delete[] dataset[i];
    }
    delete[] dataset;
}