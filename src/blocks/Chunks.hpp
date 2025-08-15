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
	glm::dvec3 loadDistance = dvec3(500, 50, 500);

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

	inline Chunk* findChunkByCoordsSafe(int cx, int cz) {
		auto it = chunkMap.find(compress_xz(cx, cz));
		return it != chunkMap.end() ? it->second.get() : nullptr;
	}

	void loadNeighbours(Chunk* chunk);

	void unloadChunks(const glm::dvec3 &position);
	void loadChunks  (const glm::dvec3 &position);
public:
	Chunks(int w, int h, int d, bool lighting);

	block getBlock(int x, int y, int z);
	Chunk* getChunk(int x, int z);
	Chunk* getChunkByBlock(int x, int y, int z);
	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);

	/*
     * @param position Basically coords of player.
	 */
	void update(const glm::dvec3 &position);
};

#endif // !CHUNKS_HPP
