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

class Chunk {
protected:
	std::weak_ptr<Chunk> neighbours_safe[6];
	std::weak_ptr<Chunk> this_weak;
	std::atomic<uint32_t> neighbours_mask = 0;

	friend class World;
	friend class ChunkCompressor;
	friend class LightSolver;
public:
	std::unique_ptr<Lightmap> lightmap;
	std::unique_ptr<block[]>  blocks;

	std::atomic<uint32_t> version = 0;
	Vector3I pos;

	static std::shared_ptr<Chunk> make(Vector3I pos) {
		auto chunk = std::make_shared<Chunk>(pos);
		chunk->this_weak = chunk;
		return chunk;
	}

	// MECHANISM FOR LOCKING NEIGHBOURS IN MULTITHREAD ENVIROMENT //
	inline void lockNeighbours(std::shared_ptr<Chunk> (&lock)[7]) {
		for (int i = 0; i < 6; i++) lock[i] = neighbours_safe[i].lock();
		lock[6] = this_weak.lock();
	}

	Chunk(std::shared_ptr<Chunk> chunk) : pos(chunk->pos), blocks(std::move(chunk->blocks)), lightmap(std::move(chunk->lightmap)) 
	{

	}

	Chunk() : pos({0, 0, 0}),
		blocks(std::make_unique<block[]>(ChunkInfo::VOLUME)),
		lightmap(new Lightmap)
	{

	}

	Chunk(Vector3I pos) : pos(pos),
		blocks(std::make_unique<block[]>(ChunkInfo::VOLUME)), 
		lightmap(new Lightmap) 
	{

	}

	~Chunk() {
		for(int i = 0; i < 6; i++) {
			if (auto n = neighbours_safe[i].lock()) {
				n->neighbours_safe[INVERTED_IND[i]].reset();
			}
		}
	}

	// CREATES LINK BETWEEN TWO CHUNKS //
	inline void loadNeighbour(int ind, const std::shared_ptr<Chunk>& neigh) {
		neighbours_safe[ind] = neigh;
		neigh->neighbours_safe[INVERTED_IND[ind]] = this_weak;
	}    

	// REMOVES LINK BETWEEN TWO CHUNKS //
	inline void removeNeighbour(int ind) {
		if (auto n = neighbours_safe[ind].lock()) {
			n->neighbours_safe[INVERTED_IND[ind]].reset();
			neighbours_safe[ind].reset();
		}
	}

	inline std::weak_ptr<Chunk> getNeighbour(int ind) {
		return neighbours_safe[ind];
	}

	/*
	 * Finds neighbour chunk of current chunk.
	 * @param bx local
	 * @param by local
	 * @param bz local
	 */
	inline std::weak_ptr<Chunk> findNeighbourChunk(int bx, int by, int bz) {
		if (bx >= 0 && bx < ChunkInfo::WIDTH &&
			by >= 0 && by < ChunkInfo::HEIGHT &&
			bz >= 0 && bz < ChunkInfo::DEPTH) {
			return this_weak;
		}
		Neighbour idx = INDEX(bx, by, bz);
    	return neighbours_safe[static_cast<size_t>(idx)];
	}

	inline block getBoundBlock(int32_t x, int32_t y, int32_t z) {
		if (x >= 0 && x < ChunkInfo::WIDTH  &&
			y >= 0 && y < ChunkInfo::HEIGHT &&
			z >= 0 && z < ChunkInfo::DEPTH) {
			return getBlock(x, y, z);
		}
		std::weak_ptr<Chunk> chunk = findNeighbourChunk(x, y, z);
		if (auto raw = chunk.lock()) {
			int32_t nx, ny, nz;
			LOCAL(nx, ny, nz, x, y, z);

			if (nx < 0 || nx >= ChunkInfo::WIDTH ||
				ny < 0 || ny >= ChunkInfo::HEIGHT ||
				nz < 0 || nz >= ChunkInfo::DEPTH) {
				return block{};
			}
			return raw->getBlock(nx, ny, nz);
		}
		else return block{};
	}

   	inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const {return blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx];}
	inline void  setBlock(int32_t lx, int32_t ly, int32_t lz, uint8_t id) {blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx].id = id;}

	uint8_t getBoundLight(int lx, int ly, int lz, int channel);
	uint8_t getLight(int32_t lx, int32_t, int32_t lz, int32_t channel) const;

	void setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission);

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