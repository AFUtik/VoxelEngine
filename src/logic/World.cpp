
#include "World.hpp"
#include "logic/blocks/ChunkInfo.hpp"
#include "logic/blocks/chunk_utils.hpp"
#include <atomic>
#include <memory>
#include <chrono>
#include <mutex>
#include <shared_mutex>

using namespace glm;

#include <math.h>
#include <limits.h>

World::World() : noise(0), menger(81, 3), lightSolver(this)  {
	noise.octaves = 2;
    noise.base_freq = 1.0f;
    //noise.lacunarity = 1.2f;
}

World::~World() {}

// BLOCK //
void World::destroyBlock(int x, int y, int z) {
    auto chunk = getChunkByBlock(x, y, z);

    int lx, ly, lz;
    Chunk::local(lx, ly, lz, x, y, z);

    if(!chunk) return;

    chunk->setBlock(lx, ly, lz, 0); 
    lightSolver.removeLightLocally(lx, ly, lz, chunk.get());

    updateChunk(chunk);
}

void World::placeBlock(int x, int y, int z, AbstractBlock &b) {
    auto chunk = getChunkByBlock(x, y, z);
    int lx, ly, lz;
    Chunk::local(lx, ly, lz, x, y, z);

    if(!chunk) return;

    chunk->setBlock(lx, ly, lz, b.getId());
    lightSolver.placeLightLocally(lx, ly, lz, b.getEmission(), chunk.get());

    updateChunk(chunk);
}

block World::getBlock(int x, int y, int z) {
	int cx = floorDiv(x, ChunkInfo::WIDTH);
	int cy = floorDiv(y, ChunkInfo::HEIGHT);
	int cz = floorDiv(z, ChunkInfo::DEPTH);
		
	auto it = chunkMap.find(ChunkPos{cx, cy, cz});
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

	auto it = chunkMap.find(ChunkPos{cx, cy, cz});
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
        int gx = x + chunk->x * (int)ChunkInfo::WIDTH;
        int gz = z + chunk->z * (int)ChunkInfo::DEPTH;
        int gy = y + chunk->y * (int)ChunkInfo::HEIGHT;

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
    auto it = chunkMap.find({x, y, z});
    if (it != chunkMap.end()) return;
        
    ChunkPtr chunk = std::make_shared<Chunk>(x, y, z);
    loadNeighbours(chunk);
    generate(chunk);

    lightSolver.propagateSunLight(chunk.get());
    lightSolver.calculateLight(chunk.get());

    chunkMap.emplace(chunk->hash_pos, chunk);
}

void World::unloadChunk(int cx, int cy, int cz) {
    auto it = chunkMap.find({cx, cy, cz});
    if (it == chunkMap.end()) return;
    
    chunkMap.erase(ChunkPos{cx, cy, cz});
}

std::shared_ptr<Chunk> World::getChunkByBlock(int x, int y, int z) {
	int cx = floorDiv(x, ChunkInfo::WIDTH);
	int cy = floorDiv(y, ChunkInfo::HEIGHT);
	int cz = floorDiv(z, ChunkInfo::DEPTH);
	auto it = chunkMap.find(ChunkPos{cx, cy, cz});
	if (it == chunkMap.end()) return nullptr;
	return it->second;
}

std::shared_ptr<Chunk> World::getChunk(int x, int y, int z) {
	ChunkPos key{x, y, z};
	auto it = chunkMap.find(key);
	if (it == chunkMap.end()) return nullptr;
	return it->second;
}

void World::loadNeighbours(std::shared_ptr<Chunk> chunk) {
    for (int i = 0; i < 26; ++i) {
        int nx = chunk->x + OFFSETS[i][0];
        int ny = chunk->y + OFFSETS[i][1];
        int nz = chunk->z + OFFSETS[i][2];

        auto it = chunkMap.find(ChunkPos{nx, ny, nz});
        if (it != chunkMap.end()) {
            auto& neigh = it->second;

            chunk->loadNeighbour(i,    neigh.get());
            neigh->loadNeighbour(25-i, chunk.get());
        }
    }
}

// WORLD LOADING //

void World::load(double x, double y, double z) {
    int cx = floorDiv(x, ChunkInfo::WIDTH);
    int cy = floorDiv(y, ChunkInfo::HEIGHT);
    int cz = floorDiv(z, ChunkInfo::DEPTH);

    auto c = getChunk(cx, cy, cz);
    std::vector<ChunkPtr> toUnload;
    for (auto& [pos, chunk] : chunkMap) {
        if (!((abs(x - pos.x) <= loadDistance) &&
              (abs(y - pos.y) <= loadDistance) &&
              (abs(z - pos.z) <= loadDistance))) 
        {
            toUnload.push_back(chunk);
        }
    }
    for (auto& chunk : toUnload) unloadChunk(chunk->x, chunk->y, chunk->z);
       
	if (cx != last_cx || cy != last_cy || cz != last_cz) {
		for (int x = cx - loadDistance; x <= cx + loadDistance; x++) {
            for (int z = cz - loadDistance; z <= cz + loadDistance; z++) {
                generateChunk(x, 0, z);
            }
        }
		last_cx = cx;
        last_cy = cy;
        last_cz = cz;
	}
}