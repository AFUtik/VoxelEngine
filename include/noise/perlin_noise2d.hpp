#ifndef PERLIN_NOISE2D_HPP
#define PERLIN_NOISE2D_HPP

typedef unsigned int uint;

#include <random>

class PerlinGenerator {
private:
	static float V[8][2];
	const std::mt19937 generator;
	const uint M, N, w, h;
	uint **R;

	float noise(int x, int y);
public:
	PerlinGenerator(uint M, uint N, uint w, uint h, const long seed);
	~PerlinGenerator();

	float** genDataset();
};

extern void free_dataset(float** dataset, size_t rows);

#endif // !PERLIN_NOISE_HPP