#include "LightSolver.hpp"
#include "../blocks/Chunk.hpp"
#include "../blocks/Block.hpp" 
#include "../LogicSystem.hpp"
#include "LightInfo.hpp"

#include <memory>
#include <mutex>
#include <shared_mutex>

const int OFFS[6][3] = {
	{0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0}, {1,0,0}, {-1,0,0}
};

LightSolver::LightSolver(LogicSystem* chunks, int channel) : chunks(chunks), channel(channel) {
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
	chunk->setLight(x, y, z, channel, entry.light);
}

void LightSolver::addLocally(int x, int y, int z, Chunk* chunk) {
	addLocally(x, y, z, chunk->getLight(x, y, z, channel), chunk);
}

void LightSolver::remove(int x, int y, int z) {}

void LightSolver::removeLocally(int lx, int ly, int lz, Chunk* chunk) {
	if (!chunk) return;

	int light = chunk->getLight(lx, ly, lz, channel);
	
	LightEntry entry{lx, ly, lz, static_cast<uint8_t>(light), chunk};
	remqueue.write(entry);
	
	chunk->setLight(lx, ly, lz, channel, 0);
}

void LightSolver::solve() {
	while (!remqueue.empty()) {
		LightEntry entry = remqueue.read();

		Chunk* entry_chunk = entry.chunk;
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
					LightEntry nentry{lx, ly, lz, light, chunk};

					
					remqueue.write(nentry);
					chunk->dirtyHot();
					chunk->setLight(lx, ly, lz, channel, 0);
				}
				else if (light >= entry.light) {
					LightEntry nentry{lx, ly, lz, light, chunk};

					chunk->dirtyHot();
					addqueue.write(nentry);
				}
			}
		}
	}

	while (!addqueue.empty()) {
		LightEntry entry = addqueue.read();

		Chunk *entry_chunk = entry.chunk;
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
				LightEntry nentry{lx, ly, lz, nl, chunk};

				chunk->dirtyHot();
				addqueue.write(nentry);
			}
		}
	}
}

BasicLightSolver::BasicLightSolver(LogicSystem* chunks) : 
	solverR(new LightSolver(chunks, 0)), 
	solverB(new LightSolver(chunks, 1)), 
	solverG(new LightSolver(chunks, 2)), 
	solverS(new LightSolver(chunks, 3)) {}


void BasicLightSolver::processBoundaryBlock(
    Chunk *A, Chunk* B,
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
    Chunk* chunk, Chunk* neighbor,
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

void BasicLightSolver::propagateSunLight(Chunk* chunk) {
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

void BasicLightSolver::propagateSunRay(int lx, int lz, Chunk* chunk) {
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

void BasicLightSolver::calculateLight(Chunk *chunk) {
	std::array<bool,4> addedAnyGlobal = {false,false,false,false};
    for (int face = 0; face < 6; ++face) {
		Chunk* neigbour = chunk->getNeigbourByFace(face);
		if(!neigbour) continue;

		neigbour->decompress();

		syncBoundaryWithNeigbour(chunk, neigbour, face, addedAnyGlobal);
		neigbour->dirtyHot();
    }

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();
}

void BasicLightSolver::removeLightLocally(int lx, int ly, int lz, Chunk *chunk) {
	solverR->removeLocally(lx, ly, lz, chunk);
	solverG->removeLocally(lx, ly, lz, chunk);
	solverB->removeLocally(lx, ly, lz, chunk);
	solverS->removeLocally(lx, ly, lz, chunk);

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();
	propagateSunRay(lx, lz, chunk);

    std::array<bool,4> addedAny = {false,false,false,false};
    for (int face = 0; face < 6; ++face) {
        if (!isOnFace(lx, ly, lz, face)) continue;

        Chunk* neighbour = chunk->getNeigbourByFace(face);
        if (!neighbour) continue;

        syncBoundaryWithNeigbour(chunk, neighbour, face, addedAny);
		neighbour->dirty();
    }

    if(addedAny[0]) solverR->solve();
    if(addedAny[1]) solverG->solve();
    if(addedAny[2]) solverB->solve();
    if(addedAny[3]) solverS->solve();
	
	chunk->dirty();
}

void BasicLightSolver::placeLightLocally(int lx, int ly, int lz, Emission emission, Chunk *chunk) {
	if(emission.r) {
		solverR->addLocally(lx, ly, lz, emission.r, chunk);
		solverR->solve();
	} 
	if(emission.g) {
		solverG->addLocally(lx, ly, lz, emission.g, chunk);
		solverG->solve();
	}
	if(emission.b) {
		solverB->addLocally(lx, ly, lz, emission.b, chunk);
		solverB->solve();
	}
	if(emission.s) {
		propagateSunRay(lx, lz, chunk);

		solverS->addLocally(lx, ly, lz, emission.s, chunk);
		solverS->solve();
	}
}