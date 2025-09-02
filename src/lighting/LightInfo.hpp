#ifndef LIGHTINFO_HPP
#define LIGHTINFO_HPP

#include <cstdint>
#include <memory>
#include <structures/RingBuffer.hpp>

class Chunk;

struct Emission {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t s = 0;
};

struct LightEntry {
	/* Local Coords */
	int32_t lx;
	int32_t ly;
	int32_t lz;
	uint8_t light;
	std::weak_ptr<Chunk> chunk;
};

#endif