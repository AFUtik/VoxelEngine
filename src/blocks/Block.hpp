#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "stdint.h"
#include <string>

struct block {
	uint16_t id    = 0;
	uint16_t state = 0;

	inline bool isOpaque() {return id!=0;}
};

#endif // !CHUNK_HPP
