#include <iostream>

#include "../../blocks/Chunk.hpp"
#include "../../blocks/Block.hpp"
#include "../model/Mesh.hpp"
#include "../vertex/VertexBuffer.hpp"
#include "../vertex/VertexConsumer.hpp"
#include "../../blocks/Chunks.hpp"
#include "../Shader.hpp"
#include "BlockRenderer.hpp"

#include <glm/ext.hpp>

#include "../../lighting/LightMap.hpp"
#include "../../graphics/Transform.hpp"
#include "../../blocks/ChunkInfo.hpp"

void BlockRenderer::generateMeshes() {
	for(Chunk* chunk : world->iterable) {
		mesher.makeChunk(chunk);

		chunk->chunk_draw.loadShader(shader);
		chunk->chunk_draw.getMesh()->upload_buffers();
		chunk->chunk_draw.getTransform().setPosition(glm::dvec3(chunk->x * ChunkInfo::WIDTH + 0.5, chunk->y * ChunkInfo::HEIGHT + 0.5f, chunk->z * ChunkInfo::DEPTH + 0.5f));
	}
}

void BlockRenderer::renderAll(Camera* camera) {
	for(Chunk* chunk : world->iterable) chunk->chunk_draw.draw(camera);
}
