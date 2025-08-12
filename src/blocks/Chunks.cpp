#include "Chunks.hpp"
#include "Chunk.hpp"
#include "Block.hpp"
#include "iostream"

#include "../lighting/LightMap.hpp"

#include <glm/glm.hpp>

using namespace glm;

#include <math.h>
#include <limits.h>
#include "ChunkRLE.hpp"

Chunks::Chunks(int w, int h, int d, bool lighting)  {
	PerlinNoise noise(0);
	noise.octaves = 2;

	if (lighting) {
		solverR = new LightSolver(this, 0);
		solverG = new LightSolver(this, 1);
		solverB = new LightSolver(this, 2);
		solverS = new LightSolver(this, 3);
	}

	uint64_t raw_total_bytes = 0;
	uint64_t rle_total_bytes = 0;

	int index = 0;
	for (int y = 0; y < h; y++) {
		for (int z = 0; z < d; z++) {
			for (int x = 0; x < w; x++, index++) {
				Chunk* chunk = new Chunk(x, y, z, noise);
				ChunkRLE* rle = ChunkRLE::encode(chunk);

				raw_total_bytes += 65536;
				rle_total_bytes += rle->rle.size()*4;

				uint64_t key = hash_xyz(x, y, z);

				chunk_map.emplace(key, std::unique_ptr<Chunk>(chunk));
				iterable.push_back(chunk);

				loadNeighbours(chunk);

				delete rle;
			}
		}
	}

	if (lighting) {
		for (Chunk* chunk : iterable) {
			for (int y = 0; y < ChunkInfo::HEIGHT; y++) {
				for (int z = 0; z < ChunkInfo::DEPTH; z++) {
					for (int x = 0; x < ChunkInfo::WIDTH; x++) {
						block vox = chunk->getBlock(x, y, z);
						if (vox.id == 1) {
							solverR->addLocally(x, y, z, 0, chunk);
							solverG->addLocally(x, y, z, 0, chunk);
							solverB->addLocally(x, y, z, 0, chunk);
						}
					}
				}
			}
		}

		for (Chunk* chunk : iterable) {

			for (int z = 0; z < ChunkInfo::DEPTH; z++) {
				for (int x = 0; x < ChunkInfo::WIDTH; x++) {
					for (int y = ChunkInfo::HEIGHT - 1; y >= 0; y--) {
						block vox = chunk->getBlock(x, y, z);
						if (vox.id != 0) {
							break;
						}
						chunk->setLight(x, y, z, 3, 0xF);
					}
				}
			}
		}

		for (Chunk* chunk : iterable) {
			for (int z = 0; z < ChunkInfo::DEPTH; z++) {
				for (int x = 0; x < ChunkInfo::WIDTH; x++) {
					for (int y = ChunkInfo::HEIGHT - 1; y >= 0; y--) {
						block vox = chunk->getBlock(x, y, z);
						if (vox.id != 0) {
							break;
						}
						if (
							chunk->getBoundLight(x-1, y, z, 3) == 0 ||
							chunk->getBoundLight(x+1, y, z, 3) == 0 ||
							chunk->getBoundLight(x, y-1, z, 3) == 0 ||
							chunk->getBoundLight(x, y+1, z, 3) == 0 ||
							chunk->getBoundLight(x, y, z-1, 3) == 0 ||
							chunk->getBoundLight(x, y, z+1, 3) == 0
							) solverS->addLocally(x, y, z, chunk);
						chunk->setLight(x, y, z, 3, 0xF);
					}
				}
			}
		}
	}

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();

	std::cout << "raw: " << raw_total_bytes/(1024*1024) << " mb " << "rle: "  << rle_total_bytes/(1024*1024) << " mb" << std::endl;
}


block Chunks::getBlock(int x, int y, int z) {
	int cx = x / ChunkInfo::WIDTH;
	int cy = y / ChunkInfo::HEIGHT;
	int cz = z / ChunkInfo::DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0)
		return block{0};
	Chunk* chunk = findChunkByCoordsSafe(cx, cy, cz);
	if (!chunk) return block{0};
	int lx = x - cx * ChunkInfo::WIDTH;
	int ly = y - cy * ChunkInfo::HEIGHT;
	int lz = z - cz * ChunkInfo::DEPTH;
	return chunk->getBlock(lx, ly, lz);
}

unsigned char Chunks::getLight(int x, int y, int z, int channel) {
	int cx = x / ChunkInfo::WIDTH;
	int cy = y / ChunkInfo::HEIGHT;
	int cz = z / ChunkInfo::DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0)
		return 0;
	Chunk* chunk = findChunkByCoordsSafe(cx, cy, cz);
	if (chunk == nullptr) return 0;
	int lx = x - cx * ChunkInfo::WIDTH;
	int ly = y - cy * ChunkInfo::HEIGHT;
	int lz = z - cz * ChunkInfo::DEPTH;
	return chunk->lightmap->get(lx, ly, lz, channel);
}

Chunk* Chunks::getChunkByBlock(int x, int y, int z) {
	int cx = x / ChunkInfo::WIDTH;
	int cy = y / ChunkInfo::HEIGHT;
	int cz = z / ChunkInfo::DEPTH;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0)
		return nullptr;
	return findChunkByCoordsSafe(cx, cy, cz);
}

Chunk* Chunks::getChunk(int32_t x, int32_t y, int32_t z) {
	if (x < 0 || y < 0 || z < 0)
		return nullptr;
	return chunk_map[hash_xyz(x, y, z)].get();
}

//void Chunks::set(int x, int y, int z, int id) {
//	int cx = x / ChunkInfo::WIDTH;
//	int cy = y / ChunkInfo::HEIGHT;
//	int cz = z / ChunkInfo::DEPTH;
//	if (x < 0) cx--;
//	if (y < 0) cy--;
//	if (z < 0) cz--;
//	if (cx < 0 || cy < 0 || cz < 0 || cx >= w || cy >= h || cz >= d)
//		return;
//	Chunk* chunk = chunks[(cy * d + cz) * w + cx];
//	int lx = x - cx * ChunkInfo::WIDTH;
//	int ly = y - cy * ChunkInfo::HEIGHT;
//	int lz = z - cz * ChunkInfo::DEPTH;
//	chunk->blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx].id = id;
//	// chunk->modified = true;
//
//	if (lx == 0 && (chunk = getChunk(cx - 1, cy, cz))) chunk->modified = true;
//	if (ly == 0 && (chunk = getChunk(cx, cy - 1, cz))) chunk->modified = true;
//	if (lz == 0 && (chunk = getChunk(cx, cy, cz - 1))) chunk->modified = true;
//
//	if (lx == ChunkInfo::WIDTH - 1 && (chunk = getChunk(cx + 1, cy, cz))) chunk->modified = true;
//	if (ly == ChunkInfo::HEIGHT - 1 && (chunk = getChunk(cx, cy + 1, cz))) chunk->modified = true;
//	if (lz == ChunkInfo::DEPTH - 1 && (chunk = getChunk(cx, cy, cz + 1))) chunk->modified = true;
//}