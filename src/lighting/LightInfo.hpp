#ifndef LIGHTINFO_HPP
#define LIGHTINFO_HPP

#include <queue>
#include <structures/RingBuffer.hpp>

class Chunk;

struct LightEntry {
	int lx;
	int ly;
	int lz;
	uint8_t light;
};

using LightBatch = std::queue<LightEntry>;

#endif