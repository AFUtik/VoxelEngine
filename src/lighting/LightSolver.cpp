#include "LightSolver.hpp"
#include "LightMap.hpp"
#include "../blocks/Chunks.hpp"
#include "../blocks/Chunk.hpp"
#include "../blocks/Block.hpp"

#include <iostream>
#include <memory>
#include <mutex>

LightSolver::LightSolver(Chunks* chunks, int channel) : chunks(chunks), channel(channel) {
}

void LightSolver::addLocally(int x, int y, int z, uint8_t emission, Chunk* chunk) {
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
	{  
		std::lock_guard<std::shared_mutex> m(chunk->dataMutex);
		chunk->setLight(x, y, z, channel, entry.light);
	}
}

void LightSolver::addLocally(int x, int y, int z, Chunk* chunk) {
	addLocally(x, y, z, chunk->getLight(x, y, z, channel), chunk);
}

void LightSolver::remove(int x, int y, int z) {
	Chunk* chunk = chunks->getChunkByBlock(x, y, z);
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

const int OFFS[6][3] = {
	{0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0}, {1,0,0}, {-1,0,0}
};

void LightSolver::solve() {
	while (!remqueue.empty()) {
		LightEntry entry = remqueue.read();

		for (size_t i = 0; i < 6; i++) {
			int x = entry.lx + OFFS[i][0];
			int y = entry.ly + OFFS[i][1];
			int z = entry.lz + OFFS[i][2];
			Chunk* chunk = chunks->getChunkByBlock(x, y, z);
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

		if (entry.light <= 1)
			continue;

		for (size_t i = 0; i < 6; i++) {
			const int x = entry.lx + OFFS[i][0];
			const int y = entry.ly + OFFS[i][1];
			const int z = entry.lz + OFFS[i][2];

			int lx = 0, ly = 0, lz = 0;
			Chunk* chunk = entry.chunk->findNeighbourChunk(x, y, z, lx, ly, lz);
			if (chunk != nullptr) {
				{
					std::lock_guard<std::shared_mutex> m(chunk->dataMutex);
					const int light = chunk->getLight(lx, ly, lz, channel);
					block v = chunk->getBlock(lx, ly, lz);
					if (v.id == 0 && light + 2 <= entry.light) {;
						chunk->setLight(lx, ly, lz, channel, entry.light - 1);
						// chunk->modify();
						LightEntry nentry;
						nentry.lx = lx;
						nentry.ly = ly;
						nentry.lz = lz;
						nentry.light = entry.light - 1;
						nentry.chunk = chunk;
						addqueue.write(nentry);
					}
				}
			}
		}
	}
}
