#include <iostream>

#include "../../blocks/Chunk.hpp"
#include "../../blocks/Block.hpp"
#include "../model/Mesh.hpp"
#include "../vertex/VertexBuffer.hpp"
#include "../vertex/VertexConsumer.hpp"
#include "../../blocks/Chunks.hpp"

#include "BlockRenderer.hpp"
#include <glm/ext.hpp>

#include "../../lighting/LightMap.hpp"

void BlockRenderer::renderAll() {
	for(Chunk* chunk : world->iterable) {
		mesher.makeChunk(chunk);

		chunk->chunk_draw.mesh->upload_buffers();
		chunk->chunk_draw.mesh->updateInstanceBuffer(
		0,
		0,
		translate(mat4(1.0f), vec3(chunk->x * ChunkInfo::WIDTH + 0.5f, chunk->y * ChunkInfo::HEIGHT + 0.5f, chunk->z * ChunkInfo::DEPTH + 0.5f))
		);
	}
}
