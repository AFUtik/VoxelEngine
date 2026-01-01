
#include "LogicSystem.hpp"
#include <atomic>
#include <memory>
#include <chrono>
#include <shared_mutex>

using namespace glm;

#include <math.h>
#include <limits.h>

LogicSystem::LogicSystem() : noise(0), menger(81, 3), lightSolver(this)  {
	noise.octaves = 2;
    noise.base_freq = 1.0f;
    //noise.lacunarity = 1.2f;

    logicThread = std::thread([this] { logicLoop(); });
    workerThread = std::thread([this] { workerThreadLoop(); });
}

LogicSystem::~LogicSystem() {
    running.store(false, std::memory_order_release);

    if (logicThread.joinable())
        logicThread.join();

    if (workerThread.joinable())
        workerThread.join();
}

void LogicSystem::loadNeighbours(std::shared_ptr<Chunk> chunk) {
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

// SIMPLE GENERATION FUNCTION // 
void LogicSystem::generate(ChunkPtr chunk)
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

void LogicSystem::generate(ChunkSnapshot *chunk)
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

void LogicSystem::updateChunk(ChunkPtr chunk) { chunk->setState(ChunkState::Finished);
    auto snapshot = std::make_unique<ChunkSnapshot>(chunk);
    {
        std::lock_guard lk(readyQueueMutex);
        readyChunks.push_front(std::move(snapshot));
        readyCv.notify_one();
    }
   
}

void LogicSystem::updateLight(ChunkPtr chunk) {
    //lightSolver.calculateLight(chunk);

    updateChunk(chunk);
}



void LogicSystem::unloadChunk(int cx, int cy, int cz) {
    {
        std::shared_lock<std::shared_mutex> mapLock(chunkMapMutex);
        auto it = chunkMap.find({cx, cy, cz});
        if (it == chunkMap.end()) return;
    }
    {
        std::unique_lock<std::shared_mutex> mapLock(chunkMapMutex);
        chunkMap.erase(ChunkPos{cx, cy, cz});
    }
}

void LogicSystem::destroyBlock(int x, int y, int z) {
    auto chunk = getChunkByBlock(x, y, z);
    int lx, ly, lz;
    Chunk::local(lx, ly, lz, x, y, z);

    std::cout << chunk->x << " " << chunk->y << " " << chunk->z << " " << std::endl;

    chunk->setBlock(lx, ly, lz, 0); 
    lightSolver.removeLightLocally(lx, ly, lz, chunk.get());

    chunk->dirty();
    updateChunk(chunk);
}

void LogicSystem::placeBlock(int x, int y, int z) {
    auto chunk = getChunkByBlock(x, y, z);
    int lx, ly, lz;
    Chunk::local(lx, ly, lz, x, y, z);

    std::cout << chunk->x << " " << chunk->y << " " << chunk->z << " " << std::endl;

    chunk->setBlock(lx, ly, lz, 1);
    lightSolver.placeLightLocally(lx, ly, lz, {0, 0, 15, 15}, chunk.get());

    chunk->dirty();
    updateChunk(chunk);
}

block LogicSystem::getBlock(int x, int y, int z) {
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

unsigned char LogicSystem::getLight(int x, int y, int z, int channel) {
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

std::shared_ptr<Chunk> LogicSystem::getChunkByBlock(int x, int y, int z) {
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

std::shared_ptr<Chunk> LogicSystem::getChunk(int x, int y, int z) {
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

void LogicSystem::processAllCommands() {
    std::queue<std::function<void()>> local;
    {
        std::lock_guard<std::mutex> lock(commandMutex);
        std::swap(local, commandQueue);
    }

    while (!local.empty()) {
        local.front()();
        local.pop();
    }
}

void LogicSystem::workerThreadLoop() {
    while(running) {
        std::function<void()> task;
        {
            std::unique_lock lk(taskQueueMutex);
            taskCv.wait(lk, [&] {
                return !running || !tasks.empty();
            });
            if (!running && tasks.empty())
                break;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

void LogicSystem::pushTask(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> m(taskQueueMutex);
        tasks.push(task);
    }
    taskCv.notify_one();
}

void LogicSystem::generateChunk(int x, int y, int z) {
        {
            std::shared_lock<std::shared_mutex> mapLock(chunkMapMutex);
            auto it = chunkMap.find({x, y, z});
            if (it != chunkMap.end()) return;
        }
        ChunkPtr chunk = std::make_shared<Chunk>(x, y, z);
        loadNeighbours(chunk);
        

        auto snapshot = std::make_shared<ChunkSnapshot>(chunk);
        pushTask([this, snapshot] {
            generate(snapshot.get());
            lightSolver.calculateLight(snapshot.get());
            
            {
                std::unique_lock<std::shared_mutex> l(commitMutex);
                toCommit.push(snapshot);
            }
        });
        
        chunkMap.emplace(chunk->hash_pos, chunk);
}

void LogicSystem::logicUpdate(double dt) {
    processAllCommands();
    {
        std::unique_lock<std::shared_mutex> l(commitMutex);
        while(!toCommit.empty()) {
            auto snapshot = toCommit.front(); toCommit.pop();
            snapshot->commit();
            
            lightSolver.calculateLight(snapshot->source.get());
        }
    }

    ivec3 playerChunk = worldToChunk3(loadPlayerPos());
    std::vector<ChunkPtr> toUnload;
    {
        std::shared_lock sl(chunkMapMutex);
        for (auto& [pos, chunk] : chunkMap) {
            if (!insideRadius(playerChunk, pos, loadDistance)) 
            {
                toUnload.push_back(chunk);
            } 
            if(chunk->checkState(ChunkState::NeedsUpdate)) 
            {
                updateChunk(chunk);
            }
        }
    }

    for (auto& chunk : toUnload) unloadChunk(chunk->x, chunk->y, chunk->z);
        
	if (playerChunk != lastPlayerChunk) {
		for (int x = playerChunk.x - loadDistance; x <= playerChunk.x + loadDistance; x++) {
            for (int z = playerChunk.z - loadDistance; z <= playerChunk.z + loadDistance; z++) {
                if(commandQueue.empty()) generateChunk(x, 0, z); 
            }
        }

		lastPlayerChunk = playerChunk;
	}
}

void LogicSystem::logicLoop() {
    using clock = std::chrono::steady_clock;
    constexpr double TICK = 1.0 / 20.0; // 20 TPS

    auto last = clock::now();
    double acc = 0.0;

    while (running)
    {
        auto now = clock::now();
        acc += std::chrono::duration<double>(now - last).count();
        last = now;

        while (acc >= TICK)
        {
            logicUpdate(TICK);
            acc -= TICK;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
