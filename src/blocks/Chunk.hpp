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

inline bool isOnFace(int lx, int ly, int lz, int face) {
    const int W = ChunkInfo::WIDTH;
    const int H = ChunkInfo::HEIGHT;
    const int D = ChunkInfo::DEPTH;
    switch(face) {
        case 0: return lz == D-1; // +Z
        case 1: return lz == 0;   // -Z
        case 2: return ly == H-1; // +Y
        case 3: return ly == 0;   // -Y
        case 4: return lx == W-1; // +X
        case 5: return lx == 0;   // -X
    }
    return false;
}

class Chunks;

class Chunk {
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