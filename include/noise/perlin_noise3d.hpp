#ifndef PERLIN_NOISE3D_HPP
#define PERLIN_NOISE3D_HPP

#include <vector>

class PerlinGenerator3D {
private:
	std::vector<std::uint8_t> p;
public:
	const unsigned long default_seed;

	PerlinGenerator3D(const unsigned long seed);
	~PerlinGenerator3D();

	float noise(double x, double y, double z);
};

#endif // !PERLIN_NOISE_HPP