#include "Block.hpp"
#include "Chunk.hpp"

#include <iostream>
#include <chrono>

#include "../noise/PerlinNoise.hpp"
#include "../lighting/LightMap.hpp"

#include <glm/ext.hpp>
#include <cmath>

uint32_t Chunk::WIDTH  = 16;
uint32_t Chunk::HEIGHT = 256;
uint32_t Chunk::DEPTH  = 16;
uint32_t Chunk::VOLUME = WIDTH*HEIGHT*DEPTH;

Chunk::Chunk(int x, int y, int z, PerlinNoise& generator) :
	lightmap(new Lightmap),
	blocks(std::make_unique<block[]>(VOLUME)),
	x(x), y(y), z(z) {
	const float scale = 0.05f;
	for (int _y = 0; _y < HEIGHT; _y++) {
		for (int _z = 0; _z < DEPTH; _z++) {
			for (int _x = 0; _x < WIDTH; _x++) {
				// Global position //
				const int gx = _x + this->x * WIDTH;
				const int gy = _y + this->y * HEIGHT;
				const int gz = _z + this->z * DEPTH;
				float value = generator.noise(
					static_cast<float>(gx)*scale,
					static_cast<float>(gy)*scale,
					static_cast<float>(gz)*scale);
				int id = 0;
				if (value <= 0.01)
					id = 1;
				getBlock(_x, _y, _z).id = id;
			}
		}
	}
}