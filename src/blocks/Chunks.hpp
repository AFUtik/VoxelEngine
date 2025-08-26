#ifndef CHUNKS_HPP
#define CHUNKS_HPP

#include <cstdint>
#include <unordered_set>
#include <vector>
#include <map>
#include <unordered_map>

#include <array>
#include <memory>

#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>

#include <glm/glm.hpp>

#include "Chunk.hpp"
#include "ChunkInfo.hpp"
#include <structures/ThreadPool.hpp>

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
	
	std::unordered_set<std::shared_ptr<Chunk>> readyChunksSet;

	/*
	 * Used to not close chunks and store them as compressed chunks using RLE algorithm.
	 */
	// std::unordered_map<uint64_t, std::unique_ptr<ChunkRLE>> rleChunkMap;

	ivec3 lastPlayerChunk = ivec3(0.0);

	LightSolver* solverR = nullptr;
	LightSolver* solverG = nullptr;
	LightSolver* solverB = nullptr;
	LightSolver* solverS = nullptr;

	// multithreading //

	mutable std::shared_mutex chunkMapMutex;
	ThreadPool threadPool;

	std::mutex readyQueueMutex;
    
    std::condition_variable readyCv;
	
	std::unordered_set<ChunkPos, ChunkPosHash> loadingSet;
	std::vector<std::future<void>> generationFutures;

	LightSolver* Chunks::getSolver(int chan) {
		switch(chan) {
			case 0: return solverR;
			case 1: return solverG;
			default: return solverB;
		}
	}

	// Neighbour methods //
	void loadNeighbours(std::shared_ptr<Chunk> chunk);
	
	void processBoundaryBlock(
		Chunk* A, Chunk* B, 
		int ax, int ay, int az, 
		int bx, int by, int bz, 
		std::array<bool, 4> &addedAny);

	void syncBoundaryWithNeigbour(
		Chunk* chunk, Chunk* neighbor, 
		int dir, std::array<bool, 4> &addedAny);

	void calculateLight(Chunk* chunk);
	Chunk* generateChunk(int x, int y, int z);

	void unloadChunk(int x, int y, int z);
	void loadChunk  (int x, int y, int z);

	friend class BlockRenderer;
	friend class ChunkMesher;
public:
	std::queue<std::shared_ptr<Chunk>> readyChunks;
	std::unordered_map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosHash> chunkMap;
	
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
