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
	BasicLightSolver lightSolver;

	MengerSpongeGenerator menger;
	PerlinNoise noise;
	
	std::map<Vector3I, ChunkPtr> chunkMap;
	Vector3I lastPlayerChunk = Vector3I(INT_MIN, 0, INT_MIN);

	int loadDistance = 3;

	void loadNeighbours(ChunkPtr chunk);
	void generate(ChunkPtr chunk);
	
	friend class BlockRenderer;
	friend class Mesher;
public:
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
