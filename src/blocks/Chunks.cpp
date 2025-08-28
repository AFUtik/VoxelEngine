#include "Chunks.hpp"
#include "Chunk.hpp"
#include "Block.hpp"
#include "ChunkInfo.hpp"
#include "iostream"

#include "../lighting/LightMap.hpp"
#include "../lighting/LightSolver.hpp"
#include "structures/ThreadPool.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>

using namespace glm;

#include <math.h>
#include <limits.h>
#include "ChunkRLE.hpp"

Chunks::Chunks(int w, int h, int d, bool lighting) : noise(0), threadPool(1), lightSolver(this)  {
	noise.octaves = 2;
}

void Chunks::loadNeighbours(std::shared_ptr<Chunk> chunk) {
    for (int i = 0; i < 26; ++i) {
        int nx = chunk->x + OFFSETS[i][0];
        int ny = chunk->y + OFFSETS[i][1];
        int nz = chunk->z + OFFSETS[i][2];

        auto it = chunkMap.find(ChunkPos{nx, ny, nz});
        if (it != chunkMap.end()) {
            auto& neigh = it->second;

            std::scoped_lock lock(chunk->dataMutex, neigh->dataMutex);
            chunk->neighbors[i]      = neigh;
            neigh->neighbors[25 - i] = chunk;
        }
    }
}

Chunk* Chunks::generateChunk(int cx, int cy, int cz) {
	auto [it, inserted] = chunkMap.emplace(
					ChunkPos{cx,cy,cz}, 
					std::make_unique<Chunk>(cx,cy,cz,noise)
				);
	Chunk* chunk = it->second.get();
	return chunk;
}

void Chunks::loadChunk(int x, int y, int z) {
    ChunkPos pos{x, y, z};
    {
        std::shared_lock<std::shared_mutex> sl(chunkMapMutex);
        if (chunkMap.find(pos) != chunkMap.end()) return;
    }

    {
        std::unique_lock<std::shared_mutex> ul(chunkMapMutex);
        if (loadingSet.find(pos) != loadingSet.end()) return;
        loadingSet.insert(pos);
    }
    auto fut = threadPool.submit([this, pos]() {
    	std::shared_ptr<Chunk> sptr = std::make_shared<Chunk>(pos.x, pos.y, pos.z, noise);
        sptr->weak_self = sptr;
        {
            std::shared_lock<std::shared_mutex> sl(chunkMapMutex);
            loadNeighbours(sptr);
        }

        lightSolver.propagateSunLight(sptr);
        lightSolver.calculateLight(sptr);
        
        {
            std::unique_lock<std::shared_mutex> mapLock(chunkMapMutex);
            auto it = chunkMap.find(pos);
            if (it == chunkMap.end()) {
                chunkMap.emplace(pos, sptr);
            } else {
                loadingSet.erase(pos);
                return;
            }
        }
        
        {
            std::lock_guard<std::mutex> ql(readyQueueMutex);
            readyChunks.push(sptr);
            readyCv.notify_one();
        }

        {
            std::unique_lock<std::shared_mutex> mapLock(chunkMapMutex);
            loadingSet.erase(pos);
        }
        
    });

    generationFutures.emplace_back(std::move(fut));
}

void Chunks::unloadChunk(int x, int y, int z) {
	ChunkPos key{x,y,z};
    std::shared_ptr<Chunk> sptr;
    {
        std::shared_lock<std::shared_mutex> mapLock(chunkMapMutex);
        auto it = chunkMap.find(key);
        if (it == chunkMap.end()) return;
        sptr = it->second;
    }
    sptr->clearNeighbours();
    {
        std::unique_lock<std::shared_mutex> mapLock(chunkMapMutex);
        chunkMap.erase(key);
    }
}


block Chunks::getBlock(int x, int y, int z) {
	{
		std::shared_lock<std::shared_mutex> sl(chunkMapMutex);
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
}

unsigned char Chunks::getLight(int x, int y, int z, int channel) {
	{
		std::shared_lock<std::shared_mutex> sl(chunkMapMutex);

		int cx = floorDiv(x, ChunkInfo::WIDTH);
		int cy = floorDiv(y, ChunkInfo::HEIGHT);
		int cz = floorDiv(z, ChunkInfo::DEPTH);
		auto it = chunkMap.find(ChunkPos{cx, cy, cz});
		if (it == chunkMap.end()) return 0;
		Chunk* chunk = it->second.get();
		int lx = x - cx * ChunkInfo::WIDTH;
		int ly = y - cy * ChunkInfo::HEIGHT;
		int lz = z - cz * ChunkInfo::DEPTH;
		return chunk->lightmap->get(lx, ly, lz, channel);
	}
	
}

std::shared_ptr<Chunk> Chunks::getChunkByBlock(int x, int y, int z) {
	{
		std::shared_lock<std::shared_mutex> sl(chunkMapMutex);

		int cx = floorDiv(x, ChunkInfo::WIDTH);
		int cy = floorDiv(y, ChunkInfo::HEIGHT);
		int cz = floorDiv(z, ChunkInfo::DEPTH);
		auto it = chunkMap.find(ChunkPos{cx, cy, cz});
		if (it == chunkMap.end()) return nullptr;
		return it->second;
	}
}

std::shared_ptr<Chunk> Chunks::getChunk(int x, int y, int z) {
	{
		std::shared_lock<std::shared_mutex> sl(chunkMapMutex);

		ChunkPos key{x, y, z};
		auto it = chunkMap.find(key);
		if (it == chunkMap.end()) return nullptr;
		return it->second;
	}
}

bool insideRadius(const ivec3 &center, const ChunkPos &p, int radius) {
    return (abs(center.x - p.x) <= radius) &&
           (abs(center.y - p.y) <= radius) &&
           (abs(center.z - p.z) <= radius);
}

void Chunks::update(const glm::dvec3 &playerPos) {
    generationFutures.erase(
    std::remove_if(generationFutures.begin(), generationFutures.end(),
                   [](std::future<void>& f){ return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }),
    generationFutures.end());
    
    ivec3 playerChunk = worldToChunk3(playerPos);

    std::vector<ChunkPos> toUnload;
    {
        std::shared_lock<std::shared_mutex> sl(chunkMapMutex);
        for (auto& [pos, chunk] : chunkMap) {
            if (!insideRadius(playerChunk, pos, loadDistance)) toUnload.push_back(pos);
        }
    }
    for (auto& pos : toUnload) unloadChunk(pos.x, pos.y, pos.z);
    
	if (playerChunk != lastPlayerChunk) {
		glm::ivec3 delta = playerChunk - lastPlayerChunk;

		if (delta.x != 0) {
			int oldX = lastPlayerChunk.x - delta.x * loadDistance;
			int newX = playerChunk.x + delta.x * loadDistance;

			for (int z = playerChunk.z - loadDistance; z <= playerChunk.z + loadDistance; z++) {
				loadChunk(newX, 0, z);
			}
		}

		if (delta.z != 0) {
			int oldZ = lastPlayerChunk.z - delta.z * loadDistance;
			int newZ = playerChunk.z + delta.z * loadDistance;

			for (int x = playerChunk.x - loadDistance; x <= playerChunk.x + loadDistance; x++) {
				loadChunk(x, 0, newZ);
			}
		}

		lastPlayerChunk = playerChunk;
	}
}

