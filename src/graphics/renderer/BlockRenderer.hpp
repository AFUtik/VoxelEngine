#ifndef BLOCKRENDERER_HPP
#define BLOCKRENDERER_HPP

class Camera;
class Mesh;
class Chunk;
class World;
class Shader;
class Frustum;

#include "ChunkMesher.hpp"
#include "Renderer.hpp"
#include "GlController.hpp"

class BlockRenderer : public Renderer {
	World* world;
	Mesher mesher;
public:
	BlockRenderer(World* chunks) : world(chunks), mesher(chunks) {}

	void render() override;
};

#endif // !BLOCKRENDERER_HPP
