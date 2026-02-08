
#include "World.hpp"
#include "blocks/ChunkInfo.hpp"
#include "blocks/chunk_utils.hpp"

using namespace glm;

#include <math.h>
#include <limits.h>

World::World(int loadDistance) : noise(0), menger(81, 3), lightSolver()  {
	noise.octaves = 2;
    noise.base_freq = 1.0f;
    //noise.lacunarity = 1.2f;
}

World::~World() {}

// BLOCK //
block World::getBlock(int x, int y, int z) {
	int cx = floorDiv(x, ChunkInfo::WIDTH);
	int cy = floorDiv(y, ChunkInfo::HEIGHT);
	int cz = floorDiv(z, ChunkInfo::DEPTH);
		
	auto it = chunkMap.find(Vector3I(cx, cy, cz));
	if (it == chunkMap.end()) return block{0};
	Chunk* chunk = it->second.get();

	int lx = x - cx * ChunkInfo::WIDTH;
	int ly = y - cy * ChunkInfo::HEIGHT;
	int lz = z - cz * ChunkInfo::DEPTH;
	return chunk->getBlock(lx, ly, lz);
}

// LIGHT //
unsigned char World::getLight(int x, int y, int z, int channel) {
	int cx = floorDiv(x, ChunkInfo::WIDTH);
	int cy = floorDiv(y, ChunkInfo::HEIGHT);
	int cz = floorDiv(z, ChunkInfo::DEPTH);

	auto it = chunkMap.find(Vector3I{cx, cy, cz});
	if (it == chunkMap.end()) return 0;
	Chunk* chunk = it->second.get();

	int lx = x - cx * ChunkInfo::WIDTH;
	int ly = y - cy * ChunkInfo::HEIGHT;
	int lz = z - cz * ChunkInfo::DEPTH;
	return chunk->getLight(lx, ly, lz, channel);
	
}

// CHUNK MANIPULATION //

// SIMPLE GENERATION FUNCTION // 
void World::generate(ChunkPtr chunk)
{
    const float scale  = 0.019873f;
    const float scale2 = 0.04f;
    const int   height = 40;

    for (int z = 0; z < ChunkInfo::DEPTH;  z++) 
    for (int x = 0; x < ChunkInfo::WIDTH;  x++) 
    for (int y = 0; y < ChunkInfo::HEIGHT; y++) 
    {
        int gx = x + chunk->pos.x * (int)ChunkInfo::WIDTH;
        int gz = z + chunk->pos.z * (int)ChunkInfo::DEPTH;
        int gy = y + chunk->pos.y * (int)ChunkInfo::HEIGHT;

        float n = noise.noise(
            gx * scale2,
            gy * scale2,
            gz * scale2
        );
        int id = 0;
        if(n>0.3f) id = 1;
        if(id) chunk->setBlock(x, y, z, id);
    }
}

void World::generateChunk(int x, int y, int z) {
    auto it = chunkMap.find(Vector3I{x, y, z});
    if (it != chunkMap.end()) return;
    
    ChunkPtr chunk = std::make_shared<Chunk>(Vector3I{x, y, z});
    {
        std::unique_lock<std::shared_mutex> lock(chunkThreadMapMutex);
        chunkThreadMap.emplace(chunk->pos, chunk);
    }

    loadNeighbours(chunk);
    generate(chunk);

    lightSolver.propagateSunLight(chunk);
    lightSolver.calculateLight(chunk);

    {
        std::unique_lock<std::shared_mutex> lock(chunkMapMutex);
        chunkMap.emplace(chunk->pos, chunk);
    }
    {
        std::unique_lock<std::shared_mutex> lock(chunkThreadMapMutex);
        chunkThreadMap.erase(chunk->pos);
    }
}

void World::unloadChunk(int cx, int cy, int cz) {
    auto it = chunkMap.find({cx, cy, cz});
    if (it == chunkMap.end()) return;
    
    it->second->unlinkAll();
    chunkMap.erase({cx, cy, cz});
}

std::shared_ptr<Chunk> World::getChunkByBlock(int x, int y, int z) {
	int cx = floorDiv(x, ChunkInfo::WIDTH);
	int cy = floorDiv(y, ChunkInfo::HEIGHT);
	int cz = floorDiv(z, ChunkInfo::DEPTH);
	auto it = chunkMap.find(Vector3I{cx, cy, cz});
	if (it == chunkMap.end()) return nullptr;
	return it->second;
}

std::shared_ptr<Chunk> World::getChunk(int x, int y, int z) {
	auto it = chunkMap.find(Vector3I{x, y, z});
	if (it == chunkMap.end()) return nullptr;
	return it->second;
}

ChunkPtr World::getChunkOutThreadSafe(Vector3I pos) {
    std::shared_lock<std::shared_mutex> read(chunkThreadMapMutex);
    auto it = chunkThreadMap.find(pos);
    if (it != chunkThreadMap.end()) return it->second;
    return {};
}

ChunkPtr World::getChunkSafe(Vector3I pos) {
    std::shared_lock<std::shared_mutex> read(chunkMapMutex);
    auto it = chunkMap.find(pos);
    if (it != chunkMap.end()) return it->second;
    return {};
}

void World::loadNeighbours(ChunkPtr chunk) {
    for (int i = 0; i < 6; ++i) {
        int nx = chunk->pos.x + FACE_DIRS[i][0];
        int ny = chunk->pos.y + FACE_DIRS[i][1];
        int nz = chunk->pos.z + FACE_DIRS[i][2];

        if (ChunkPtr neigh = getChunkSafe({nx, ny, nz})) {
            chunk->loadNeighbour(i, neigh);
        }
        else {
            if (ChunkPtr neighOutThread = getChunkOutThreadSafe({ nx, ny, nz })) {
                chunk->loadNeighbour(i, neighOutThread);
            }
        }
    }
}

void World::loadWithDistance(double x, double y, double z) {
    Vector3I playerChunk = { floorDiv(x, ChunkInfo::WIDTH), floorDiv(y, ChunkInfo::HEIGHT), floorDiv(z, ChunkInfo::DEPTH) };

    auto c = getChunk(playerChunk.x, playerChunk.y, playerChunk.z);
    std::vector<ChunkPtr> toUnload;

	if (playerChunk != lastPlayerChunk) {
        for (auto& [pos, chunk] : chunkMap) {
            if (!((abs(x - pos.x) <= loadDistance) &&
                  (abs(y - pos.y) <= loadDistance) &&
                  (abs(z - pos.z) <= loadDistance))) 
            {
                toUnload.push_back(chunk);
            }
        }
        for (auto& chunk : toUnload) unloadChunk(chunk->pos.x, chunk->pos.y, chunk->pos.z);

        //for (int x = playerChunk.x - loadDistance; x <= playerChunk.x + loadDistance; x++) {
        //    for (int z = playerChunk.z - loadDistance; z <= playerChunk.z + loadDistance; z++) {
        //        generateChunk(x, 0, z);
        //    }
        //}

        lastPlayerChunk = playerChunk;
	}
}