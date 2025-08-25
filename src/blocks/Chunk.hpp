#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <array>
#include <memory>
#include <shared_mutex>

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

const int FACE_DIRS[6][3] = {
	{0,0,1}, 
	{0,0,-1}, 
	{0,1,0}, 
	{0,-1,0}, 
	{1,0,0}, 
	{-1,0,0}
};

static constexpr int encode3(int dx, int dy, int dz) noexcept {
    return (dx + 1) + (dy + 1) * 3 + (dz + 1) * 9; // 0..26
}

static std::array<int, 27> buildNeiIndexByEncode() {
    std::array<int,27> t;
    t.fill(-1);
    for (int i = 0; i < 26; ++i) {
        int dx = OFFSETS[i][0];
        int dy = OFFSETS[i][1];
        int dz = OFFSETS[i][2];
        t[ encode3(dx, dy, dz) ] = i;
    }
    return t;
}

static const std::array<int,27> NEI_INDEX_BY_ENCODE = buildNeiIndexByEncode();

static std::array<int,26> makeOpposites() {
        std::array<int,26> arr{};
        for (int i = 0; i < 26; ++i) {
            arr[i] = -1;
            for (int j = 0; j < 26; ++j) {
                if (OFFSETS[j][0] == -OFFSETS[i][0] &&
                    OFFSETS[j][1] == -OFFSETS[i][1] &&
                    OFFSETS[j][2] == -OFFSETS[i][2]) {
                    arr[i] = j;
                    break;
                }
            }
            assert(arr[i] != -1 && "No opposite offset found!");
        }
        return arr;
    }

static inline std::array<int,26> oppositeIdx = makeOpposites();

static constexpr inline int floorDiv(int a, int b) {
	if (b < 0) b = -b;
	int q = a / b;
	int r = a % b;
	if (r != 0 && ((r < 0) != (b < 0))) --q;
	return q;
}

static inline int dirFromDelta(int dx, int dy, int dz) {
    for (int d = 0; d < 6; ++d) {
        if (FACE_DIRS[d][0] == dx && FACE_DIRS[d][1] == dy && FACE_DIRS[d][2] == dz) return d;
    }
    return -1;
}


static inline int faceToIdx(int face) {
    assert(face >= 0 && face < 6);
    int dx = FACE_DIRS[face][0];
    int dy = FACE_DIRS[face][1];
    int dz = FACE_DIRS[face][2];

    for (int i = 0; i < 26; ++i) {
        if (OFFSETS[i][0] == dx &&
            OFFSETS[i][1] == dy &&
            OFFSETS[i][2] == dz) {
            return i;
        }
    }
    return -1;
}

class Chunks;

class Chunk {
	/* Chunk has 4 horizontal neighbours, 2 vertical and 20 corner neigbours for correct lighting on chunk borders. */
	Chunk* neighbors[26];
	mutable std::shared_mutex dataMutex;
	
	std::unique_ptr<block[]> blocks;
	std::unique_ptr<Lightmap> lightmap;

	friend class Chunks;
	friend class ChunkRLE;
	friend class ChunkMesher;
	friend class LightSolver;

	bool modified = false;

	Chunks* world;
public:
	DrawableObject chunk_draw;

	// World Pos //
	int32_t x, y, z;
	glm::vec3 min;
	glm::vec3 max;

	Chunk(int x, int y, int z) : x(x), y(y), z(z) {}

	inline void unmodify() {modified = false;}
	inline void modify() {modified = true;}
	inline bool isModified() {return modified;}

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
		if (idx < 0 || idx >= 26) return nullptr;
		return neighbors[idx];
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
			lz >= 0 && lz < ChunkInfo::DEPTH) 
		{
			out_x = lx; out_y = ly; out_z = lz;
			return this;
		}
		int idx = getNeighbourIndex(lx, ly, lz);
		if (idx < 0 || idx >= 26) return nullptr;
		Chunk* chunk = neighbors[idx];
		if (!chunk) return nullptr;

		out_x = lx - (chunk->x - x) * ChunkInfo::WIDTH;
		out_y = ly - (chunk->y - y) * ChunkInfo::HEIGHT;
		out_z = lz - (chunk->z - z) * ChunkInfo::DEPTH;
		return chunk;
	}

	inline block getBoundBlock(int32_t lx, int32_t ly, int32_t lz) {
		if (lx >= 0 && lx < ChunkInfo::WIDTH &&
			ly >= 0 && ly < ChunkInfo::HEIGHT &&
			lz >= 0 && lz < ChunkInfo::DEPTH) {
			return getBlock(lx, ly, lz);
		}
		int nx, ny, nz;
		Chunk* chunk = findNeighbourChunk(lx, ly, lz, nx, ny, nz);
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