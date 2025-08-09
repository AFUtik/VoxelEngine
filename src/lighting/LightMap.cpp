#include "LightMap.hpp"

Lightmap::Lightmap() {
	map = new unsigned short[Chunk::VOLUME];
	for (unsigned int i = 0; i < Chunk::VOLUME; i++) {
		map[i] = 0x0000;
	}
}

Lightmap::~Lightmap() {
	delete[] map;
}