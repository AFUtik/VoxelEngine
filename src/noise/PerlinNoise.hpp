#ifndef PERLINNOISE_HPP
#define PERLINNOISE_HPP

#include <vector>
#include <random>

class PerlinNoise {
	uint8_t p[512] = {};
	std::mt19937 gen;


	float scalex, scaley, scalez;

	uint64_t seed;

	static inline double grad2(uint8_t hash, double x, double y) {
		switch (uint8_t h = hash & 3) {
			case 0: return x+y;
			case 1: return -x+y;
			case 2: return x-y;
			case 3: return -x-y;
			default: return 0;
		}
	}

	static inline double grad3(uint8_t hash, double x, double y, double z) {
		const std::uint8_t h = hash & 15;
		const float u = h < 8 ? x : y;
		const float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

	static inline double fade(double t) noexcept {
		return t * t * t * ((6 * t - 15) * t + 10);
	}

	static inline double lerp(double a, double b, double t) noexcept {
		return a + (b - a) * t;
	}

public:
	uint32_t octaves  = 1;
	float base_freq   = 1.0;
	float persistance = 0.5f;
	float lacunarity  = 2.0f;

	PerlinNoise(uint64_t seed);

	float detail(double x, double y, double z);
	float detail(double x, double y);

	float noise(double x, double y);
	float noise(double x, double y, double z);
};

#endif // !PERLIN_NOISE2D_HP