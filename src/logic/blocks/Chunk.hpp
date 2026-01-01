#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <atomic>
#include <memory>
#include <shared_mutex>

#include "Block.hpp"

// LOGIC
#include "../lighting/LightMap.hpp"
#include "../lighting/LightInfo.hpp"
#include "ChunkCompressor.hpp"
#include "ChunkInfo.hpp"
#include "chunk_utils.hpp"

// GRAPHICS
#include "../../graphics/renderer/Drawable.hpp"

class Chunks;

struct ChunkPos {
    int32_t x, y, z;
    bool operator==(const ChunkPos& o) const noexcept {
        return x==o.x && y==o.y && z==o.z;
    }

	bool operator<(const ChunkPos& o) const noexcept {
        if (x != o.x) return x < o.x;
        if (y != o.y) return y < o.y;
        return z < o.z;
    }


};

struct ChunkPosHash {
    std::size_t operator()(const ChunkPos& p) const noexcept {
        std::size_t hx = std::hash<int32_t>{}(p.x);
        std::size_t hy = std::hash<int32_t>{}(p.y);
        std::size_t hz = std::hash<int32_t>{}(p.z);
        return hx ^ (hy << 1) ^ (hz << 2);
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
	Compressed,
	NeedsUpdate
};

enum class FaceDirection {
    POS_X,
    NEG_X,
    POS_Y,
    NEG_Y,
    POS_Z, 
    NEG_Z
};

const int FACES_ARRAY[] = {
	13, 12, 21, 4, 15, 10
};

class Chunk {
	Chunk* neighbors[26] {nullptr};

	friend class LogicSystem;
	friend class ChunkCompressor;
	friend class ChunkMesher;
	friend class LightSolver;
	
	std::atomic<ChunkState> state {ChunkState::Empty};
	std::atomic<uint32_t> version;
	
	ChunkPos hash_pos;
	
	std::shared_ptr<DataCompressedRLE> compressed;
public:
	std::unique_ptr<Lightmap> lightmap;
	std::unique_ptr<block[]> blocks;
	
	glm::vec3 min;
	glm::vec3 max;

	int32_t x, y, z;

	mutable std::shared_mutex dataMutex;
	DrawableObject drawable;

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
	
	inline void loadNeighbour(int ind, Chunk *neigh) {
		neighbors[ind] = neigh;
	}    

	Chunk(int x, int y, int z) : x(x), y(y), z(z),
		min(x*ChunkInfo::WIDTH, y*ChunkInfo::HEIGHT, z*ChunkInfo::DEPTH),
		max((x+1)*ChunkInfo::WIDTH, (y+1)*ChunkInfo::HEIGHT, (z+1)*ChunkInfo::DEPTH), 
		hash_pos({x, y, z}), 
		blocks(std::make_unique<block[]>(ChunkInfo::VOLUME)), 
		lightmap(new Lightmap) 
	{

	}

	~Chunk() {
		for(int i = 0; i < 26; i++) {
			if(neighbors[i]) neighbors[i]->neighbors[25-i] = nullptr;
		}
	}

	
	
	/*
	 * Compresses chunk in RLE data.
	 */
	void compress(bool toDelete=true);

	/*
	 * Compresses chunk in RLE data.
	 */
	void decompress(bool toDelete=true);

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

	void Chunk::setLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel, int32_t emission);
};

/*
 * 
 */
class ChunkSnapshot { 
	

	std::unique_ptr<block[]> blocks;

    block border_z[2][ChunkInfo::HEIGHT * ChunkInfo::WIDTH]; // 0 = -Z, 1 = +Z
    block border_x[2][ChunkInfo::HEIGHT * ChunkInfo::DEPTH]; // 0 = -X, 1 = +X

    std::unique_ptr<Lightmap> lightmap;

    uint16_t light_border_z[2][ChunkInfo::HEIGHT * ChunkInfo::WIDTH] {0};
    uint16_t light_border_x[2][ChunkInfo::HEIGHT * ChunkInfo::DEPTH] {0};

    uint16_t light_border_diag[4][ChunkInfo::HEIGHT] {0};
public:
	int x, y, z;
	std::shared_ptr<Chunk> source;

    ChunkSnapshot(std::shared_ptr<Chunk> src) : source(src), blocks(new block[ChunkInfo::VOLUME]), lightmap(new Lightmap), x(src->x), y(src->y), z(src->z) {
		constexpr int W = ChunkInfo::WIDTH; constexpr int H = ChunkInfo::HEIGHT; constexpr int D = ChunkInfo::DEPTH;

		std::copy_n(src->blocks.get(), ChunkInfo::VOLUME, blocks.get());
		std::copy_n(src->lightmap->map, ChunkInfo::VOLUME, lightmap->map);

		Chunk* nXpos = src->getNeighbourByFace<FaceDirection::POS_X>();
		Chunk* nXneg = src->getNeighbourByFace<FaceDirection::NEG_X>();
		Chunk* nZpos = src->getNeighbourByFace<FaceDirection::POS_Z>();
		Chunk* nZneg = src->getNeighbourByFace<FaceDirection::NEG_Z>();

		// -Z (z = 0)
		if (nZneg) {
			for (int y = 0; y < H; ++y) {
				std::copy_n(
					&nZneg->blocks[(y * D + (D - 1)) * W],
					W,
					&border_z[0][y * W]
				);
				std::copy_n(
					&nZneg->lightmap->map[(y * D + (D - 1)) * W],
					W,
					&light_border_z[0][y * W]
				);
			}
		}

		// +Z
		if (nZpos) {
			for (int y = 0; y < H; ++y) {
				std::copy_n(
					&nZpos->blocks[(y * D + 0) * W],
					W,
					&border_z[1][y * W]
				);
				std::copy_n(
					&nZpos->lightmap->map[(y * D + 0) * W],
					W,
					&light_border_z[1][y * W]
				);
			}
		}

		if (nXneg) {
			for (int y = 0; y < H; ++y) {
				for (int z = 0; z < D; ++z) {
					border_x[0][y * D + z] = nXneg->blocks[(y * D + z) * W + (W - 1)];
					light_border_x[0][y * D + z] = nXneg->lightmap->map[(y * D + z) * W + (W - 1)];
				}
			}
		}

		if (nXpos) {
			for (int y = 0; y < H; ++y) {
				for (int z = 0; z < D; ++z) {
        			border_x[1][y * D + z] = nXpos->blocks[(y * D + z) * W + 0];
					light_border_x[1][y * D + z] = nXpos->lightmap->map[(y * D + z) * W + 0];
    			}
			}
		}

		for (int y = 0; y < H; ++y) {
			light_border_diag[0][y] = src->getLight(0,     y, 0,     0); // -X -Z
			light_border_diag[1][y] = src->getLight(0,     y, D - 1, 0); // -X +Z
			light_border_diag[2][y] = src->getLight(W - 1, y, 0,     0); // +X -Z
			light_border_diag[3][y] = src->getLight(W - 1, y, D - 1, 0); // +X +Z
		}
	}

	void commit() {
		if(source) {
			std::copy_n(blocks.get(), ChunkInfo::VOLUME, source->blocks.get());
			std::copy_n(lightmap->map, ChunkInfo::VOLUME, source->lightmap->map);

		}
	}

	inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const {return blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx];}
	inline void  setBlock(int32_t lx, int32_t ly, int32_t lz, int id)       {blocks[(ly * ChunkInfo::DEPTH + lz) * ChunkInfo::WIDTH + lx].id = id;}

	inline uint8_t getLight(int32_t lx, int32_t ly, int32_t lz, int channel) const {return lightmap->get(lx, ly, lz, channel);}
	inline void  setLight(int32_t lx, int32_t ly, int32_t lz, int channel, int emission) {lightmap->set(lx, ly, lz, channel, emission);}

	inline block getBoundBlock(int32_t lx, int32_t ly, int32_t lz) const {
		constexpr int W = ChunkInfo::WIDTH;
		constexpr int H = ChunkInfo::HEIGHT;
		constexpr int D = ChunkInfo::DEPTH;
		if (lx >= 0 && lx < W &&
			ly >= 0 && ly < H &&
			lz >= 0 && lz < D)
		{
			return blocks[(ly * D + lz) * W + lx];
		}
		if (lx < 0) {
			return border_x[0][ly * D + lz];
		}
		else if (lx >= W) {
			return border_x[1][ly * D + lz];
		}
		if (lz < 0) {
			return border_z[0][ly * W + lx];
		}
		else if (lz >= D) {
			return border_z[1][ly * W + lx];
		}
		return block{};
	}

	inline uint8_t getBoundLight(int32_t lx, int32_t ly, int32_t lz, int32_t channel) const {
		constexpr int W = ChunkInfo::WIDTH;
		constexpr int H = ChunkInfo::HEIGHT;
		constexpr int D = ChunkInfo::DEPTH;
		if (lx >= 0 && lx < W &&
			ly >= 0 && ly < H &&
			lz >= 0 && lz < D)
		{
			return lightmap->get(lx, ly, lz, channel);
		}
		if (lx < 0 && lz >= 0 && lz < D) {
			return (light_border_x[0][ly * D + lz] >> (channel * 4)) & 0xF;
		}
		if (lx >= W && lz >= 0 && lz < D) {
			return (light_border_x[1][ly * D + lz] >> (channel * 4)) & 0xF;
		}
		if (lz < 0 && lx >= 0 && lx < W) {
			return (light_border_z[0][ly * W + lx] >> (channel * 4)) & 0xF;
		}
		if (lz >= D && lx >= 0 && lx < W) {
			return (light_border_z[1][ly * W + lx] >> (channel * 4)) & 0xF;
		}
		return 0;
	}
};

using ChunkPtr  = std::shared_ptr<Chunk>;
using ChunkRef  = const std::shared_ptr<Chunk>&;
using ChunkWeak = std::weak_ptr<Chunk>;
using ChunkUPtr = std::unique_ptr<Chunk>;

#endif