#ifndef BLOCKRENDERER_HPP
#define BLOCKRENDERER_HPP

#include <stdlib.h>

class Mesh;
class Chunk;

class BlockRenderer {
	size_t capacity;
	float* buffer;
public:
	
	BlockRenderer(size_t capacity);
	~BlockRenderer();

	Mesh* render(Chunk* chunk);
};

#endif // !BLOCKRENDERER_HPP
