#include "LightSolver.hpp"
#include "../blocks/Chunk.hpp"
#include "../blocks/Block.hpp" 
#include "../World.hpp"
#include "LightInfo.hpp"

#include <memory>
#include <mutex>
#include <shared_mutex>

const int OFFS[6][3] = {
	{0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0}, {1,0,0}, {-1,0,0}
};

void BasicLightSolver::addLocally(int x, int y, int z, uint8_t emission, int32_t channel, Chunk* chunk) {
	if (emission <= 1)
		return;

	LightLocal entry;
	entry.lx = x;
	entry.ly = y;
	entry.lz = z;
	entry.light = emission;
	entry.channel = channel;

	chunk->lightmap->set(x, y, z, channel, emission);
	chunk->lightmap->toAdd.push_back(entry);	
}

void BasicLightSolver::solve(ChunkPtr schunk) {
	Chunk* chunk = schunk.get();

	std::deque<LightLocal>& addqueue = chunk->lightmap->toAdd;
	std::deque<LightLocal>& remqueue = chunk->lightmap->toDel;

	if (addqueue.empty() && remqueue.empty()) return;

	//while (!remqueue.empty()) {
	//	LightEntry entry = remqueue.front(); remqueue.pop();
	//
	//	Chunk* entry_chunk = entry.chunk;
	//	if (!entry_chunk) continue;
	//
	//	for (size_t i = 0; i < 6; i++) {
	//		int x = entry.lx + OFFS[i][0];
	//		int y = entry.ly + OFFS[i][1];
	//		int z = entry.lz + OFFS[i][2];
	//
	//		Chunk* chunk = entry_chunk->findNeighbourChunk(x, y, z);
	//
	//		if (chunk) {
	//			int lx, ly, lz;
	//			Chunk::local(lx, ly, lz, x, y, z);
	//
	//			uint8_t light = chunk->getLight(lx, ly, lz, channel);
	//			if (light != 0 && light == entry.light - 1) {
	//				LightEntry nentry{ lx, ly, lz, light, chunk };
	//
	//
	//				remqueue.push(nentry);
	//				chunk->setLight(lx, ly, lz, channel, 0);
	//			}
	//			else if (light >= entry.light) {
	//				LightEntry nentry{ lx, ly, lz, light, chunk };
	//
	//				addqueue.push(nentry);
	//			}
	//		}
	//	}
	//}

	//std::shared_lock<std::shared_mutex> read(chunk->lightmap->mutex);
	//{
		while (!addqueue.empty()) {
			LightLocal entry = addqueue.front(); addqueue.pop_front();
			if (entry.light <= 1) continue;

			for (size_t i = 0; i < 6; i++) {
				const int x = entry.lx + OFFS[i][0];
				const int y = entry.ly + OFFS[i][1];
				const int z = entry.lz + OFFS[i][2];

				if (Chunk::INSIDE(x, y, z)) {
					block v = chunk->getBlock(x, y, z);
					unsigned char curLight = chunk->getLight(x, y, z, entry.channel);

					if (v.id == 0 && static_cast<int>(curLight) + 1 < entry.light) {
						uint8_t nl = entry.light - 1;
						LightLocal nentry{ x, y, z, nl, entry.channel };

						chunk->setLight(x, y, z, entry.channel, nl);
						addqueue.push_back(nentry);
					}
				}
			}
		}
	//}
}

void BasicLightSolver::solveRecursive(ChunkPtr chunk) {
	if(chunk->lightmap->toAdd.empty() && chunk->lightmap->toDel.empty()) return;

	solve(chunk);
	for (int i = 0; i < 6; i++) {
		if (auto n = chunk->getNeighbour(i).lock()) calculateLight(n);
	}
}

void BasicLightSolver::processBoundaryBlock(
	Chunk* A, Chunk* B,
	int ax, int ay, int az,
	int bx, int by, int bz)
{
	block aBlock = A->getBlock(ax, ay, az);
	block bBlock = B->getBlock(bx, by, bz);

	if (aBlock.id != 0 || bBlock.id != 0) return;

	for (int chan = 0; chan <= 3; ++chan) {
		uint8_t L_a = A->getLight(ax, ay, az, chan);
		uint8_t L_b = B->getLight(bx, by, bz, chan);
		if      (L_b > L_a + 1) addLocally(ax, ay, az, L_b-1, chan, A);
		else if (L_a > L_b + 1) addLocally(bx, by, bz, L_a-1, chan, B);
	}
}

void BasicLightSolver::syncBoundaryWithNeigbour(Chunk* chunk, Chunk* neighbor, int dir)
{
	const int W = ChunkInfo::WIDTH;
	const int H = ChunkInfo::HEIGHT;
	const int D = ChunkInfo::DEPTH;

	int dx = FACE_DIRS[dir][0];
	int dy = FACE_DIRS[dir][1];
	int dz = FACE_DIRS[dir][2];

	if (dx != 0) {
		int ax = (dx > 0) ? (W - 1) : 0;
		int bx = (dx > 0) ? 0 : (W - 1);
		for (int y = 0; y < H; ++y) for (int z = 0; z < D; ++z) {
			processBoundaryBlock(chunk, neighbor, ax, y, z, bx, y, z);
		}
	}
	else if (dy != 0) {
		int ay = (dy > 0) ? (H - 1) : 0;
		int by = (dy > 0) ? 0 : (H - 1);
		for (int x = 0; x < W; ++x) for (int z = 0; z < D; ++z) {
			processBoundaryBlock(chunk, neighbor, x, ay, z, x, by, z);
		}
			
	}
	else {
		int az = (dz > 0) ? (D - 1) : 0;
		int bz = (dz > 0) ? 0 : (D - 1);
		for (int x = 0; x < W; ++x) for (int y = 0; y < H; ++y) {
			processBoundaryBlock(chunk, neighbor, x, y, az, x, y, bz);
		}
	}
}

void BasicLightSolver::propagateSunLight(ChunkPtr chunk) {
	Chunk* raw = chunk.get();

	/*
	for (int y = 0; y < ChunkInfo::HEIGHT; y++) {
		for (int z = 0; z < ChunkInfo::DEPTH; z++) {
			for (int x = 0; x < ChunkInfo::WIDTH; x++) {
				block vox = chunk->getBlock(x, y, z);
				if (vox.id == 1) {
					solverR->addLocally(x, y, z, 0, chunk);
					solverG->addLocally(x, y, z, 0, chunk);
					solverB->addLocally(x, y, z, 0, chunk);
				}
			}
		}
	}*/
	for (int z = 0; z < ChunkInfo::DEPTH; z++) {
		for (int x = 0; x < ChunkInfo::WIDTH; x++) {
			for (int y = ChunkInfo::HEIGHT - 1; y >= 0; y--) {
				block vox = chunk->getBlock(x, y, z);
				if (vox.id != 0) {
					break;
				}
				if (
					chunk->getBoundLight(x - 1, y, z, 3) == 0 ||
					chunk->getBoundLight(x + 1, y, z, 3) == 0 ||
					chunk->getBoundLight(x, y - 1, z, 3) == 0 ||
					chunk->getBoundLight(x, y + 1, z, 3) == 0 ||
					chunk->getBoundLight(x, y, z - 1, 3) == 0 ||
					chunk->getBoundLight(x, y, z + 1, 3) == 0
					) addLocally(x, y, z, 0xF, 3, raw);
				chunk->setLight(x, y, z, 3, 0xF);
			}
		}
	}
}

void BasicLightSolver::propagateSunRay(int lx, int lz, ChunkPtr &chunk) {
	for (int y = ChunkInfo::HEIGHT - 1; y >= 0; y--) {
		block vox = chunk->getBlock(lx, y, lz);
		if (vox.id != 0) {
			break;
		}
		if (
			chunk->getBoundLight(lx - 1, y, lz, 3) == 0 ||
			chunk->getBoundLight(lx + 1, y, lz, 3) == 0 ||
			chunk->getBoundLight(lx, y - 1, lz, 3) == 0 ||
			chunk->getBoundLight(lx, y + 1, lz, 3) == 0 ||
			chunk->getBoundLight(lx, y, lz - 1, 3) == 0 ||
			chunk->getBoundLight(lx, y, lz + 1, 3) == 0
			) addLocally(lx, y, lz, 0xF, 3, chunk.get());
	}
}

void BasicLightSolver::calculateLight(ChunkPtr chunk) {
	for (int face = 0; face < 6; ++face) {
		if (auto n = chunk->getNeighbour(face).lock()) {
			syncBoundaryWithNeigbour(chunk.get(), n.get(), face);
		}
	}
	solveRecursive(chunk);
}