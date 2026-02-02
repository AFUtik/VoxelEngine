#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <ptypes.hpp>

#include "Block.hpp"

// LOGIC
#include "../lighting/LightMap.hpp"
#include "../lighting/LightInfo.hpp"
#include "ChunkInfo.hpp"
#include "chunk_utils.hpp"

class Chunks;

enum class FaceDirection {
    POS_X,
    NEG_X,
    POS_Y,
    NEG_Y,
    POS_Z, 
    NEG_Z
};

const int FACES_ARRAY[] = {
	21, 4, 15, 10, 13, 12
};

class Chunk {
protected:
	std::weak_ptr<Chunk> neigbours_safe[26];
	Chunk* neighbors[26] {nullptr};

	friend class World;
	friend class ChunkCompressor;
	friend class LightSolver;
public:
	std::unique_ptr<Lightmap> lightmap;
	std::unique_ptr<block[]> blocks;
	Vector3I pos;
	uint32_t version = 1;

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
		for(int i = 0; i < 26; i++) {
			if(neighbors[i]) neighbors[i]->neighbors[25-i] = nullptr;
		}
	}

	inline void loadNeighbour(int ind, Chunk *neigh) {neighbors[ind] = neigh;}    
	
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
		gx = x + chunk->pos.x * ChunkInfo::WIDTH;
		gy = y + chunk->pos.y * ChunkInfo::HEIGHT;
		gz = z + chunk->pos.z * ChunkInfo::DEPTH;
	}
	
	static inline bool inside(int x, int y, int z) {
		return (x >= 0 && x < ChunkInfo::WIDTH  &&
			    y >= 0 && y < ChunkInfo::HEIGHT &&
			    z >= 0 && z < ChunkInfo::DEPTH);
	}

	inline Chunk* getNeigbour(int ind) {
		return neighbors[ind];
	}

	template<FaceDirection FD> 
	inline Chunk* getNeighbourByFace() {
		if constexpr(FD==FaceDirection::POS_X) return neighbors[13]; 
		else if constexpr(FD==FaceDirection::NEG_X) return neighbors[12]; 
		else if constexpr(FD==FaceDirection::POS_Y) return neighbors[15]; 
		else if constexpr(FD==FaceDirection::NEG_Y) return neighbors[10]; 
		else if constexpr(FD==FaceDirection::POS_Z) return neighbors[21]; 
		else if constexpr(FD==FaceDirection::NEG_Z) return neighbors[4]; 
	}
	
	inline Chunk* getNeigbourByFace(int face) {
		return neighbors[FACES_ARRAY[face]];
	}

	inline int getNeighbourIndex(int lx, int ly, int lz) const {
		int dx = (lx < 0) ? -1 : (lx >= ChunkInfo::WIDTH ? 1 : 0);
		int dy = (ly < 0) ? -1 : (ly >= ChunkInfo::HEIGHT ? 1 : 0);
		int dz = (lz < 0) ? -1 : (lz >= ChunkInfo::DEPTH ? 1 : 0);

		if (dx == 0 && dy == 0 && dz == 0) return -1;
		return NEI_INDEX_BY_ENCODE[ encode3(dx, dy, dz) ];
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
    	return neighbors[idx];
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
	inline void  setBlock(int32_t lx, int32_t ly, int32_t lz, uint8_t id) {blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx].id = id;}

	uint8_t getBoundLight(int lx, int ly, int lz, int channel);
	uint8_t getLight(int32_t lx, int32_t, int32_t lz, int32_t channel) const;

	void setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission);
};

using ChunkPtr  = std::shared_ptr<Chunk>;
using ChunkRef  = const std::shared_ptr<Chunk>&;
using ChunkWeak = std::weak_ptr<Chunk>;
using ChunkUPtr = std::unique_ptr<Chunk>;

#endif