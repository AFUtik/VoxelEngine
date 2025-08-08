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

inline bool is_blocked(int x, int y, int z, Chunk* chunk, Chunks* chunks) {
	return chunks->get(chunk->x*CHUNK_W+x, chunk->y*CHUNK_H+y, chunk->z*CHUNK_D+z).id;
}

inline unsigned char get_light(int x, int y, int z, int channel, Chunks* chunks) {
	Chunk* chunk = chunks->getChunkByBlock(x, y, z);
	if (chunk == nullptr) return 0;
	return chunk->lightmap->get(
		x >= CHUNK_W ? (x - CHUNK_W) : (x < 0) ? (CHUNK_W+x) : x,
		y >= CHUNK_H ? (y - CHUNK_H) : (y < 0) ? (CHUNK_H+y) : y,
		z >= CHUNK_D ? (z - CHUNK_D) : (z < 0) ? (CHUNK_D+z) : z, channel
		);
}

inline void emitFace(
	VertexConsumer& consumer,
	float x, float y, float z,
	const glm::vec3 offsets[4],
	float u, float v, float uvsize,
	bool flip = false)
{
	for (int i = 0; i < 4; i++) {
		const glm::vec3 &offset = offsets[i];

		consumer.vertex(Vertex{x+offset.x, y+offset.y, z+offset.z,
			i == 0 || i == 1 ? u + uvsize : u,
			i == 0 || i == 3 ? v : v + uvsize,
			1, 1, 1, 1});
	}

	if (flip) consumer.index(3, 1, 0).index(3, 2, 1).endIndex();
	else      consumer.index(0, 1, 3).index(1, 2, 3).endIndex();
}

void BlockRenderer::render(Chunk *chunk) {
	Mesh* mesh = chunk->chunk_draw.getMesh();
	VertexConsumer consumer = mesh->get_consumer();

	size_t index = 0;
	for (int y = 0; y < CHUNK_H; y++) {
		for (int z = 0; z < CHUNK_D; z++) {
			for (int x = 0; x < CHUNK_W; x++) {
				block& vox = chunk->blocks[(y * CHUNK_D + z) * CHUNK_W + x];
				unsigned int id = vox.id;

				if (!id) {
					continue;
				}

				if (chunk->x == 5 && chunk->y == 0 && chunk->z == 0) {
					id=2;
				}

				float uvsize = 1.0f / 16.0f;
				float u = (id % 16) * uvsize;
				float v = 1 - ((1 + id / 16) * uvsize);

				if (!is_blocked(x, y + 1, z, chunk, chunks)) {
					constexpr glm::vec3 offsets[4] = {
						{-0.5f, 0.5f, -0.5f},
						{-0.5f, 0.5f,  0.5f},
						{ 0.5f, 0.5f,  0.5f},
						{ 0.5f, 0.5f, -0.5f}
					};
					emitFace(consumer, x, y, z, offsets, u, v, uvsize);
				}
				if (!is_blocked(x, y - 1, z, chunk, chunks)) {
					constexpr glm::vec3 offsets[4] = {
						{-0.5f, -0.5f, -0.5f},
						{-0.5f, -0.5f,  0.5f},
						{ 0.5f, -0.5f,  0.5f},
						{ 0.5f, -0.5f, -0.5f}
					};

					emitFace(consumer, x, y, z, offsets, u, v, uvsize, true);
				}

				if (!is_blocked(x + 1, y, z, chunk, chunks)) {
					constexpr glm::vec3 offsets[4] = {
						{0.5f, -0.5f, -0.5f},
						{0.5f, -0.5f,  0.5f},
						{ 0.5f, 0.5f,  0.5f},
						{ 0.5f, 0.5f, -0.5f}
					};
					emitFace(consumer, x, y, z, offsets, u, v, uvsize, true);
				}
				if (!is_blocked(x - 1, y, z, chunk, chunks)) {
					constexpr glm::vec3 offsets[4] = {
						{-0.5f, -0.5f, -0.5f},
						{-0.5f, -0.5f,  0.5f},
						{-0.5f, 0.5f,  0.5f},
						{-0.5f, 0.5f, -0.5f}
					};
					emitFace(consumer, x, y, z, offsets, u, v, uvsize);
				}

				if (!is_blocked(x, y, z + 1, chunk, chunks)) {
					constexpr glm::vec3 offsets[4] = {
						{0.5f, -0.5f, 0.5f},
						{-0.5f, -0.5f,  0.5f},
						{-0.5f, 0.5f,  0.5f},
						{0.5f, 0.5f, 0.5f}
					};
					emitFace(consumer, x, y, z, offsets, u, v, uvsize, true);
				}
				if (!is_blocked(x, y, z - 1, chunk, chunks)) {
					constexpr glm::vec3 offsets[4] = {
						{0.5f, -0.5f, -0.5f},
						{-0.5f, -0.5f,  -0.5f},
						{-0.5f, 0.5f,  -0.5f},
						{0.5f, 0.5f, -0.5f}
					};
					emitFace(consumer, x, y, z, offsets, u, v, uvsize);
				}
			}
		}
	}





	mesh->upload_buffers();
	mesh->updateInstanceBuffer(0, 0,
		glm::translate(glm::mat4(1.0f), glm::vec3(chunk->x * CHUNK_W + 0.5f, chunk->y * CHUNK_H + 0.5f, chunk->z * CHUNK_D + 0.5f)));
}
