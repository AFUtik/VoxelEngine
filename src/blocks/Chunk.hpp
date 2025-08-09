#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <memory>
#include "Block.hpp"
#include "../noise/PerlinNoise.hpp"
#include "../graphics/renderer/Drawable.hpp"

class Lightmap;

class Chunk {

	std::unique_ptr<block[]> blocks;

	friend class Chunks;
public:
	std::unique_ptr<Lightmap> lightmap;
	DrawableObject chunk_draw;

	static uint32_t WIDTH;
	static uint32_t HEIGHT;
	static uint32_t DEPTH;
	static uint32_t VOLUME;

	static constexpr void setChunkSize(uint32_t w, uint32_t h, uint32_t d) {
		WIDTH = w;
		HEIGHT = h;
		DEPTH = d;
		VOLUME = w*h*d;
	}

	int32_t x, y, z;

	inline block& getBlock(const int32_t bx, const int32_t by, const int32_t bz) {return blocks[(by * DEPTH + bz) * WIDTH + bx];}

	Chunk(int x, int y, int z, PerlinNoise& generator);
};

#endif // !CHUNK_HPP
