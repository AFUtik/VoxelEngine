#ifndef CHUNKS_HPP
#define CHUNKS_HPP

#include <cstdint>
#include <unordered_set>
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

enum class WorldTaskType : uint8_t {
	Generate,
	GenerateLight,
	CalculateLight,
	Load,
	Unload,
	Finish,

	CreateBlock,
	DestroyBlock,
};

typedef ChunkPos Pos;
typedef std::pair<WorldTaskType, std::variant<Pos, std::shared_ptr<Chunk>>> WorldTask;

class Chunks {
	PerlinNoise noise;
	int loadDistance = 5;
	
	std::queue<std::shared_ptr<Chunk>> readyChunks;

	std::map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosLess> chunkMap;
	mutable std::shared_mutex chunkMapMutex;

	std::map<ChunkPos, std::shared_ptr<ChunkCompressed>, ChunkPosLess> comprsChunkMap;
	mutable std::shared_mutex comprsChunkMapMutex;

	ivec3 lastPlayerChunk = ivec3(0.0);

	// multithreading //
	std::mutex readyQueueMutex;
    std::condition_variable readyCv;

	// MAIN WORLD WORKER // 
	ChunkState finalStep = ChunkState::Lighted;

	std::thread worldWorker;

	std::queue<WorldTask> tasks;

	std::mutex taskQueueMutex;
	std::condition_variable taskCv;
	bool stop_workers = false;

	void workerThread();

	// Neighbour methods //
	void loadNeighbours(std::shared_ptr<Chunk> chunk);
	Chunk* generateChunk(int x, int y, int z);

	void finishChunk(std::shared_ptr<Chunk> chunk);
	void generateChunk(std::shared_ptr<Chunk> chunk);

	friend class BlockRenderer;
	friend class ChunkMesher;
public:
	void pushTask(WorldTaskType task, std::variant<Pos, std::shared_ptr<Chunk>> var);

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
