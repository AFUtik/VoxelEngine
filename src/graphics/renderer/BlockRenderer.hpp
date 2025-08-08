#ifndef BLOCKRENDERER_HPP
#define BLOCKRENDERER_HPP

class Mesh;
class Chunk;
class Chunks;

#include "ChunkDrawBuilder.hpp"

class BlockRenderer {
	Chunks *chunks;
public:
	BlockRenderer(Chunks* world) :  chunks(world) {};

	void render(Chunk *chunk);
};

#endif // !BLOCKRENDERER_HPP
