#ifndef WORLD_HPP
#define WORLD_HPP

// STD
#include <ptypes.hpp>

// LOGIC
#include "blocks/Chunk.hpp"
#include "lighting/LightSolver.hpp"
#include "gen/noise/PerlinNoise.hpp"
#include "gen/MengerSpongeGenerator.hpp"
#include "blocks/AbstractBlock.hpp"
#include "physics/EntitySystem.hpp"

class Chunk;
struct block;

class World {
	unordered_map<Vector3I, ChunkPtr> chunkThreadMap;
	std::shared_mutex chunkThreadMapMutex;

	unordered_map<Vector3I, ChunkPtr> chunkMap;
	std::shared_mutex chunkMapMutex;

	BasicLightSolver lightSolver;

	MengerSpongeGenerator menger;
	PerlinNoise noise;

	Vector3I lastPlayerChunk = Vector3I(INT_MIN, 0, INT_MIN);

	int loadDistance = 8;

	void loadNeighbours(ChunkPtr chunk);
	void generate(ChunkPtr chunk);
	
	friend class IntergratedServer;

	ChunkPtr getChunkOutThreadSafe(Vector3I pos);
public:
	ChunkPtr getChunkSafe(Vector3I pos);

	const auto& getChunkMap() {return chunkMap;}

	void generateChunk(int cx, int cy, int cz);
	void unloadChunk  (int cx, int cy, int cz);

	void destroyBlock (int gx, int gy, int gz);
	void placeBlock   (int gx, int gy, int gz, AbstractBlock &b);

	World(int loadDisntace);
	~World();

	block getBlock(int x, int y, int z);
	ChunkPtr getChunk(int x, int y, int z);
	ChunkPtr getChunkByBlock(int x, int y, int z);

	unsigned char getLight(int x, int y, int z, int channel);

	void loadWithDistance(double x, double y, double z);
};

#endif // !CHUNKS_HPP
