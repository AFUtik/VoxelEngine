#ifndef BLOCKRENDERER_HPP
#define BLOCKRENDERER_HPP


class Mesh;
class Chunk;
class Chunks;

#include "ChunkMesher.hpp"

class BlockRenderer {
	ChunkMesher mesher;
	Chunks *world;
public:
	BlockRenderer(Chunks* world) :  mesher(world), world(world)  {};

	void renderAll();
};

#endif // !BLOCKRENDERER_HPP
