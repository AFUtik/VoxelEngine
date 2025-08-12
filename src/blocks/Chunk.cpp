#include "Block.hpp"
#include "Chunk.hpp"

#include <iostream>
#include <chrono>

#include "../noise/PerlinNoise.hpp"
#include "../lighting/LightMap.hpp"

#include <glm/ext.hpp>
#include <cmath>

Chunk::Chunk(int x, int y, int z, PerlinNoise& generator) :
	lightmap(new Lightmap),
	blocks(std::make_unique<block[]>(ChunkInfo::ChunkInfo::VOLUME)),
	x(x), y(y), z(z) {
	const float scale = 0.02f;
	const float scale2 = 0.02f;
	const float height = 10.0f;
	for (int _z = 0; _z < ChunkInfo::DEPTH; _z++) {
		for (int _x = 0; _x < ChunkInfo::WIDTH; _x++) {
			// Global position //
			const int gx = _x + this->x * ChunkInfo::WIDTH;
			const int gz = _z + this->z * ChunkInfo::DEPTH;
			int y = height * generator.noise(
				static_cast<float>(gx)*scale,
				static_cast<float>(gz)*scale);

			for(int _y = 0; _y < y + 100; _y++) {
				float n = generator.noise(
				static_cast<float>(gx)*scale2,
				static_cast<float>(_y + this->y * ChunkInfo::HEIGHT)*scale2,
				static_cast<float>(gz)*scale2
				);
				int id = 1;
				if (n < 0.0005) id = 0;

				setBlock(_x, _y, _z, id);
			}
		}
	}

}

uint8_t Chunk::getBoundLight(int lx, int ly, int lz, int channel) {
	Chunk* chunk = findNeighbourChunk(lx, ly, lz);
	if (chunk == nullptr) return 0;

	int localX = lx - (chunk->x-x) * ChunkInfo::WIDTH;
	int localY = ly - (chunk->y-y) * ChunkInfo::HEIGHT;
	int localZ = lz - (chunk->z-z) * ChunkInfo::DEPTH;

	return chunk->lightmap->get(localX, localY, localZ, channel);
}

uint8_t Chunk::getLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel) const {
	return lightmap->get(lx, ly, lz, channel);
}

void Chunk::setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission) {
	return lightmap->set(lx, ly, lz, channel, emission);
}