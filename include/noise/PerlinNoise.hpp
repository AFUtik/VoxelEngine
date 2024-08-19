#ifndef PERLINNOISE_HPP
#define PERLINNOISE_HPP

#include <vector>

class PerlinNoise {
private:
	static std::vector<std::uint8_t> p;
public:
	static unsigned long default_seed;

	static void seed(unsigned long seed);

	static float noise(double x, double y);
	static float noise(double x, double y, double z);
};

#endif // !PERLIN_NOISE2D_HP