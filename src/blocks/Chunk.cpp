#include "blocks/Block.hpp"
#include "blocks/Chunk.hpp"

#include <iostream>
#include <chrono>

#include "noise/perlin_noise.hpp"

#include <cmath>

Chunk::Chunk() {
	PerlinNoise p(0);
	const float scale = 0.05f;
	auto start = std::chrono::high_resolution_clock::now();
	blocks = new block[CHUNK_VOL];
	for (int y = 0; y < CHUNK_H; y++) {
		for (int z = 0; z < CHUNK_D; z++) {
			for (int x = 0; x < CHUNK_W; x++) {
				int id = 0;
				float value = p.noise2d(x * scale, y * scale);
				if (0.5f <= value) {
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