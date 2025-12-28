
#include "LogicSystem.hpp"
#include <atomic>

using namespace glm;

#include <math.h>
#include <limits.h>

LogicSystem::LogicSystem() : noise(0), menger(81, 3), lightSolver(this)  {
	noise.octaves = 2;
    noise.base_freq = 1.0f;
    //noise.lacunarity = 1.2f;
    logicThread = std::thread([this] { logicLoop(); });
}

LogicSystem::~LogicSystem() {
    running.store(false, std::memory_order_release);

    if (logicThread.joinable())
        logicThread.join();
}

void LogicSystem::loadNeighbours(std::shared_ptr<Chunk> chunk) {
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
void LogicSystem::generate(ChunkPtr chunk)
{
    const float scale  = 0.019873f;
    const float scale2 = 0.04f;
    const int   height = 40;

    for (int z = 0; z < ChunkInfo::DEPTH; z++) {
        for (int x = 0; x < ChunkInfo::WIDTH; x++) {
            int gx = x + chunk->x * (int)ChunkInfo::WIDTH;
            int gz = z + chunk->z * (int)ChunkInfo::DEPTH;

            float h = noise.detail(gx*scale, gz*scale);
            h = (h + 1.0f) * 0.5f;

            int groundY = static_cast<int>(h * height);
            int maxY = std::min(groundY + 65, (int)ChunkInfo::HEIGHT);

            for (int y = 0; y < maxY; y++) {
                int gy = y + chunk->y * ChunkInfo::HEIGHT;
//
                float cave = noise.noise(
                    gx * scale2,
                    gy * scale2,
                    gz * scale2
                );
//
                int id = (cave > 0.4f) ? 0 : 1;
                chunk->setBlock(x, y, z, id);
            }
        }
    }
}

void LogicSystem::updateChunk(ChunkPtr chunk) {
    bool needNotify = false;
    if (chunk->state.load(std::memory_order_acquire) != ChunkState::Finished) {
        chunk->setState(ChunkState::Finished);
        needNotify = true;
    }
    if (needNotify) {
        std::lock_guard lk(readyQueueMutex);
        readyChunks.push(chunk);
        readyCv.notify_one();
    }
}

void LogicSystem::updateLight(ChunkPtr chunk) {
    //lightSolver.calculateLight(chunk);

    updateChunk(chunk);
}

void LogicSystem::generateChunk(int x, int y, int z) {
        {
            std::shared_lock<std::shared_mutex> mapLock(chunkMapMutex);
            auto it = chunkMap.find({x, y, z});
            if (it != chunkMap.end()) return;
        }
        
        ChunkPtr chunk = std::make_shared<Chunk>(x, y, z);
        generate(chunk);
        loadNeighbours(chunk);
    
        lightSolver.propagateSunLight(chunk);
        lightSolver.calculateLight(chunk);
        
        chunkMap.emplace(chunk->position(), chunk);
        
        updateChunk(chunk);
}

void LogicSystem::unloadChunk(ChunkPtr chunk) {
        {
            std::shared_lock<std::shared_mutex> mapLock(chunkMapMutex);
            auto it = chunkMap.find(chunk->hash_pos);
            if (it == chunkMap.end()) return;
        }
        chunk->clearNeighbours();
        {
            std::unique_lock<std::shared_mutex> mapLock(chunkMapMutex);
            chunkMap.erase(chunk->hash_pos);
        }
        chunk->setState(ChunkState::Removed);
}

void LogicSystem::destroyBlock(int x, int y, int z) {


        auto chunk = getChunkByBlock(x, y, z);
        int lx, ly, lz;
        Chunk::local(lx, ly, lz, x, y, z);

        std::cout << chunk->x << " " << chunk->y << " " << chunk->z << " " << std::endl;

        chunk->setBlock(lx, ly, lz, 0); 
        lightSolver.removeLightLocally(lx, ly, lz, chunk);

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

void LogicSystem::logicUpdate(double dt) {
    processAllCommands();

    ivec3 playerChunk = worldToChunk3(loadPlayerPos());
    std::vector<ChunkPtr> toUnload;
    {
        std::shared_lock sl(chunkMapMutex);
        for (auto& [pos, chunk] : chunkMap) {
            if (!insideRadius(playerChunk, pos, loadDistance))
                toUnload.push_back(chunk);
            else if(chunk->checkState(ChunkState::NeedsUpdate)) updateChunk(chunk);
        }
    }

    for (auto& chunk : toUnload)
        unloadChunk(chunk);

	if (playerChunk != lastPlayerChunk) {
		for (int x = playerChunk.x - loadDistance; x <= playerChunk.x + loadDistance; x++) {
            for (int z = playerChunk.z - loadDistance; z <= playerChunk.z + loadDistance; z++) {
                //for (int y = playerChunk.y - loadDistance; y <= playerChunk.y + loadDistance; y++) {
                    generateChunk(x, 0, z);
                //}
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
