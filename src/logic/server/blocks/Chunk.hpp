#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <ptypes.hpp>

#include "Block.hpp"

// LOGIC
#include "../lighting/LightMap.hpp"
#include "ChunkInfo.hpp"
#include "chunk_utils.hpp"

class Chunks;

enum class Neighbour {
    POS_X,
    NEG_X,
    POS_Y,
    NEG_Y,
    POS_Z, 
    NEG_Z,
	None
};

const uint32_t XZD_NEIGHBOURS_PRESENT  = (1 << 21) | (1 << 4) | (1 << 13) | (1 << 12) | (1 << 3) | (1 << 5) | (1 << 20) | (1 << 22);
const uint32_t XYZD_NEIGHBOURS_PRESENT = UINT_MAX;

const uint32_t INVERTED_IND[] = {1, 0, 3, 2, 5, 4};

class Section {
	std::unique_ptr<Lightmap> lightmap;
	std::unique_ptr<block[]>  blocks;
	
	std::atomic<uint32_t> version{ 0 };

	friend class Chunk;
public:
	Section() : 
		blocks(std::make_unique<block[]>(ChunkInfo::SECTION_VOLUME)),
		lightmap(new Lightmap) {}
};

class Chunk : public std::enable_shared_from_this<Chunk> {
protected:
	std::array<Section, ChunkInfo::HEIGHT / ChunkInfo::SECTION_HEIGHT> sections;

	std::weak_ptr<Chunk> neighbours_safe[6];
	std::mutex neighbours_mtx;

	friend class World;
	friend class LightSolver;
public:
	std::unique_ptr<Lightmap> lightmap;
	std::unique_ptr<block[]>  blocks;
	Vector3I pos;

	Chunk() : pos({0, 0, 0})
	{

	}

	Chunk(Vector3I pos) : pos(pos)
	{

	}

	inline Section& getSection(int y) 
	{
		return sections[y / ChunkInfo::SECTION_HEIGHT];
	}

	// Simple mechanism for locking neighbours of the chunk in order to prevent
	// them from deletion
	inline void lockNeighbours(std::shared_ptr<Chunk>(&lock)[7]) {
		for (int i = 0; i < 6; i++) {
			auto neighbour = neighbours_safe[i].lock();
			lock[i] = neighbour ? neighbour : nullptr;
		}
		lock[6] = shared_from_this();
	}

	// Call before destruction of a chunk in World.hpp.
	inline void unlinkAll() {
		for (int i = 0; i < 6; i++) removeNeighbour(i);
	}

	// Safely creates a link between two chunks 
	inline void loadNeighbour(int ind, const std::shared_ptr<Chunk>& neigh) {
		//std::scoped_lock lock(neighbours_mtx, neigh->neighbours_mtx);
		neighbours_safe[ind] = neigh;
		neigh->neighbours_safe[INVERTED_IND[ind]] = weak_from_this();
	}    

	// Safely unlinks neighbour from chunk 
	inline void removeNeighbour(int ind) {
		std::shared_ptr<Chunk> neigh;
		//{
		//	std::lock_guard lock(neighbours_mtx);
			neigh = neighbours_safe[ind].lock();
			if (!neigh) return;
		//}
		//std::scoped_lock lock(neighbours_mtx, neigh->neighbours_mtx);

		if (neighbours_safe[ind].lock() != neigh)
			return;

		neighbours_safe[ind].reset();
		neigh->neighbours_safe[INVERTED_IND[ind]].reset();
	}

	inline std::shared_ptr<Chunk> getNeighbour(int ind) {
		//std::lock_guard lock(neighbours_mtx);
		return neighbours_safe[ind].lock();
	}

	/*
	 * Finds neighbour chunk of current chunk.
	 * @param bx local
	 * @param by local
	 * @param bz local
	 */
	inline std::shared_ptr<Chunk> findNeighbourChunk(int bx, int by, int bz) {
		if (bx >= 0 && bx < ChunkInfo::WIDTH &&
			by >= 0 && by < ChunkInfo::HEIGHT &&
			bz >= 0 && bz < ChunkInfo::DEPTH) {
			return shared_from_this();
		}
		Neighbour idx = INDEX(bx, by, bz);
    	return getNeighbour(static_cast<size_t>(idx));
	}

   	inline block getBlock(int32_t lx, int32_t ly, int32_t lz) {
		return getSection(ly).blocks[((ly % ChunkInfo::SECTION_HEIGHT) * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx];
	}

	inline void  setBlock(int32_t lx, int32_t ly, int32_t lz, uint8_t id) {
		getSection(ly).blocks[((ly % ChunkInfo::SECTION_HEIGHT) * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx].id = id;
	}

	inline uint8_t getLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel) {
		return getSection(ly).lightmap->get(lx, (ly % ChunkInfo::SECTION_HEIGHT), lz, channel);
	}

	inline void setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission) {
		getSection(ly).lightmap->set(lx, (ly % ChunkInfo::SECTION_HEIGHT), lz, channel, emission);
	}

	inline block getBoundBlock(int32_t x, int32_t y, int32_t z) {
		if (INSIDE(x, y, z)) {
			return getBlock(x, y, z);
		}
		std::weak_ptr<Chunk> chunk = findNeighbourChunk(x, y, z);
		if (auto raw = chunk.lock()) {
			int32_t nx, ny, nz;
			LOCAL(nx, ny, nz, x, y, z);
			return raw->getBlock(nx, ny, nz);
		}
		else return block{};
	}

	uint8_t getBoundLight(int x, int y, int z, int channel) {
		if (INSIDE(x, y, z)) {
			return getLight(x, y, z, channel);
		}
		std::weak_ptr<Chunk> chunk = findNeighbourChunk(x, y, z);
		if (auto raw = chunk.lock()) {
			int32_t nx, ny, nz;
			LOCAL(nx, ny, nz, x, y, z);
			return raw->getLight(nx, ny, nz, channel);
		}
		else return 0;
	}

	/*
	 * Transforms global coordinates into local coords.
	 */
	static inline void LOCAL(int& lx, int& ly, int& lz, int x, int y, int z) {
		lx = x - floorDiv(x, ChunkInfo::WIDTH) * ChunkInfo::WIDTH;
		ly = y - floorDiv(y, ChunkInfo::HEIGHT) * ChunkInfo::HEIGHT;
		lz = z - floorDiv(z, ChunkInfo::DEPTH) * ChunkInfo::DEPTH;
	}

	/*
	 * Transforms local coordinates into global coords.
	 */
	static inline void GLOBAL(int& gx, int& gy, int& gz, int x, int y, int z, Chunk* chunk) {
		gx = x + chunk->pos.x * ChunkInfo::WIDTH;
		gy = y + chunk->pos.y * ChunkInfo::HEIGHT;
		gz = z + chunk->pos.z * ChunkInfo::DEPTH;
	}

	static inline bool INSIDE(int x, int y, int z) {
		return (x >= 0 && x < ChunkInfo::WIDTH &&
			y >= 0 && y < ChunkInfo::HEIGHT &&
			z >= 0 && z < ChunkInfo::DEPTH);
	}

	static inline Neighbour INDEX(int x, int y, int z) {
		if (x >= ChunkInfo::WIDTH)  return Neighbour::POS_X;
		if (x < 0)                  return Neighbour::NEG_X;

		if (y >= ChunkInfo::HEIGHT) return Neighbour::POS_Y;
		if (y < 0)                  return Neighbour::NEG_Y;

		if (z >= ChunkInfo::DEPTH)  return Neighbour::POS_Z;
		if (z < 0)                  return Neighbour::NEG_Z;

		return Neighbour::None;
	}
};

using ChunkPtr  = std::shared_ptr<Chunk>;
using ChunkRef  = const std::shared_ptr<Chunk>&;
using ChunkWeak = std::weak_ptr<Chunk>;
using ChunkUPtr = std::unique_ptr<Chunk>;
using NeighboursLock = std::shared_ptr<Chunk>[7];

#endif