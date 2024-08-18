#include "blocks/Block.hpp"
#include "blocks/Chunk.hpp"

#include "noise/perlin_noise3d.hpp"

#include <cmath>

Chunk::Chunk() {
	PerlinGenerator3D* p = new PerlinGenerator3D(5, 5, 5, 64, 64, 64, 8);

	blocks = new block[CHUNK_VOL];
	for (int y = 0; y < CHUNK_H; y++) {
		for (int z = 0; z < CHUNK_D; z++) {
			for (int x = 0; x < CHUNK_W; x++) {
				int id = 5.2f <= p->noise(x, y, z);

				if (y <= 2)
					id = 2;
				blocks[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
			}
		}
	}
}

Chunk::~Chunk() {
	delete[] blocks;
}