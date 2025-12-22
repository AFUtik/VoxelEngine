#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <atomic>
#include <memory>
#include <shared_mutex>

#include "Block.hpp"

#include "../lighting/LightMap.hpp"
#include "../noise/PerlinNoise.hpp"
#include "../graphics/renderer/Drawable.hpp"
#include "../lighting/LightInfo.hpp"

#include "ChunkCompressor.hpp"
#include "ChunkInfo.hpp"
#include "chunk_utils.hpp"
#include "RenderInfo.hpp"

#include <iostream>

class Chunks;

struct ChunkPos {
    int32_t x, y, z;
    bool operator==(const ChunkPos& o) const noexcept {
        return x==o.x && y==o.y && z==o.z;
    }
};

struct ChunkPosHash {
    std::size_t operator()(const ChunkPos& p) const noexcept {
        // FNV-1a style mix (fast and decent)
        uint64_t h = 14695981039346656037ull;
        auto mix = [&](uint32_t v){
            h ^= v;
            h *= 1099511628211ull;
        };
        mix(static_cast<uint32_t>(p.x));
        mix(static_cast<uint32_t>(p.y));
        mix(static_cast<uint32_t>(p.z));
        return static_cast<std::size_t>(h ^ (h >> 32));
    }
};

struct ChunkPosLess {
    bool operator()(const ChunkPos& a, const ChunkPos& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }
};

enum class ChunkState : uint8_t {
	Empty,
	Generated,
	Changed,
	Lighted,
	Finished,
	Removed,

	NeedsUpdate
};

class Chunk {
	std::shared_ptr<Chunk> neighbors[26];
	Chunk* rawNeighbours[26] {nullptr};

	std::weak_ptr<Chunk> weak_self;

	friend class Chunks;
	friend class ChunkCompressor;
	friend class ChunkMesher;
	friend class LightSolver;
	
	std::atomic<ChunkState> state {ChunkState::Empty};
	std::atomic<uint32_t> version;
	
	ChunkPos hash_pos;
	glm::vec3 min;
	glm::vec3 max;
public:
	std::unique_ptr<Lightmap> lightmap;
	std::unique_ptr<block[]> blocks;

	int32_t x, y, z;

	mutable std::shared_mutex dataMutex;
	DrawableObject chunk_draw;

	inline ChunkPos position() {return hash_pos;}

	inline bool checkState(ChunkState expected) {return expected == state.load(std::memory_order_relaxed);}
	inline bool checkVersion(uint32_t expected) {return expected == version.load(std::memory_order_relaxed);}

	inline void setState(ChunkState new_state) {
		version.fetch_add(1, std::memory_order_release);
		state.store(new_state, std::memory_order_release);
	}

	inline void dirty() {
		version.fetch_add(1, std::memory_order_release);
		state.store(ChunkState::NeedsUpdate, std::memory_order_release);
	}

	inline void dirtyHot() {state.store(ChunkState::NeedsUpdate, std::memory_order_relaxed);}
	
	inline void loadNeighbour(int ind, const std::shared_ptr<Chunk> &neigh) {
		neighbors[ind]     = neigh;
		rawNeighbours[ind] = neigh.get();
	}

	inline void clearNeighbours() {
		for(int i = 0; i < 26; i++) {
			neighbors[i].reset();
			rawNeighbours[i] = nullptr;
		}
	}

	// World Pos //
	Chunk(int x, int y, int z) : x(x), y(y), z(z), hash_pos({x, y, z}), blocks(std::make_unique<block[]>(ChunkInfo::VOLUME)), lightmap(new Lightmap) {}

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

	inline const std::shared_ptr<Chunk>& getNeigbour(int ind) {
		return neighbors[ind];
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
		if (lx >= 0 && lx < ChunkInfo::WIDTH  &&
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
};

using ChunkPtr  = std::shared_ptr<Chunk>;
using ChunkRef  = const std::shared_ptr<Chunk>&;
using ChunkWeak = std::weak_ptr<Chunk>;
using ChunkUPtr = std::unique_ptr<Chunk>;

#endif