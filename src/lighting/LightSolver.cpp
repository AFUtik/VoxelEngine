#include "LightSolver.hpp"
#include "LightMap.hpp"
#include "../blocks/Chunks.hpp"
#include "../blocks/Chunk.hpp"
#include "../blocks/Block.hpp" 
#include "LightInfo.hpp"

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
	
	{
		std::unique_lock<std::shared_mutex> wlock(lightMutex);
		addqueue.write(entry);
	}

	// chunk->modified = true;
	chunk->setLight(x, y, z, channel, entry.light);
}

void LightSolver::addLocally(int x, int y, int z, const std::shared_ptr<Chunk>& chunk) {
	addLocally(x, y, z, chunk->getLight(x, y, z, channel), chunk);
}

void LightSolver::remove(int x, int y, int z) {}

void LightSolver::removeLocally(int lx, int ly, int lz, const std::shared_ptr<Chunk> &chunk) {
	if (!chunk) return;

	int light = chunk->getLight(lx, ly, lz, channel);
	
	LightEntry entry{lx, ly, lz, static_cast<uint8_t>(light), chunk};
	{
		std::unique_lock<std::shared_mutex> wlock(lightMutex);
		remqueue.write(entry);
	}
	chunk->setLight(lx, ly, lz, channel, 0);
}

void LightSolver::solve(bool (&neigbours_dirty)[26]) {
	std::unique_lock<std::shared_mutex> wlock(lightMutex);

	while (!remqueue.empty()) {
		LightEntry entry = remqueue.read();

		std::shared_ptr<Chunk> entry_chunk = entry.chunk.lock();
		if (!entry_chunk) continue;

		for (size_t i = 0; i < 6; i++) {
			int x = entry.lx + OFFS[i][0];
			int y = entry.ly + OFFS[i][1];
			int z = entry.lz + OFFS[i][2];

			Chunk* chunk = entry_chunk->findNeighbourChunk(x, y, z);

			if (chunk) {
				int lx, ly, lz;
				Chunk::local(lx, ly, lz, x, y, z);

				uint8_t light = chunk->getLight(lx, ly, lz, channel);
				if (light != 0 && light == entry.light - 1) {
					LightEntry nentry{lx, ly, lz, light, chunk->weak_self};

					if(!inBounds(x, y, z)) neigbours_dirty[coordsIntoNeighInd(x, y, z)]=true;

					remqueue.write(nentry);
					chunk->setLight(lx, ly, lz, channel, 0);
				}
				else if (light >= entry.light) {
					LightEntry nentry{lx, ly, lz, light, chunk->weak_self};

					if(!inBounds(x, y, z)) neigbours_dirty[coordsIntoNeighInd(x, y, z)]=true;

					addqueue.write(nentry);
				}
			}
		}
	}

	while (!addqueue.empty()) {
		LightEntry entry = addqueue.read();

		std::shared_ptr<Chunk> entry_chunk = entry.chunk.lock();
		if (!entry_chunk) continue;

		if (entry.light <= 1)
			continue;

		for (size_t i = 0; i < 6; i++) {
			const int x = entry.lx + OFFS[i][0];
			const int y = entry.ly + OFFS[i][1];
			const int z = entry.lz + OFFS[i][2];

			Chunk* chunk = entry_chunk->findNeighbourChunk(x, y, z);
			
			if(!chunk) continue;

			int lx, ly, lz;
			Chunk::local(lx, ly, lz, x, y, z);

			bool should_propagate = false;
			block v = chunk->getBlock(lx, ly, lz);
			unsigned char curLight = chunk->getLight(lx, ly, lz, channel);

			if (v.id == 0 && static_cast<int>(curLight) + 1 < entry.light) should_propagate = true;
			
			if(should_propagate) {
				chunk->setLight(lx, ly, lz, channel, entry.light - 1);
				uint8_t nl = entry.light-1;
				LightEntry nentry{lx, ly, lz, nl, chunk->weak_self};

				if(!inBounds(x, y, z)) {
					neigbours_dirty[coordsIntoNeighInd(x, y, z)]=true;
				} 
				else if(onBorders(x, y, z)) {
					int cx = (x == 0) ? -1 : (x == ChunkInfo::WIDTH  - 1)  ? 1 : 0;
					int cy = (y == 0) ? -1 : (y == ChunkInfo::HEIGHT - 1)  ? 1 : 0;
					int cz = (z == 0) ? -1 : (z == ChunkInfo::DEPTH  - 1)  ? 1 : 0;
					neigbours_dirty[neighbourIndexFromDelta(cx, cy, cz)] = true;
				}
				addqueue.write(nentry);
			}
		}
	}
}

BasicLightSolver::BasicLightSolver(Chunks* chunks) : 
	solverR(new LightSolver(chunks, 0)), 
	solverB(new LightSolver(chunks, 1)), 
	solverG(new LightSolver(chunks, 2)), 
	solverS(new LightSolver(chunks, 3)) {}

void BasicLightSolver::processBoundaryBlockSingle(
    const std::shared_ptr<Chunk>& A, const std::shared_ptr<Chunk>& B,
    int ax, int ay, int az,
    int face,
    std::array<bool,4> &addedAny)
{
    // вычислим координаты в B
    int bx = ax + FACE_DIRS[face][0];
    int by = ay + FACE_DIRS[face][1];
    int bz = az + FACE_DIRS[face][2];

    block aBlock = A->getBlock(ax, ay, az);
    block bBlock = B->getBlock(bx, by, bz);
    if (aBlock.id != 0 || bBlock.id != 0) return;

    for (int chan = 0; chan <= 3; ++chan) {
        unsigned char L_a = A->getBoundLight(ax, ay, az, chan);
        unsigned char L_b = B->getBoundLight(bx, by, bz, chan);

        if (L_b > L_a) {
            if (chan < 3) {
                getSolver(chan)->addLocally(ax, ay, az, static_cast<int>(L_b), A);
                addedAny[chan] = true;
            } else {
                solverS->addLocally(bx, by, bz, B);
                addedAny[3] = true;
            }
        } else if (L_a > L_b) {
            if (chan < 3) {
                getSolver(chan)->addLocally(bx, by, bz, static_cast<int>(L_a), B);
                addedAny[chan] = true;
            } else {
                solverS->addLocally(ax, ay, az, A);
                addedAny[3] = true;
            }
        }
    }
}

void BasicLightSolver::processBoundaryBlock(
    const std::shared_ptr<Chunk>& A, const std::shared_ptr<Chunk>& B,
    int ax, int ay, int az,
    int bx, int by, int bz,
    std::array<bool, 4> &addedAny)
{
    block aBlock = A->getBlock(ax, ay, az);
    block bBlock = B->getBlock(bx, by, bz);
  
	if(aBlock.id != 0 || bBlock.id != 0) return;

    for (int chan = 0; chan <= 3; ++chan) {
        unsigned char L_a = A->getBoundLight(ax, ay, az, chan);
        unsigned char L_b = B->getBoundLight(bx, by, bz, chan);

        if (L_b > L_a) {
            if (chan < 3) {
                getSolver(chan)->addLocally(ax, ay, az, A);
                addedAny[chan] = true;
            } else {
                solverS->addLocally(bx, by, bz, B);
                addedAny[3] = true;
            }
        } else if (L_a > L_b) {
            if (chan < 3) {
                getSolver(chan)->addLocally(bx, by, bz, B);
                addedAny[chan] = true;
            } else {
                solverS->addLocally(ax, ay, az, A);
                addedAny[3] = true;
            }
        }
    }
}

void BasicLightSolver::syncBoundaryWithNeigbour(
    const std::shared_ptr<Chunk>& chunk, const std::shared_ptr<Chunk>& neighbor,
    int dir, std::array<bool, 4> &addedAny)
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
        for (int y = 0; y < H; ++y) for (int z = 0; z < D; ++z)
            processBoundaryBlock(chunk, neighbor, ax, y, z, bx, y, z, addedAny);
    } else if (dy != 0) {
        int ay = (dy > 0) ? (H - 1) : 0;
        int by = (dy > 0) ? 0 : (H - 1);
        for (int x = 0; x < W; ++x) for (int z = 0; z < D; ++z)
            processBoundaryBlock(chunk, neighbor, x, ay, z, x, by, z, addedAny);
    } else {
        int az = (dz > 0) ? (D - 1) : 0;
        int bz = (dz > 0) ? 0 : (D - 1);
        for (int x = 0; x < W; ++x) for (int y = 0; y < H; ++y)
            processBoundaryBlock(chunk, neighbor, x, y, az, x, y, bz, addedAny);
    }
}

void BasicLightSolver::propagateSunLight(const std::shared_ptr<Chunk>& chunk) {
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
	{
		std::unique_lock<std::shared_mutex> wlock(chunk->dataMutex);
		for (int z = 0; z < ChunkInfo::DEPTH; z++) {
			for (int x = 0; x < ChunkInfo::WIDTH; x++) {
				for (int y = ChunkInfo::HEIGHT - 1; y >= 0; y--) {
					block vox = chunk->getBlock(x, y, z);
					if (vox.id != 0) {
						break;
					}
					chunk->setLight(x, y, z, 3, 0xF);
				}
			}
		}
	
		for (int z = 0; z < ChunkInfo::DEPTH; z++) {
			for (int x = 0; x < ChunkInfo::WIDTH; x++) {
				for (int y = ChunkInfo::HEIGHT - 1; y >= 0; y--) {
					block vox = chunk->getBlock(x, y, z);
					if (vox.id != 0) {
						break;
					}
					if (
						chunk->getBoundLight(x-1, y, z, 3) == 0 ||
						chunk->getBoundLight(x+1, y, z, 3) == 0 ||
						chunk->getBoundLight(x, y-1, z, 3) == 0 ||
						chunk->getBoundLight(x, y+1, z, 3) == 0 ||
						chunk->getBoundLight(x, y, z-1, 3) == 0 ||
						chunk->getBoundLight(x, y, z+1, 3) == 0
						) solverS->addLocally(x, y, z, chunk);   
					chunk->setLight(x, y, z, 3, 0xF);
				}
			}
		}
	}
}

void BasicLightSolver::propagateSunRay(int lx, int lz, const std::shared_ptr<Chunk>& chunk) {
	{
		std::unique_lock<std::shared_mutex> wlock(chunk->dataMutex);
		for (int y = ChunkInfo::HEIGHT - 1; y >= 0; y--) {
			block vox = chunk->getBlock(lx, y, lz);
			if (vox.id != 0) {
				break;
			}
			chunk->setLight(lx, y, lz, 3, 0xF);
		}

		for (int y = ChunkInfo::HEIGHT - 1; y >= 0; y--) {
			block vox = chunk->getBlock(lx, y, lz);
			if (vox.id != 0) {
				break;
			}
			if (
				chunk->getBoundLight(lx-1, y, lz, 3) == 0 ||
				chunk->getBoundLight(lx+1, y, lz, 3) == 0 ||
				chunk->getBoundLight(lx, y-1, lz, 3) == 0 ||
				chunk->getBoundLight(lx, y+1, lz, 3) == 0 ||
				chunk->getBoundLight(lx, y, lz-1, 3) == 0 ||
				chunk->getBoundLight(lx, y, lz+1, 3) == 0
				) solverS->addLocally(lx, y, lz, chunk);   
			chunk->setLight(lx, y, lz, 3, 0xF);
		}
			
		
	}
}

void BasicLightSolver::calculateLight(const std::shared_ptr<Chunk> &chunk) {
	std::array<bool,4> addedAnyGlobal = {false,false,false,false};
    for (int face = 0; face < 6; ++face) {
		std::shared_ptr<Chunk> neigbour = chunk->getSharedNeigbourByFace(face);
		if(!neigbour) continue;

		syncBoundaryWithNeigbour(chunk, neigbour, face, addedAnyGlobal);
		neigbour->makeDirty();
    }

	bool neighbours[26] {false};

	solverR->solve(neighbours);
	solverG->solve(neighbours);
	solverB->solve(neighbours);
	solverS->solve(neighbours);

	for(int i = 0; i < 26; i++)
		if(neighbours[i]) {
			auto &neigh = chunk->getNeigbour(i);
			if(neigh) neigh->makeDirty();
		}
}

void BasicLightSolver::removeLightLocally(int lx, int ly, int lz, const std::shared_ptr<Chunk> &chunk) {
	bool neighbours[26] {false};

	solverR->removeLocally(lx, ly, lz, chunk);
	solverG->removeLocally(lx, ly, lz, chunk);
	solverB->removeLocally(lx, ly, lz, chunk);
	solverS->removeLocally(lx, ly, lz, chunk);

	solverR->solve(neighbours);
	solverG->solve(neighbours);
	solverB->solve(neighbours);
	solverS->solve(neighbours);
	propagateSunRay(lx, lz, chunk);

    std::array<bool,4> addedAny = {false,false,false,false};
    for (int face = 0; face < 6; ++face) {
        if (!isOnFace(lx, ly, lz, face)) continue;

        auto& neighbour = chunk->getSharedNeigbourByFace(face);
        if (!neighbour) continue;

        syncBoundaryWithNeigbour(chunk, neighbour, face, addedAny);
		neighbour->makeDirty();
    }
	
    if(addedAny[0]) solverR->solve(neighbours);
    if(addedAny[1]) solverG->solve(neighbours);
    if(addedAny[2]) solverB->solve(neighbours);
    if(addedAny[3]) solverS->solve(neighbours);

	chunk->makeDirty();
	for(int i = 0; i < 26; i++)
		if(neighbours[i]) {
			auto &neigh = chunk->getNeigbour(i);
			if(neigh) neigh->makeDirty();
	}
}


void BasicLightSolver::placeLightLocally(int lx, int ly, int lz, Emission emission, const std::shared_ptr<Chunk> &chunk) {
	bool neighbours[26] {false};
	
	if(emission.r) {
		solverR->addLocally(lx, ly, lz, emission.r, chunk);
		solverR->solve(neighbours);
	} 
	if(emission.g) {
		solverG->addLocally(lx, ly, lz, emission.g, chunk);
		solverG->solve(neighbours);
	}
	if(emission.b) {
		solverB->addLocally(lx, ly, lz, emission.b, chunk);
		solverB->solve(neighbours);
	}
	if(emission.s) {
		propagateSunRay(lx, lz, chunk);

		solverS->addLocally(lx, ly, lz, emission.s, chunk);
		solverS->solve(neighbours);
	}

	chunk->makeDirty();
	for(int i = 0; i < 26; i++)
		if(neighbours[i]) {
			auto &neigh = chunk->getNeigbour(i);
			if(neigh) neigh->makeDirty();
		}
}