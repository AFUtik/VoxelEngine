#include "Chunk.hpp"

#include "../lighting/LightMap.hpp"
#include "ChunkCompressor.hpp"
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

void Chunk::compress(bool toDelete) {
	if(!checkState(ChunkState::Finished)) return;

	setState(ChunkState::Compressed);
	compressed = ChunkCompressor::compress(blocks.get(), lightmap.get());
	
	if(toDelete) {
		blocks.reset();
		lightmap.reset();
	}
}

void Chunk::decompress(bool toDelete) {
	if(!checkState(ChunkState::Compressed)) return;

	blocks = std::make_unique<block[]>(ChunkInfo::VOLUME);
	lightmap = std::make_unique<Lightmap>();

	ChunkCompressor::decompress(compressed.get(), blocks.get(), lightmap.get());

	if(toDelete) {
		compressed.reset();
	}
	setState(ChunkState::Finished);
}