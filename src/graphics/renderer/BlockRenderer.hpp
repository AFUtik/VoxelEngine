#ifndef BLOCKRENDERER_HPP
#define BLOCKRENDERER_HPP

class Camera;
class Mesh;
class Chunk;
class Chunks;
class Shader;
class Frustum;

#include "ChunkMesher.hpp"
#include "Renderer.hpp"

class BlockRenderer : public Renderer {
	ChunkMesher mesher;
	Chunks* world;
public:
	BlockRenderer(Chunks* chunks) : world(chunks) {}

	void generateMeshes();
	void render() override;
};

#endif // !BLOCKRENDERER_HPP
