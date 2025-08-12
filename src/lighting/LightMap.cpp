#include "LightMap.hpp"

Lightmap::Lightmap() {
	map = new unsigned short[ChunkInfo::VOLUME];
	for (unsigned int i = 0; i < ChunkInfo::VOLUME; i++) {
		map[i] = 0x0000;
	}
}

Lightmap::~Lightmap() {
	delete[] map;
}