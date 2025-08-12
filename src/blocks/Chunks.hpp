#ifndef CHUNKS_HPP
#define CHUNKS_HPP

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>

#include <memory>

#include <glm/glm.hpp>

#include "Chunk.hpp"

#include "../lighting/LightSolver.hpp"

class Chunk;
struct block;

inline uint64_t hash_xyz(int x, int y, int z) noexcept {
	return (static_cast<uint64_t>(x) * 73856093) ^ (static_cast<uint64_t>(y) * 19349663) ^ (static_cast<uint64_t>(z) * 83492791);
}

class Chunks {
	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> chunk_map;

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

	void write(unsigned char* dest);
	void read(unsigned char* source);
};

#endif // !CHUNKS_HPP
