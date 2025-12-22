#ifndef CHUNKS_HPP
#define CHUNKS_HPP

#include <cstdint>
#include <functional>
#include <map>

#include <memory>

#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <queue>

#include <glm/glm.hpp>
#include <variant>

#include "Chunk.hpp"
#include "ChunkInfo.hpp"

#include "../lighting/LightSolver.hpp"
#include "../noise/PerlinNoise.hpp"
#include "ChunkCompressor.hpp"

class Chunk;
struct block;

inline glm::ivec3 worldToChunk3(glm::dvec3 pos) {
    return { floorDiv(pos.x, ChunkInfo::WIDTH),
             floorDiv(pos.y, ChunkInfo::HEIGHT),
             floorDiv(pos.z, ChunkInfo::DEPTH) };
}

struct BlockCommand {
	int x, y, z;
	int newId;
};

typedef ChunkPos Pos;

class Chunks {
	PerlinNoise noise;
	int loadDistance = 5;
	
	std::queue<std::shared_ptr<Chunk>> readyChunks;

	std::map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosLess> chunkMap;
	mutable std::shared_mutex chunkMapMutex;

	ivec3 lastPlayerChunk = ivec3(0.0);

	// multithreading //
	std::thread worldWorker;
	std::mutex readyQueueMutex;
    std::condition_variable readyCv;
	
	std::queue<BlockCommand> blockCommands;
	std::queue<std::function<void()>> tasks;

	std::mutex taskQueueMutex;
	std::condition_variable taskCv;
	bool stop_workers = false;

	void workerThread();

	// Neighbour methods //
	void loadNeighbours(std::shared_ptr<Chunk> chunk);
	void updateChunk(std::shared_ptr<Chunk> chunk);
	void updateLight(std::shared_ptr<Chunk> chunk);
	void generate(std::shared_ptr<Chunk> chunk);

	friend class BlockRenderer;
	friend class ChunkMesher;
public:
	void pushTask(std::function<void()> task);

	void generateChunk(int x, int y, int z);
	void unloadChunk  (ChunkPtr chunk);
	void destroyBlock (int x, int y, int z);
	
	//void destroyBlockLocally(int x, int y, int z);
	//void createBlock(int x, int y, int z);
	//void createBlockLocally(int x, int y, int z);

	BasicLightSolver lightSolver;

	Chunks(int w, int h, int d, bool lighting);
	~Chunks();

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
