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

inline glm::ivec3 worldToChunk3(glm::dvec3 pos) {
    return { floorDiv(pos.x, ChunkInfo::WIDTH),
             floorDiv(pos.y, ChunkInfo::HEIGHT),
             floorDiv(pos.z, ChunkInfo::DEPTH) };
}

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

struct ChunkPosLess {
    bool operator()(const ChunkPos& a, const ChunkPos& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }
};

class Chunks {
	PerlinNoise noise;
	int loadDistance = 10;
	
	std::queue<std::shared_ptr<Chunk>> readyChunks;
	std::map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosLess> chunkMap;

	

	/*
	 * Used to find a chunk near player.
	 */
	
	std::unordered_set<std::shared_ptr<Chunk>> readyChunksSet;

	ivec3 lastPlayerChunk = ivec3(0.0);

	// multithreading //

	mutable std::shared_mutex chunkMapMutex;
	ThreadPool threadPool;

	std::mutex readyQueueMutex;
    
    std::condition_variable readyCv;
	
	std::unordered_set<ChunkPos, ChunkPosHash> loadingSet;
	std::vector<std::future<void>> generationFutures;

	// Neighbour methods //
	void loadNeighbours(std::shared_ptr<Chunk> chunk);
	
	Chunk* generateChunk(int x, int y, int z);

	void unloadChunk(int x, int y, int z);
	void loadChunk  (int x, int y, int z);

	friend class BlockRenderer;
	friend class ChunkMesher;
public:
	BasicLightSolver lightSolver;

	Chunks(int w, int h, int d, bool lighting);

	block getBlock(int x, int y, int z);
	std::shared_ptr<Chunk> getChunk(int x, int y, int z);
	std::shared_ptr<Chunk> getChunkByBlock(int x, int y, int z);
	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);

	/*
     * @param position Basically coords of player.
	 */
	void update(const glm::dvec3 &position);
};

#endif // !CHUNKS_HPP
