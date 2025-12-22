
#include "Chunks.hpp"
#include "Chunk.hpp"
#include "Block.hpp"
#include "ChunkInfo.hpp"
#include "chunk_utils.hpp"
#include "iostream"

#include "../lighting/LightMap.hpp"
#include "../lighting/LightSolver.hpp"

#include <atomic>
#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>

using namespace glm;

#include <math.h>
#include <limits.h>
#include "ChunkCompressor.hpp"

#include "../lighting/LightCompressor.hpp"

Chunks::Chunks(int w, int h, int d, bool lighting) : noise(0), lightSolver(this)  {
	noise.octaves = 2;
    worldWorker = std::thread([this] { workerThread(); });
}

Chunks::~Chunks() {
    {
        std::lock_guard<std::mutex> lk(taskQueueMutex);
        stop_workers = true;
    }
    taskCv.notify_all();
    if (worldWorker.joinable()) worldWorker.join();
}

void Chunks::loadNeighbours(std::shared_ptr<Chunk> chunk) {
    chunk->weak_self = chunk;
    for (int i = 0; i < 26; ++i) {
        int nx = chunk->x + OFFSETS[i][0];
        int ny = chunk->y + OFFSETS[i][1];
        int nz = chunk->z + OFFSETS[i][2];

        auto it = chunkMap.find(ChunkPos{nx, ny, nz});
        if (it != chunkMap.end()) {
            auto& neigh = it->second;

            std::scoped_lock lock(chunk->dataMutex, neigh->dataMutex);
            chunk->loadNeighbour(i,    neigh);
            neigh->loadNeighbour(25-i, chunk);
        }
    }
}

// SIMPLE GENERATION FUNCTION // 
void Chunks::generateChunk(std::shared_ptr<Chunk> chunk) {
    const float scale = 0.02f;
	const float scale2 = 0.02f;
	const float height = 10.0f;
	for (int _z = 0; _z < ChunkInfo::DEPTH; _z++) {
		for (int _x = 0; _x < ChunkInfo::WIDTH; _x++) {
			// Global position //
			const int gx = _x + chunk->x * ChunkInfo::WIDTH;
			const int gz = _z + chunk->z * ChunkInfo::DEPTH;
            
			int y = height * noise.noise(
				static_cast<float>(gx)*scale,
				static_cast<float>(gz)*scale);

			for(int _y = 0; _y < y + 200; _y++) {
				float n = noise.noise(
				static_cast<float>(gx)*scale2,
				static_cast<float>(_y + chunk->y * ChunkInfo::HEIGHT)*scale2,
				static_cast<float>(gz)*scale2
				);
				int id = 1;
				if (n < 0.0005) id = 0;
				
				//if(id != 0 && (_x == ChunkInfo::WIDTH-1 || _z == ChunkInfo::DEPTH-1)) {
				//	chunk->setBlock(_x, _y, _z, 2);
				//	continue;
				//};

				chunk->setBlock(_x, _y, _z, id);
			}
		}
	}
}

void Chunks::pushTask(WorldTaskType task, std::variant<Pos, std::shared_ptr<Chunk>> var) {
    {
        std::unique_lock<std::mutex> m(taskQueueMutex);
        tasks.push({task, var});
    }
    taskCv.notify_one();
}

void Chunks::finishChunk(std::shared_ptr<Chunk> chunk) {
    chunk->setState(ChunkState::Finished); 
    {
        std::lock_guard lk(readyQueueMutex);
        readyChunks.push(chunk);
    }
    readyCv.notify_one();
}

void Chunks::workerThread() {
    while (true) {
        WorldTask task;
        {
            std::unique_lock lk(taskQueueMutex);
            taskCv.wait(lk, [&] {
                return stop_workers || !tasks.empty();
            });
            if (stop_workers && tasks.empty())
                break;
            task = tasks.front();
            tasks.pop();
        }
        switch(task.first) {
            case WorldTaskType::Generate: 
            { 
                std::shared_ptr<Chunk> &chunk = std::get<std::shared_ptr<Chunk>>(task.second);
                if(chunk->state.load(std::memory_order_relaxed) == ChunkState::Removed) continue;

                std::shared_lock<std::shared_mutex> mapLock(chunkMapMutex);
                {
                    if(!chunk->checkState(ChunkState::Empty)) continue;
                    auto it = chunkMap.find(chunk->hash_pos);
                    if (it != chunkMap.end()) continue;
                }
                generateChunk(chunk);
                loadNeighbours(chunk);
                chunkMap.emplace(chunk->position(), chunk);

                chunk->state.store(ChunkState::Generated, std::memory_order_release);
                pushTask(WorldTaskType::GenerateLight, chunk);

                break;
            }
            case WorldTaskType::Unload: 
            {
                std::shared_ptr<Chunk> &chunk = std::get<std::shared_ptr<Chunk>>(task.second);
                if(chunk->state.load(std::memory_order_relaxed) == ChunkState::Removed) continue;

                {
                    std::shared_lock<std::shared_mutex> mapLock(chunkMapMutex);
                    auto it = chunkMap.find(chunk->hash_pos);
                    if (it == chunkMap.end()) continue;
                }
                chunk->clearNeighbours();
                {
                    std::unique_lock<std::shared_mutex> mapLock(chunkMapMutex);
                    chunkMap.erase(chunk->hash_pos);
                }
                chunk->setState(ChunkState::Removed);
                break;
            }
            case WorldTaskType::GenerateLight: {
                std::shared_ptr<Chunk> &chunk = std::get<std::shared_ptr<Chunk>>(task.second);
                if(chunk->state.load(std::memory_order_relaxed) == ChunkState::Removed) continue;

                lightSolver.propagateSunLight(chunk);
                lightSolver.calculateLight(chunk);

                chunk->setState(ChunkState::Lighted);

                break;
            }
            case WorldTaskType::CalculateLight: {
                std::shared_ptr<Chunk> &chunk = std::get<std::shared_ptr<Chunk>>(task.second);
                if(chunk->state.load(std::memory_order_relaxed) == ChunkState::Removed) continue;

                lightSolver.calculateLight(chunk);

                chunk->setState(ChunkState::Lighted);

                break;
            }
            case WorldTaskType::DestroyBlock: {
                Pos pos = std::get<Pos>(task.second);

                auto chunk = getChunkByBlock(pos.x, pos.y, pos.z);
  
                chunk->setBlock(pos.x, pos.y, pos.z, 0);
                chunk->light();

                break;
            }
            default: {
                break;
            }
        }
        if(std::holds_alternative<std::shared_ptr<Chunk>>(task.second)) {
            std::shared_ptr<Chunk> &chunk = std::get<std::shared_ptr<Chunk>>(task.second);
            if(chunk->checkState(ChunkState::Lighted)) finishChunk(chunk);
        }
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
		return chunk->getLight(lx, ly, lz, channel);
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
    ivec3 playerChunk = worldToChunk3(playerPos);
    {
        std::shared_lock<std::shared_mutex> sl(chunkMapMutex);
        for (auto& [pos, chunk] : chunkMap) {
            if (!insideRadius(playerChunk, pos, loadDistance)) pushTask(WorldTaskType::Unload, chunk);
            else {
                switch (chunk->state) {
                    case ChunkState::Finished: {
                        pushTask(WorldTaskType::Finish, chunk);
                    }
                    default: {
                        break;
                    }
                }
            }
        }
    }

	if (playerChunk != lastPlayerChunk) {
		glm::ivec3 delta = playerChunk - lastPlayerChunk;

		if (delta.x != 0) {
			int oldX = lastPlayerChunk.x - delta.x * loadDistance;
			int newX = playerChunk.x + delta.x * loadDistance;

			for (int z = playerChunk.z - loadDistance; z <= playerChunk.z + loadDistance; z++) {
                pushTask(WorldTaskType::Generate, std::make_shared<Chunk>(newX, 0, z));
			}
		}

		if (delta.z != 0) {
			int oldZ = lastPlayerChunk.z - delta.z * loadDistance;
			int newZ = playerChunk.z + delta.z * loadDistance;

			for (int x = playerChunk.x - loadDistance; x <= playerChunk.x + loadDistance; x++) {
                pushTask(WorldTaskType::Generate, std::make_shared<Chunk>(x, 0, newZ));
			}
		}

		lastPlayerChunk = playerChunk;
	}
}