#ifndef CHUNKS_HPP
#define CHUNKS_HPP

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>

#include <memory>

#include <glm/glm.hpp>

#include "Chunk.hpp"

class Chunk;
struct block;

inline uint64_t hash_xyz(int x, int y, int z) noexcept {
	return (static_cast<uint64_t>(x) * 73856093) ^ (static_cast<uint64_t>(y) * 19349663) ^ (static_cast<uint64_t>(z) * 83492791);
}

class Chunks {
	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> chunk_map;
public:
	std::vector<Chunk*> iterable;
	Chunks(int w, int h, int d);

	block getBlock(int x, int y, int z);
	Chunk* getChunk(int x, int y, int z);
	Chunk* getChunkByBlock(int x, int y, int z);
	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);

	void write(unsigned char* dest);
	void read(unsigned char* source);
};

#endif // !CHUNKS_HPP
