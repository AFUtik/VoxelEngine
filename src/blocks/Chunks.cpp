#include "Chunks.hpp"
#include "Chunk.hpp"
#include "Block.hpp"
#include "iostream"

#include "../lighting/LightMap.hpp"

#include <glm/glm.hpp>

using namespace glm;

#include <math.h>
#include <limits.h>

Chunks::Chunks(int w, int h, int d)  {
	PerlinNoise noise(0);
	noise.octaves = 2;

	int index = 0;
	for (int y = 0; y < h; y++) {
		for (int z = 0; z < d; z++) {
			for (int x = 0; x < w; x++, index++) {
				Chunk* chunk = new Chunk(x, y, z, noise);

				chunk_map[hash_xyz(x, y, z)] = std::unique_ptr<Chunk>(chunk);
				iterable.push_back(chunk);
			}
		}
	}
}

block Chunks::getBlock(int x, int y, int z) {
	int cx = x / Chunk::WIDTH;
	int cy = y / Chunk::HEIGHT;
	int cz = z / Chunk::DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0)
		return block{0};
	Chunk* chunk = chunk_map[hash_xyz(cx, cy, cz)].get();
	if (!chunk) return block{0};
	int lx = x - cx * Chunk::WIDTH;
	int ly = y - cy * Chunk::HEIGHT;
	int lz = z - cz * Chunk::DEPTH;
	return chunk->getBlock(lx, ly, lz);
}

unsigned char Chunks::getLight(int x, int y, int z, int channel) {
	int cx = x / Chunk::WIDTH;
	int cy = y / Chunk::HEIGHT;
	int cz = z / Chunk::DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0)
		return 0;
	Chunk* chunk = chunk_map[hash_xyz(cx, cy, cz)].get();
	if (chunk == nullptr) return 0;
	int lx = x - cx * Chunk::WIDTH;
	int ly = y - cy * Chunk::HEIGHT;
	int lz = z - cz * Chunk::DEPTH;
	return chunk->lightmap->get(lx, ly, lz, channel);
}

Chunk* Chunks::getChunkByBlock(int x, int y, int z) {
	int cx = x / Chunk::WIDTH;
	int cy = y / Chunk::HEIGHT;
	int cz = z / Chunk::DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0)
		return nullptr;
	return chunk_map[hash_xyz(cx, cy, cz)].get();
}

Chunk* Chunks::getChunk(int32_t x, int32_t y, int32_t z) {
	if (x < 0 || y < 0 || z < 0)
		return nullptr;
	return chunk_map[hash_xyz(x, y, z)].get();
}

//void Chunks::set(int x, int y, int z, int id) {
//	int cx = x / Chunk::WIDTH;
//	int cy = y / Chunk::HEIGHT;
//	int cz = z / Chunk::DEPTH;
//	if (x < 0) cx--;
//	if (y < 0) cy--;
//	if (z < 0) cz--;
//	if (cx < 0 || cy < 0 || cz < 0 || cx >= w || cy >= h || cz >= d)
//		return;
//	Chunk* chunk = chunks[(cy * d + cz) * w + cx];
//	int lx = x - cx * Chunk::WIDTH;
//	int ly = y - cy * Chunk::HEIGHT;
//	int lz = z - cz * Chunk::DEPTH;
//	chunk->blocks[(ly * Chunk::DEPTH + lz) * Chunk::WIDTH + lx].id = id;
//	// chunk->modified = true;
//
//	if (lx == 0 && (chunk = getChunk(cx - 1, cy, cz))) chunk->modified = true;
//	if (ly == 0 && (chunk = getChunk(cx, cy - 1, cz))) chunk->modified = true;
//	if (lz == 0 && (chunk = getChunk(cx, cy, cz - 1))) chunk->modified = true;
//
//	if (lx == Chunk::WIDTH - 1 && (chunk = getChunk(cx + 1, cy, cz))) chunk->modified = true;
//	if (ly == Chunk::HEIGHT - 1 && (chunk = getChunk(cx, cy + 1, cz))) chunk->modified = true;
//	if (lz == Chunk::DEPTH - 1 && (chunk = getChunk(cx, cy, cz + 1))) chunk->modified = true;
//}