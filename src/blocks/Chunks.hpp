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
#include "ChunkInfo.hpp"

#include "../lighting/LightSolver.hpp"
#include "../noise/PerlinNoise.hpp"

class Chunk;
struct block;

struct ChunkPos {
    int32_t x, y, z;
    bool operator==(const ChunkPos& o) const noexcept {
        return x==o.x && y==o.y && z==o.z;
    }
};

struct ChunkPosHash {
    std::size_t operator()(const ChunkPos& p) const noexcept {
        // FNV-1a style mix (fast and decent)
        uint64_t h = 14695981039346656037ull;
        auto mix = [&](uint32_t v){
            h ^= v;
            h *= 1099511628211ull;
        };
        mix(static_cast<uint32_t>(p.x));
        mix(static_cast<uint32_t>(p.y));
        mix(static_cast<uint32_t>(p.z));
        return static_cast<std::size_t>(h ^ (h >> 32));
    }
};

class Chunks {
	PerlinNoise noise;
	int loadDistance = 5;

	/*
	 * Used to find a chunk near player.
	 */
	std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash> chunkMap;

	/*
	 * Used to not close chunks and store them as compressed chunks using RLE algorithm.
	 */
	// std::unordered_map<uint64_t, std::unique_ptr<ChunkRLE>> rleChunkMap;

	ivec2 lastPlayerChunk = ivec2(0.0);

	LightSolver* solverR = nullptr;
	LightSolver* solverG = nullptr;
	LightSolver* solverB = nullptr;
	LightSolver* solverS = nullptr;

	inline Chunk* findChunkByCoordsSafe(int cx, int cy, int cz) {
		auto it = chunkMap.find(ChunkPos{cx, cy, cz});
		return it != chunkMap.end() ? it->second.get() : nullptr;
	}

	inline ivec3 worldToChunk(const dvec3 &worldPos) {
		return ivec3(worldPos.x/ChunkInfo::WIDTH, worldPos.y/ChunkInfo::HEIGHT, worldPos.y/ChunkInfo::DEPTH);
	}

	void loadNeighbours(Chunk* chunk);

	Chunk* generateChunk(int x, int y, int z);
	void unloadChunk(int x, int y, int z);
	void loadChunk  (int x, int y, int z);

	friend class BlockRenderer;
public:
	std::vector<Chunk*> iterable;

	Chunks(int w, int h, int d, bool lighting);

	block getBlock(int x, int y, int z);
	Chunk* getChunk(int x, int y, int z);
	Chunk* getChunkByBlock(int x, int y, int z);
	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);

	/*
     * @param position Basically coords of player.
	 */
	void update(const glm::dvec3 &position);
};

#endif // !CHUNKS_HPP
