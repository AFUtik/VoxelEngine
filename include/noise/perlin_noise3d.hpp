#ifndef PERLIN_NOISE3D_HPP
#define PERLIN_NOISE3D_HPP

typedef unsigned int uint;

#include <random>

class PerlinGenerator3D {
private:
	static float V[12][3];
	const long default_seed;
	const uint M, N, B, w, h, l;
	uint ***R;
public:
	PerlinGenerator3D(uint M, uint N, uint B, uint w, uint h, uint l, const long seed);
	~PerlinGenerator3D();

	float noise(int x, int y, int z);
	float*** genDataset();
};

extern void free_dataset(float*** dataset, int width, int height);

#endif // !PERLIN_NOISE_HPP