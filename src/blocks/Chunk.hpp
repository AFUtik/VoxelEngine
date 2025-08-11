#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <array>
#include <memory>
#include "Block.hpp"
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

class Lightmap;
class Chunks;

class Chunk {
	Chunk* neighbors[26];
	std::unique_ptr<block[]> blocks;
	std::unique_ptr<Lightmap> lightmap;

	friend class Chunks;

	Chunks* world;
public:

	DrawableObject chunk_draw;

	static uint32_t WIDTH;
	static uint32_t HEIGHT;
	static uint32_t DEPTH;
	static uint32_t VOLUME;

	int32_t x, y, z;

	inline static constexpr void setChunkSize(uint32_t w, uint32_t h, uint32_t d) {
		WIDTH = w;
		HEIGHT = h;
		DEPTH = d;
		VOLUME = w*h*d;
	}

	inline int getNeighbourIndex(int lx, int ly, int lz) {
		int dx = (lx < 0) ? -1 : (lx >= WIDTH  ? 1 : 0);
		int dy = (ly < 0) ? -1 : (ly >= HEIGHT ? 1 : 0);
		int dz = (lz < 0) ? -1 : (lz >= DEPTH  ? 1 : 0);

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
		if (bx >= 0 && bx < WIDTH &&
			by >= 0 && by < HEIGHT &&
			bz >= 0 && bz < DEPTH)
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
		if (lx >= 0 && lx < WIDTH &&
			ly >= 0 && ly < HEIGHT &&
			lz >= 0 && lz < DEPTH) {
			out_x = lx;
			out_y = ly;
			out_z = lz;
			return this;
		}
		Chunk* chunk = neighbors[getNeighbourIndex(lx, ly, lz)];
		if (chunk!=nullptr) {
			out_x = lx - (chunk->x-x) * WIDTH;
			out_y = ly - (chunk->y-y) * HEIGHT;
			out_z = lz - (chunk->z-z) * DEPTH;
		}
		return chunk;
	}

	inline block getBoundBlock(int32_t lx, int32_t ly, int32_t lz) {
		if (lx >= 0 && lx < WIDTH &&
			ly >= 0 && ly < HEIGHT &&
			lz >= 0 && lz < DEPTH) return getBlock(lx, ly, lz);
		int nx, ny, nz;
		Chunk* chunk = findNeighbourChunk(lx, ly, lz, nx, ny, nz);

		if (!chunk) return block{};

		return chunk->getBlock(nx, ny, nz);
	}

	inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const {return blocks[(ly * DEPTH + lz) * WIDTH + lx];}

	inline void setBlock(int32_t lx, int32_t ly, int32_t lz, uint8_t id) const {blocks[(ly * DEPTH + lz) * WIDTH + lx].id = id;}

	uint8_t getBoundLight(int lx, int ly, int lz, int channel);

	uint8_t getLight(int32_t lx, int32_t, int32_t lz, int32_t channel) const;

	void Chunk::setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission);

	Chunk(int x, int y, int z, PerlinNoise& generator);
};

#endif // !CHUNK_HPP
