#ifndef WORLD_HPP
#define WORLD_HPP

// STD
#include <map>
#include <memory>

// LOGIC
#include "blocks/Chunk.hpp"
#include "blocks/ChunkCompressor.hpp"
#include "lighting/LightSolver.hpp"
#include "gen/noise/PerlinNoise.hpp"
#include "gen/MengerSpongeGenerator.hpp"
#include "blocks/AbstractBlock.hpp"
#include "physics/EntitySystem.hpp"

class Chunk;
struct block;

class World {
	BasicLightSolver lightSolver;

	MengerSpongeGenerator menger;
	PerlinNoise noise;
	int loadDistance = 2;
	
	std::map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosLess> chunkMap;
	
	int last_cx = 0, last_cy = 0, last_cz = 0;

	void loadNeighbours(std::shared_ptr<Chunk> chunk);
	void updateChunk(std::shared_ptr<Chunk> chunk);
	void updateLight(std::shared_ptr<Chunk> chunk);

	void generate(std::shared_ptr<Chunk> chunk);
	void generate(ChunkSnapshot *chunk);

	friend class BlockRenderer;
	friend class Mesher;
public:
	const std::map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosLess>& getChunkMap() {
		return chunkMap;
	}

	void generateChunk(int cx, int cy, int cz);
	void unloadChunk  (int cx, int cy, int cz);

	void destroyBlock (int gx, int gy, int gz);
	void placeBlock   (int gx, int gy, int gz, AbstractBlock &b);

	World();
	~World();

	block getBlock(int x, int y, int z);
	ChunkPtr getChunk(int x, int y, int z);
	ChunkPtr getChunkByBlock(int x, int y, int z);

	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);

	void load(double x, double y, double z);
};

#endif // !CHUNKS_HPP
