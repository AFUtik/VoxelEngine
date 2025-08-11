#include "Block.hpp"
#include "Chunk.hpp"

#include <iostream>
#include <chrono>

#include "../noise/PerlinNoise.hpp"
#include "../lighting/LightMap.hpp"

#include <glm/ext.hpp>
#include <cmath>

uint32_t Chunk::WIDTH  = 16;
uint32_t Chunk::HEIGHT = 256;
uint32_t Chunk::DEPTH  = 16;
uint32_t Chunk::VOLUME = WIDTH*HEIGHT*DEPTH;

Chunk::Chunk(int x, int y, int z, PerlinNoise& generator) :
	lightmap(new Lightmap),
	blocks(std::make_unique<block[]>(VOLUME)),
	x(x), y(y), z(z) {
	const float scale = 0.05f;
	for (int _y = 0; _y < HEIGHT; _y++) {
		for (int _z = 0; _z < DEPTH; _z++) {
			for (int _x = 0; _x < WIDTH; _x++) {
				// Global position //
				const int gx = _x + this->x * WIDTH;
				const int gy = _y + this->y * HEIGHT;
				const int gz = _z + this->z * DEPTH;
				float value = generator.noise(
					static_cast<float>(gx)*scale,
					static_cast<float>(gy)*scale,
					static_cast<float>(gz)*scale);
				int id = 0;
				if (value <= 0.01)
					id = 1;
				setBlock(_x, _y, _z, id);
			}
		}
	}
}

uint8_t Chunk::getBoundLight(int lx, int ly, int lz, int channel) {
	Chunk* chunk = findNeighbourChunk(lx, ly, lz);
	if (chunk == nullptr) return 0;

	int localX = lx - (chunk->x-x) * WIDTH;
	int localY = ly - (chunk->y-y) * HEIGHT;
	int localZ = lz - (chunk->z-z) * DEPTH;

	return chunk->lightmap->get(localX, localY, localZ, channel);
}

uint8_t Chunk::getLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel) const {
	return lightmap->get(lx, ly, lz, channel);
}

void Chunk::setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission) {
	return lightmap->set(lx, ly, lz, channel, emission);
}