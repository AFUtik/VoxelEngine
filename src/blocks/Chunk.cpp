#include "blocks/Block.hpp"
#include "blocks/Chunk.hpp"

#include <iostream>
#include <chrono>

#include "noise/perlin_noise3d.hpp"

#include <cmath>

Chunk::Chunk() {
	PerlinGenerator3D* p = new PerlinGenerator3D(12, 12, 12, 
												 64, 64, 64,
												 CHUNK_W, CHUNK_H, CHUNK_D, 2);
	auto start = std::chrono::high_resolution_clock::now();
	blocks = new block[CHUNK_VOL];
	for (int y = 0; y < CHUNK_H; y++) {
		for (int z = 0; z < CHUNK_D; z++) {
			for (int x = 0; x < CHUNK_W; x++) {
				int id = 0;
				if (10.0f <= p->noise(x, y, z)) {
					id = 2;
				}
				blocks[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	std::cout << "Generation of Chunk was completed in " << duration.count() << " seconds." << std::endl;
}

Chunk::~Chunk() {
	delete[] blocks;
}