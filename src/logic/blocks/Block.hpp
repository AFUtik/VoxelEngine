#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <cstdint>

struct block {
	uint16_t id    = 0;
	uint16_t meta  = 0;
	inline bool isOpaque() {return id;}
};

#endif // !CHUNK_HPP
