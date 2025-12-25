#ifndef BLOCKRENDERER_HPP
#define BLOCKRENDERER_HPP

class Camera;
class Mesh;
class Chunk;
class LogicSystem;
class Shader;
class Frustum;

#include "ChunkMesher.hpp"
#include "Renderer.hpp"
#include "GlController.hpp"

class BlockRenderer : public Renderer {
	LogicSystem* world;
	ChunkMesher mesher;
public:
	BlockRenderer(LogicSystem* chunks) : world(chunks), mesher(chunks) {}

	void render() override;
};

#endif // !BLOCKRENDERER_HPP
