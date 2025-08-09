#ifndef LIGHTMAP_H_
#define LIGHTMAP_H_

#include "../blocks/Chunk.hpp"

#include <cstdint>

class Lightmap {
public:
	unsigned short* map;
	Lightmap();
	~Lightmap();

	inline unsigned char get(int x, int y, int z, int channel) {
		return (map[y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x] >> (channel << 2)) & 0xF;
	}

	inline unsigned char getR(int x, int y, int z) {
		return map[y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x] & 0xF;
	}

	inline unsigned char getG(int x, int y, int z) {
		return (map[y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x] >> 4) & 0xF;
	}

	inline unsigned char getB(int x, int y, int z) {
		return (map[y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x] >> 8) & 0xF;
	}

	inline unsigned char getS(int x, int y, int z) {
		return (map[y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x] >> 12) & 0xF;
	}

	inline void setR(int x, int y, int z, int value) {
		const int index = y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x;
		map[index] = (map[index] & 0xFFF0) | value;
	}

	inline void setG(int x, int y, int z, int value) {
		const int index = y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x;
		map[index] = (map[index] & 0xFF0F) | (value << 4);
	}

	inline void setB(int x, int y, int z, int value) {
		const int index = y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x;
		map[index] = (map[index] & 0xF0FF) | (value << 8);
	}

	inline void setS(int x, int y, int z, int value) {
		const int index = y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x;
		map[index] = (map[index] & 0x0FFF) | (value << 12);
	}

	inline void set(int x, int y, int z, int channel, int value) {
		const int index = y * Chunk::DEPTH * Chunk::WIDTH + z * Chunk::WIDTH + x;
		map[index] = (map[index] & (0xFFFF & (~(0xF << (channel * 4))))) | (value << (channel << 2));
	}
};

#endif /* LIGHTMAP_HPP */