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
	BlockRenderer(Chunks* world) :  world(world)  {};

	void generateMeshes();
	void renderAll(uint32_t shader, const glm::dvec3& camera_pos);
};

#endif // !BLOCKRENDERER_HPP
