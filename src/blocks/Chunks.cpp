#include "Chunks.hpp"
#include "Chunk.hpp"
#include "Block.hpp"
#include "ChunkInfo.hpp"
#include "iostream"

#include "../lighting/LightMap.hpp"
#include "structures/ThreadPool.hpp"

#include <glm/glm.hpp>

using namespace glm;

#include <math.h>
#include <limits.h>
#include "ChunkRLE.hpp"

glm::ivec3 worldToChunk3(glm::dvec3 pos) {
    return { floorDiv(pos.x, ChunkInfo::WIDTH),
             floorDiv(pos.y, ChunkInfo::HEIGHT),
             floorDiv(pos.z, ChunkInfo::DEPTH) };
}

Chunks::Chunks(int w, int h, int d, bool lighting) : noise(0), threadPool(4)  {
	PerlinNoise noise(0);
	noise.octaves = 2;

	if (lighting) {
		solverR = new LightSolver(this, 0);
		solverG = new LightSolver(this, 1);
		solverB = new LightSolver(this, 2);
		solverS = new LightSolver(this, 3);
	}

	int index = 0;
	for (int y = 0; y < h; y++) {
		for (int z = 0; z < d; z++) {
			for (int x = 0; x < w; x++, index++) {
				auto [it, inserted] = chunkMap.emplace(
					ChunkPos{x,y,z}, 
					std::make_unique<Chunk>(x,y,z,noise)
				);
				Chunk* chunkPtr = it->second.get();
				iterable.push_back(chunkPtr);
			}
		}
	}

	for (auto& [pos, uptr] : chunkMap) {
    	loadNeighbours(uptr.get());
	}

	if (lighting) {
		for (Chunk* chunk : iterable) {
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
			}
		}

		for (Chunk* chunk : iterable) {

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
		}

		for (Chunk* chunk : iterable) {
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

	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();
}

void Chunks::loadNeighbours(Chunk* chunk) {
    for (int i = 0; i < 26; ++i) {
        int nx = chunk->x + OFFSETS[i][0];
        int ny = chunk->y + OFFSETS[i][1];
        int nz = chunk->z + OFFSETS[i][2];

        auto it = chunkMap.find(ChunkPos{nx, ny, nz});
        if (it != chunkMap.end()) {
            Chunk* neighbour = it->second.get();
            chunk->neighbors[i] = neighbour;

            // найдем индекс противоположного смещения (или заранее подготовь opposite[i])
            int opp = oppositeIdx[i];
            if (opp >= 0) neighbour->neighbors[opp] = chunk;
        } else {
            chunk->neighbors[i] = nullptr;
        }
    }
}

void Chunks::processBoundaryBlock(
    Chunk* A, Chunk* B,
    int ax, int ay, int az,
    int bx, int by, int bz,
    std::array<bool, 4> &addedAny)
{
    block aBlock = A->getBlock(ax, ay, az);
    block bBlock = B->getBlock(bx, by, bz);

    // Если один из блоков непрозрачный для света (solid) — не передаём свет между ними
    if (aBlock.id != 0 && aBlock.id != 0) return;

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
                solverS->addLocally(ax, ay, bz, B);
                addedAny[3] = true;
            }
        }
    }
}

void Chunks::syncBoundaryWithNeigbour(
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

void Chunks::calculateLight(Chunk* chunk) {
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

	{ // сначала проверим, нет ли уже чанка
        std::shared_lock<std::shared_mutex> sl(chunkMapMutex);
        if (chunkMap.find(pos) != chunkMap.end()) return; // уже загружен
    }

    // Запланировать задачу генерации в пул потоков
    auto fut = threadPool.submit([this, pos]() {
        // generateChunk делает только локальную работу — не трогает chunkMap
        auto chunk_ptr = std::make_unique<Chunk>(pos.x, pos.y, pos.z, noise);

        // Поместим готовый чанк в очередь для интеграции в главный поток
        {
            std::lock_guard<std::mutex> lk(this->readyQueueMutex);
            this->readyChunks.push(std::move(chunk_ptr));
        }
        this->readyCv.notify_one();
    });

    // (опционально) сохранить future, чтобы отслеживать или дождаться позже
    generationFutures.emplace_back(std::move(fut));
}

void Chunks::unloadChunk(int x, int y, int z) {
	ChunkPos key{x,y,z};
    std::unique_lock<std::shared_mutex> mapLock(chunkMapMutex);
    auto it = chunkMap.find(key);
    if (it == chunkMap.end()) return;
    // Если нужен безопасный отцеп от iterable — удалить там тоже
    Chunk* ptr = it->second.get();
    for (int i = 0; i < 26; ++i) {
        Chunk* n = ptr->neighbors[i];
        if (!n) continue;
        int opp = 25 - i;
        if (opp >= 0 && n->neighbors[opp] == ptr) {
            n->neighbors[opp] = nullptr;
        }
    }
    chunkMap.erase(it);
}


block Chunks::getBlock(int x, int y, int z) {
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

unsigned char Chunks::getLight(int x, int y, int z, int channel) {
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

Chunk* Chunks::getChunkByBlock(int x, int y, int z) {
    int cx = floorDiv(x, ChunkInfo::WIDTH);
    int cy = floorDiv(y, ChunkInfo::HEIGHT);
    int cz = floorDiv(z, ChunkInfo::DEPTH);
    auto it = chunkMap.find(ChunkPos{cx, cy, cz});
    if (it == chunkMap.end()) return nullptr;
    return it->second.get();
}

Chunk* Chunks::getChunk(int x, int y, int z) {
    ChunkPos key{x, y, z};
    std::shared_lock<std::shared_mutex> sl(chunkMapMutex);
    auto it = chunkMap.find(key);
    if (it == chunkMap.end()) return nullptr;
    return it->second.get();
}

void Chunks::update(const glm::dvec3 &playerPos) {
	{
        std::unique_lock<std::mutex> lk(readyQueueMutex);
        while (!readyChunks.empty()) {
            auto c = std::move(readyChunks.front());
            readyChunks.pop();
            lk.unlock(); // разблокируем очередь пока вставляем в map

            // Вставляем в map под эксклюзивным shared_mutex
            {
                std::unique_lock<std::shared_mutex> mapLock(chunkMapMutex);
                ChunkPos key{c->x, c->y, c->z};
                // двойная проверка: возможно уже вставлен другим джобом
                if (chunkMap.find(key) == chunkMap.end()) {
                    chunkMap.emplace(key, std::move(c));
                    // теперь можно обновить соседей, пересчитать свет, построить меш и т.д.
                    Chunk* inserted = chunkMap.at(key).get();
                    loadNeighbours(inserted);
                    calculateLight(inserted);

                    std::array<bool,4> addedAnyGlobal = { false, false, false, false };
					
					for (int face = 0; face < 6; ++face) {
						int idx = faceToIdx(face); // face -> индекс в OFFSETS/neighbors
						if (idx < 0) continue;
						Chunk* neigh = inserted->neighbors[idx];
						if (!neigh) continue;
						syncBoundaryWithNeigbour(inserted, neigh, face, addedAnyGlobal);
						neigh->modify();
					}
					
					solverS->solve();
                }
            }

            lk.lock(); // снова блокируем очередь, чтобы продолжить
        }
    }

	ivec3 playerChunk = worldToChunk3(playerPos);

	if (playerChunk != lastPlayerChunk) {
		glm::ivec3 delta = playerChunk - lastPlayerChunk;

		if (delta.x != 0) {
			int oldX = lastPlayerChunk.x - delta.x * loadDistance;
			int newX = playerChunk.x + delta.x * loadDistance;

			for (int z = playerChunk.z - loadDistance; z <= playerChunk.z + loadDistance; z++) {
				unloadChunk(oldX, 0, z);
				loadChunk(newX, 0, z);
			}
		}

		if (delta.z != 0) {
			int oldZ = lastPlayerChunk.z - delta.z * loadDistance;
			int newZ = playerChunk.z + delta.z * loadDistance;

			for (int x = playerChunk.x - loadDistance; x <= playerChunk.x + loadDistance; x++) {

				unloadChunk(x, 0, oldZ);
				loadChunk(x, 0, newZ);
			}
		}

		lastPlayerChunk = playerChunk;
	}
}

