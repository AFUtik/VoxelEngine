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

inline void emitFace(
	VertexConsumer& consumer,
	float x, float y, float z,
	const glm::vec3 offsets[4],
	float u, float v, float uvsize, float l,
	bool flip = false)
{
	for (int i = 0; i < 4; i++) {
		const glm::vec3 &offset = offsets[i];

		consumer.vertex(Vertex{x+offset.x, y+offset.y, z+offset.z,
			i == 0 || i == 1 ? u + uvsize : u,
			i == 0 || i == 3 ? v : v + uvsize,
			0.0, 0.0, 0.0, l/15.0f});
	}

	if (flip) consumer.index(3, 1, 0).index(3, 2, 1).endIndex();
	else      consumer.index(0, 1, 3).index(1, 2, 3).endIndex();
}

void BlockRenderer::renderAll() {
	for(Chunk* chunk : world->iterable) {
		mesher.makeChunk(chunk);

		chunk->chunk_draw.mesh->upload_buffers();
		chunk->chunk_draw.mesh->updateInstanceBuffer(
		0,
		0,
		translate(mat4(1.0f), vec3(chunk->x * Chunk::WIDTH + 0.5f, chunk->y * Chunk::HEIGHT + 0.5f, chunk->z * Chunk::DEPTH + 0.5f))
		);
	}
}
