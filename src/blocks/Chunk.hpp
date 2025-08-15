#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <array>
#include <memory>

#include "ChunkInfo.hpp"
#include "Block.hpp"

#include "../lighting/LightMap.hpp"
#include "../noise/PerlinNoise.hpp"
#include "../graphics/renderer/Drawable.hpp"

#include <iostream>

const int OFFSETS[26][3] = {
	{-1,-1,-1}, {0,-1,-1}, {1,-1,-1},
	{-1,0,-1}, {0,0,-1}, {1,0,-1},
	{-1,1,-1}, {0,1,-1}, {1,1,-1},
	{-1,-1,0}, {0,-1,0}, {1,-1,0},
	{-1,0,0},           {1,0,0},
	{-1,1,0},  {0,1,0}, {1,1,0},
	{-1,-1,1}, {0,-1,1}, {1,-1,1},
	{-1,0,1},  {0,0,1},  {1,0,1},
	{-1,1,1},  {0,1,1},  {1,1,1}
};

class Chunks;

class Chunk {
	/* Chunk has 4 horizontal neighbours, 2 vertical and 20 corner neigbours for correct lighting on chunk borders. */
	Chunk* neighbors[26];
	
	std::unique_ptr<block[]> blocks;
	std::unique_ptr<Lightmap> lightmap;

	friend class Chunks;
	friend class ChunkRLE;

	bool modified;

	Chunks* world;
public:
	DrawableObject chunk_draw;

	// World Pos //
	int32_t x, y, z;
	glm::vec3 min;
	glm::vec3 max;

	Chunk(int x, int y, int z) : x(x), y(y), z(z) {}

	inline int getNeighbourIndex(int lx, int ly, int lz) {
		int dx = (lx < 0) ? -1 : (lx >= ChunkInfo::WIDTH  ? 1 : 0);
		int dy = (ly < 0) ? -1 : (ly >= ChunkInfo::HEIGHT ? 1 : 0);
		int dz = (lz < 0) ? -1 : (lz >= ChunkInfo::DEPTH  ? 1 : 0);

		if (dx == 0 && dy == 0 && dz == 0)
			return -1;

		for (int i = 0; i < 26; ++i) {
			if (OFFSETS[i][0] == dx && OFFSETS[i][1] == dy && OFFSETS[i][2] == dz)
				return i;
		}
		return -1;
	}
	

	/*
	 * Finds neighbour chunk of current chunk.
	 * @param bx local
	 * @param by local
	 * @param bz local
	 */
	inline Chunk* findNeighbourChunk(int bx, int by, int bz) {
		if (bx >= 0 && bx < ChunkInfo::WIDTH  &&
			by >= 0 && by < ChunkInfo::HEIGHT &&
			bz >= 0 && bz < ChunkInfo::DEPTH)
			return this;
		return neighbors[getNeighbourIndex(bx, by, bz)];
	}

	/*
	 * Finds neighbour chunk of current chunk.
	 * @param lx local
	 * @param ly local
	 * @param lz local
	 * @param out_x New local x
	 * @param out_y New local y
	 * @param out_z New local z
	 */
	inline Chunk* findNeighbourChunk(int lx, int ly, int lz, int &out_x, int &out_y, int &out_z) {
		if (lx >= 0 && lx < ChunkInfo::WIDTH &&
			ly >= 0 && ly < ChunkInfo::HEIGHT &&
			lz >= 0 && lz < ChunkInfo::DEPTH) {
			out_x = lx;
			out_y = ly;
			out_z = lz;
			return this;
		}
		Chunk* chunk = neighbors[getNeighbourIndex(lx, ly, lz)];
		if (chunk!=nullptr) {
			out_x = lx - (chunk->x-x) * ChunkInfo::WIDTH;
			out_y = ly - (chunk->y-y) * ChunkInfo::HEIGHT;
			out_z = lz - (chunk->z-z) * ChunkInfo::DEPTH;
		}
		return chunk;
	}

	inline block getBoundBlock(int32_t lx, int32_t ly, int32_t lz) {
		if (lx >= 0 && lx < ChunkInfo::WIDTH &&
			ly >= 0 && ly < ChunkInfo::HEIGHT &&
			lz >= 0 && lz < ChunkInfo::DEPTH) return getBlock(lx, ly, lz);
		int nx, ny, nz;
		Chunk* chunk = findNeighbourChunk(lx, ly, lz, nx, ny, nz);

		if (!chunk) return block{};

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
