#ifndef BLOCKRENDERER_HPP
#define BLOCKRENDERER_HPP

class Camera;
class Mesh;
class Chunk;
class Chunks;
class Shader;

#include "ChunkMesher.hpp"

class BlockRenderer {
	ChunkMesher mesher;
	Chunks *world;

	Shader* shader;
public:
	BlockRenderer(Chunks* world, Shader* shader) :  world(world), shader(shader)  {};

	void generateMeshes();
	void renderAll(Camera* camera);
};

#endif // !BLOCKRENDERER_HPP
