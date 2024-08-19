#ifndef PERLIN_NOISE_HPP
#define PERLIN_NOISE_HPP

#include <vector>

class PerlinNoise {
private:
	std::vector<std::uint8_t> p;
public:
	const unsigned long default_seed;

	PerlinNoise(const unsigned long seed);

	float noise2d(double x, double y);
	float noise3d(double x, double y, double z);
};

#endif // !PERLIN_NOISE2D_HP