#include "LightSolver.hpp"
#include "LightMap.hpp"
#include "../blocks/Chunks.hpp"
#include "../blocks/Chunk.hpp"
#include "../blocks/Block.hpp"

#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>

const int OFFS[6][3] = {
	{0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0}, {1,0,0}, {-1,0,0}
};

LightSolver::LightSolver(Chunks* chunks, int channel) : chunks(chunks), channel(channel) {
}

void LightSolver::addLocally(int x, int y, int z, uint8_t emission, const std::shared_ptr<Chunk>& chunk) {
	if (emission <= 1)
		return;

	LightEntry entry;
	entry.lx = x;
	entry.ly = y;
	entry.lz = z;
	entry.light = emission;
	entry.chunk = chunk;
	addqueue.write(entry);

	// chunk->modified = true;
	chunk->setLight(x, y, z, channel, entry.light);
}

void LightSolver::addLocally(int x, int y, int z, const std::shared_ptr<Chunk>& chunk) {
	addLocally(x, y, z, chunk->getLight(x, y, z, channel), chunk);
}

void LightSolver::remove(int x, int y, int z) {
	std::shared_ptr<Chunk> chunk = chunks->getChunkByBlock(x, y, z);
	if (chunk == nullptr)
		return;

	int light = chunk->getLight(x - chunk->x * ChunkInfo::WIDTH, y - chunk->y * ChunkInfo::HEIGHT, z - chunk->z * ChunkInfo::DEPTH, channel);
	if (light == 0) {
		return;
	}

	LightEntry entry;
	entry.lx = x;
	entry.ly = y;
	entry.lz = z;
	entry.light = light;
	remqueue.write(entry);

	chunk->setLight(entry.lx - chunk->x * ChunkInfo::WIDTH, entry.ly - chunk->y * ChunkInfo::HEIGHT, entry.lz - chunk->z * ChunkInfo::DEPTH, channel, 0);
}

void LightSolver::solve() {
	while (!remqueue.empty()) {
		LightEntry entry = remqueue.read();

		for (size_t i = 0; i < 6; i++) {
			int x = entry.lx + OFFS[i][0];
			int y = entry.ly + OFFS[i][1];
			int z = entry.lz + OFFS[i][2];
			std::shared_ptr<Chunk> chunk = chunks->getChunkByBlock(x, y, z);
			if (chunk) {
				int light = chunks->getLight(x, y, z, channel);
				if (light != 0 && light == entry.light - 1) {
					LightEntry nentry;
					nentry.lx = x;
					nentry.ly = y;
					nentry.lz = z;
					nentry.light = light;
					remqueue.write(nentry);
					chunk->setLight(x - chunk->x * ChunkInfo::WIDTH, y - chunk->y * ChunkInfo::HEIGHT, z - chunk->z * ChunkInfo::DEPTH, channel, 0);
					// chunk->modified = true;
				}
				else if (light >= entry.light) {
					LightEntry nentry;
					nentry.lx = x;
					nentry.ly = y;
					nentry.lz = z;
					nentry.light = light;
					addqueue.write(nentry);
				}
			}
		}
	}

	while (!addqueue.empty()) {
		LightEntry entry = addqueue.read();

		Chunk* entry_chunk = entry.chunk.lock().get();
		if (!entry_chunk) continue;

		if (entry.light <= 1)
			continue;

		for (size_t i = 0; i < 6; i++) {
			const int x = entry.lx + OFFS[i][0];
			const int y = entry.ly + OFFS[i][1];
			const int z = entry.lz + OFFS[i][2];

			Chunk *chunk = entry_chunk->findNeighbourChunk(x, y, z);
			
			if(!chunk) continue;

			int lx, ly, lz;
			Chunk::local(lx, ly, lz, x, y, z);

			block v = chunk->getBlock(lx, ly, lz);
			unsigned char curLight = chunk->getLight(lx, ly, lz, channel);
			
			if (v.id == 0 && static_cast<int>(curLight) + 2 <= entry.light) {
				chunk->setLight(lx, ly, lz, channel, entry.light - 1);
				LightEntry nentry;
				nentry.lx = lx;
				nentry.ly = ly;
				nentry.lz = lz;
				nentry.light = entry.light - 1;
				nentry.chunk = chunk->weak_self;
				addqueue.write(nentry);
			}
		}
	}
}
