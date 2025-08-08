#include "Block.hpp"
#include "Chunk.hpp"

#include <iostream>
#include <chrono>

#include "../noise/PerlinNoise.hpp"
#include "../lighting/LightMap.hpp"

#include <glm/ext.hpp>
#include <cmath>

Chunk::Chunk(int x, int y, int z, PerlinNoise& generator) :
	lightmap(new Lightmap),
	blocks(std::make_unique<block[]>(CHUNK_VOL)),
	x(x), y(y), z(z) {
	const float scale = 0.05f;
	for (int _y = 0; _y < CHUNK_H; _y++) {
		for (int _z = 0; _z < CHUNK_D; _z++) {
			for (int _x = 0; _x < CHUNK_W; _x++) {
				int real_x = _x + this->x * CHUNK_W;
				int real_y = _y + this->y * CHUNK_H;
				int real_z = _z + this->z * CHUNK_D;
				float value = generator.noise(real_x*scale, real_y*scale, real_z*scale);
				int id = 0;
				if (value <= 0.1)
					id = 1;
				blocks[(_y * CHUNK_D + _z) * CHUNK_W + _x].id = id;
			}
		}
	}
}