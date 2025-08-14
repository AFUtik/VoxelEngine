#ifndef CHUNKS_HPP
#define CHUNKS_HPP

#include <cstdint>
#include <unordered_set>
#include <vector>
#include <map>
#include <unordered_map>

#include <memory>

#include <glm/glm.hpp>

#include "Chunk.hpp"

#include "../lighting/LightSolver.hpp"
#include "blocks/ChunkInfo.hpp"

class Chunk;
struct block;

inline uint64_t compress_xz(int32_t x, int32_t z) {
	return ((long)x & 0xFFFFFFFFL) | (((long)z & 0xFFFFFFFFL) << 32);
}

class Chunks {
	/*
	 * Used to find a chunk near player.
	 */
	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> chunkMap;

	/*
	 * Used to not close chunks and store them as compressed chunks using RLE algorithm.
	 */
	std::unordered_map<uint64_t, std::unique_ptr<ChunkRLE>> rleChunkMap;


	LightSolver* solverR = nullptr;
	LightSolver* solverG = nullptr;
	LightSolver* solverB = nullptr;
	LightSolver* solverS = nullptr;

	inline Chunk* findChunkByCoordsSafe(int cx, int cy, int cz) {
		auto it = chunk_map.find(hash_xyz(cx, cy, cz));
		return it != chunk_map.end() ? it->second.get() : nullptr;
	}

	inline void loadNeighbours(Chunk* chunk) {
		for (int i = 0; i < 26; i++) {
			int nx = chunk->x + OFFSETS[i][0];
			int ny = chunk->y + OFFSETS[i][1];
			int nz = chunk->z + OFFSETS[i][2];

			auto it = chunk_map.find(hash_xyz(nx, ny, nz));
			if (it != chunk_map.end()) {
				Chunk* neighbour = it->second.get();

				chunk->neighbors[i] = neighbour;
				neighbour->neighbors[25-i] = chunk;
			} else {
				chunk->neighbors[i] = nullptr;
			}
		}
	}
public:
	std::vector<Chunk*> iterable;
	Chunks(int w, int h, int d, bool lighting);

	block getBlock(int x, int y, int z);
	Chunk* getChunk(int x, int y, int z);
	Chunk* getChunkByBlock(int x, int y, int z);
	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);


};

#endif // !CHUNKS_HPP
