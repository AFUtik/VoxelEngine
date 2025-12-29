#ifndef LOGICSYSTEM_HPP
#define LOGICSYSTEM_HPP

// STD
#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <glm/glm.hpp>

// GLM
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/vector_int3.hpp"

// LOGIC
#include "blocks/Chunk.hpp"
#include "blocks/ChunkInfo.hpp"
#include "blocks/ChunkCompressor.hpp"
#include "lighting/LightSolver.hpp"
#include "gen/noise/PerlinNoise.hpp"
#include "gen/MengerSpongeGenerator.hpp"

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

template<typename T>
struct AtomicPos {
    std::atomic<T> x{0};
    std::atomic<T> y{0};
    std::atomic<T> z{0};
};

typedef AtomicPos<double> PlayerPos;

class LogicSystem {
	// GEN //
	MengerSpongeGenerator menger;
	PerlinNoise noise;
	int loadDistance = 4;
	
	std::map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosLess> chunkMap;
	mutable std::shared_mutex chunkMapMutex;

	PlayerPos playerPos;
	glm::ivec3 lastPlayerChunk;

	std::atomic<bool> running = true;

	// multithreading //
	std::thread logicThread;

	std::mutex readyQueueMutex;
	std::queue<std::unique_ptr<ChunkSnapshot>> readyChunks;
    std::condition_variable readyCv;

	std::queue<std::function<void()>> commandQueue;
	std::mutex commandMutex;

	void loadNeighbours(std::shared_ptr<Chunk> chunk);
	void updateChunk(std::shared_ptr<Chunk> chunk);
	void updateLight(std::shared_ptr<Chunk> chunk);
	void generate(std::shared_ptr<Chunk> chunk);

	friend class BlockRenderer;
	friend class Mesher;

	void processAllCommands();
public:
	void enqueueCommand(std::function<void()> cmd)
	{
		std::lock_guard<std::mutex> lock(commandMutex);
		commandQueue.push(std::move(cmd));
	}

	void setPlayerPos(dvec3 pos) {
		playerPos.y.store(pos.y, std::memory_order_relaxed);
		playerPos.x.store(pos.x, std::memory_order_relaxed);
		playerPos.z.store(pos.z, std::memory_order_relaxed);
	}

	glm::dvec3 loadPlayerPos() {
		return glm::dvec3(playerPos.x.load(std::memory_order_relaxed),
						  playerPos.y.load(std::memory_order_relaxed),
		                  playerPos.z.load(std::memory_order_relaxed));
	}

	void generateChunk(int x, int y, int z);
	void unloadChunk  (ChunkPtr chunk);

	void destroyBlock (int x, int y, int z);
	
	//void destroyBlockLocally(int x, int y, int z);
	//void createBlock(int x, int y, int z);
	//void createBlockLocally(int x, int y, int z);

	BasicLightSolver lightSolver;

	LogicSystem();
	~LogicSystem();

	block getBlock(int x, int y, int z);
	ChunkPtr getChunk(int x, int y, int z);
	ChunkPtr getChunkByBlock(int x, int y, int z);

	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);

	void logicLoop();
	void logicFreeze();
	void logicUpdate(double dt);
};

#endif // !CHUNKS_HPP
