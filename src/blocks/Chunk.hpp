#ifndef CHUNK_HPP
#define CHUNK_HPP

#define CHUNK_W 16
#define CHUNK_H 128
#define CHUNK_D 16
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

#include <memory>
#include "Block.hpp"
#include "../noise/PerlinNoise.hpp"
#include "../graphics/renderer/Drawable.hpp"

class Lightmap;

class Chunk {
public:
	std::unique_ptr<Lightmap> lightmap;
	std::unique_ptr<block[]> blocks;
	DrawableObject chunk_draw;
	int x, y, z;

	Chunk(int x, int y, int z, PerlinNoise& generator);
};

#endif // !CHUNK_HPP
