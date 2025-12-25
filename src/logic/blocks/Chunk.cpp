#include "Chunk.hpp"

#include "../lighting/LightMap.hpp"
#include <glm/ext.hpp>
#include <memory>

uint8_t Chunk::getBoundLight(int lx, int ly, int lz, int channel) {
	Chunk *chunk = findNeighbourChunk(lx, ly, lz);
	if (!chunk) return 0;

	int localX, localY, localZ;
	local(localX, localY, localZ, lx, ly, lz);

	return chunk->lightmap->get(localX, localY, localZ, channel);
}

uint8_t Chunk::getLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel) const {
	return lightmap->get(lx, ly, lz, channel);
}

void Chunk::setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission) {
	return lightmap->set(lx, ly, lz, channel, emission);
}