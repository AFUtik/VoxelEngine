#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <memory>
#include <shared_mutex>

#include "Block.hpp"

#include "../lighting/LightMap.hpp"
#include "../noise/PerlinNoise.hpp"
#include "../graphics/renderer/Drawable.hpp"

#include "chunk_utils.hpp"
#include "RenderInfo.hpp"

#include <iostream>

class Chunks;

class Chunk {
	//ChunkRenderInfo renderInfo;

	/* Chunk has 4 horizontal neighbours, 2 vertical and 20 corner neigbours for correct lighting on chunk borders. */
	std::shared_ptr<Chunk> neighbors[26];
	std::weak_ptr<Chunk> weak_self;
	
	std::unique_ptr<block[]> blocks;
	std::unique_ptr<Lightmap> lightmap;

	friend class Chunks;
	friend class ChunkRLE;
	friend class ChunkMesher;
	friend class LightSolver;

	std::atomic<bool> modified{false};

	Chunks* world;
public:
	mutable std::shared_mutex dataMutex;
	DrawableObject chunk_draw;

	inline void clearNeighbours() {
		for(int i = 0; i < 26; i++) neighbors[i].reset();
	}

	inline void modify() { modified.store(true, std::memory_order_relaxed); }
	inline void unmodify() { modified.store(false, std::memory_order_relaxed); }
	inline bool isModified() const { return modified.load(std::memory_order_relaxed); }
	inline bool test_and_clear_modified() { return modified.exchange(false, std::memory_order_acq_rel); }

	// World Pos //
	int32_t x, y, z;
	glm::vec3 min;
	glm::vec3 max;

	Chunk(int x, int y, int z) : x(x), y(y), z(z) {}
	
	/*
	 * Transforms global coordinates into local coords.
	 */
	static inline void local(int& lx, int& ly, int& lz, int x, int y, int z) {
		lx = x - floorDiv(x, ChunkInfo::WIDTH)  * ChunkInfo::WIDTH;
		ly = y - floorDiv(y, ChunkInfo::HEIGHT) * ChunkInfo::HEIGHT;
		lz = z - floorDiv(z, ChunkInfo::DEPTH)  * ChunkInfo::DEPTH;
	}

	/*
	 * Transforms local coordinates into global coords.
	 */
	static inline void global(int& gx, int& gy, int& gz, int x, int y, int z, Chunk* chunk) {
		gx = x + chunk->x * ChunkInfo::WIDTH;
		gy = y + chunk->y * ChunkInfo::HEIGHT;
		gz = z + chunk->z * ChunkInfo::DEPTH;
	}

	inline int getNeighbourIndex(int lx, int ly, int lz) const {
		int dx = (lx < 0) ? -1 : (lx >= ChunkInfo::WIDTH ? 1 : 0);
		int dy = (ly < 0) ? -1 : (ly >= ChunkInfo::HEIGHT ? 1 : 0);
		int dz = (lz < 0) ? -1 : (lz >= ChunkInfo::DEPTH ? 1 : 0);

		if (dx == 0 && dy == 0 && dz == 0) return -1;
		return NEI_INDEX_BY_ENCODE[ encode3(dx, dy, dz) ];
	}

	inline Chunk* getNeigbourByFace(int face) {
		int idx = faceToIdx(face);
        if (idx < 0) return nullptr;

		return neighbors[face].get();
	}

	inline const std::shared_ptr<Chunk>& getSharedNeigbourByFace(int face) {
		return neighbors[faceToIdx(face)];
	}

	/*
	 * Finds neighbour chunk of current chunk.
	 * @param bx local
	 * @param by local
	 * @param bz local
	 */
	inline Chunk* findNeighbourChunk(int bx, int by, int bz) {
		if (bx >= 0 && bx < ChunkInfo::WIDTH &&
			by >= 0 && by < ChunkInfo::HEIGHT &&
			bz >= 0 && bz < ChunkInfo::DEPTH) {
			return this;
		}
		int idx = getNeighbourIndex(bx, by, bz);
    	return neighbors[idx].get();
	}

	inline block getBoundBlock(int32_t lx, int32_t ly, int32_t lz) {
		if (lx >= 0 && lx < ChunkInfo::WIDTH &&
			ly >= 0 && ly < ChunkInfo::HEIGHT &&
			lz >= 0 && lz < ChunkInfo::DEPTH) {
			return getBlock(lx, ly, lz);
		}
		int nx, ny, nz;
		Chunk *chunk = findNeighbourChunk(lx, ly, lz);
		local(nx, ny, nz, lx, ly, lz);

		if (!chunk) return block{};
		if (nx < 0 || nx >= ChunkInfo::WIDTH ||
			ny < 0 || ny >= ChunkInfo::HEIGHT ||
			nz < 0 || nz >= ChunkInfo::DEPTH) {
			return block{};
		}
		return chunk->getBlock(nx, ny, nz);
	}

   	inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const {return blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx];}

	inline void setBlock(int32_t lx, int32_t ly, int32_t lz, uint8_t id) const {blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx].id = id;}

	uint8_t getBoundLight(int lx, int ly, int lz, int channel);

	uint8_t getLight(int32_t lx, int32_t, int32_t lz, int32_t channel) const;

	void Chunk::setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission);

	Chunk(int x, int y, int z, PerlinNoise& generator);
};



#endif // !CHUNK_HPP