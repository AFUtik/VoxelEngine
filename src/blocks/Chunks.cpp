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

void Chunks::loadNeighbours(Chunk* chunk) {
    for (int i = 0; i < 26; ++i) {
        int nx = chunk->x + OFFSETS[i][0];
        int ny = chunk->y + OFFSETS[i][1];
        int nz = chunk->z + OFFSETS[i][2];

        auto it = chunkMap.find(ChunkPos{nx, ny, nz});
        if (it != chunkMap.end()) {
            Chunk* neighbour = it->second.get();
            chunk->neighbors[i] = neighbour;

            // найдем индекс противоположного смещения (или заранее подготовь opposite[i])
            int opp = 25-i;
            if (opp >= 0) neighbour->neighbors[opp] = chunk;
        } else {
            chunk->neighbors[i] = nullptr;
        }
    }
}

Chunks::Chunks(int w, int h, int d, bool lighting) : noise(0)  {
	PerlinNoise noise(0);
	noise.octaves = 2;

	if (lighting) {
		solverR = new LightSolver(this, 0);
		solverG = new LightSolver(this, 1);
		solverB = new LightSolver(this, 2);
		solverS = new LightSolver(this, 3);
	}

	int index = 0;
	for (int y = 0; y < h; y++) {
		for (int z = 0; z < d; z++) {
			for (int x = 0; x < w; x++, index++) {
				auto [it, inserted] = chunkMap.emplace(
					ChunkPos{x,y,z}, 
					std::make_unique<Chunk>(x,y,z,noise)
				);
				Chunk* chunkPtr = it->second.get();
				iterable.push_back(chunkPtr);
			}
		}
	}

	for (auto& [pos, uptr] : chunkMap) {
    	loadNeighbours(uptr.get());
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
}

Chunk* Chunks::generateChunk(int x, int y, int z) {
	auto [it, inserted] = chunkMap.emplace(
					ChunkPos{x,y,z}, 
					std::make_unique<Chunk>(x,y,z,noise)
				);
	Chunk* chunk = it->second.get();
	loadNeighbours(chunk);

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
	
	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();

	return chunk;
}

void Chunks::loadChunk(int x, int y, int z) {
	Chunk* chunk = findChunkByCoordsSafe(x, y, z);
    if (chunk != nullptr) return;

	chunk = generateChunk(x, y, z);
}

void Chunks::unloadChunk(int x, int y, int z) {
	chunkMap.erase(ChunkPos{x, y, z});
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

Chunk* Chunks::getChunk(int x, int y, int z) {
	if (x < 0 ||  z < 0)
		return nullptr;
	return chunkMap[ChunkPos{x, y, z}].get();
}

void Chunks::update(const glm::dvec3 &playerPos) {
	glm::ivec2 playerChunk = worldToChunk(playerPos);

	if (playerChunk != lastPlayerChunk) {
		glm::ivec2 delta = playerChunk - lastPlayerChunk;

		if (delta.x != 0) {
			int oldX = lastPlayerChunk.x - delta.x * loadDistance;
			int newX = playerChunk.x + delta.x * loadDistance;

			for (int z = playerChunk.y - loadDistance; z <= playerChunk.y + loadDistance; z++) {
				unloadChunk(oldX, 0, z);
				loadChunk(newX, 0, z);
			}
		}

		if (delta.y != 0) {
			int oldZ = lastPlayerChunk.y - delta.y * loadDistance;
			int newZ = playerChunk.y + delta.y * loadDistance;

			for (int x = playerChunk.x - loadDistance; x <= playerChunk.x + loadDistance; x++) {
				unloadChunk(x, 0, oldZ);
				loadChunk(x, 0, newZ);
			}
		}

		lastPlayerChunk = playerChunk;
	}
}

