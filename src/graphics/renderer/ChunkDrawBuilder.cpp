#include "ChunkDrawBuilder.hpp"

#include "../model/Mesh.hpp"
#include "../../lighting/LightMap.hpp"

#include "../../blocks/Chunk.hpp"

void ChunkDrawBuilder::build(Chunk *chunk) {
	this->chunk = chunk;
    this->consumer = chunk->chunk_draw.getMesh()->get_consumer();
    this->lightmap = chunk->lightmap.get();

	for (int y = 0; y < CHUNK_H; y++) {
		for (int z = 0; z < CHUNK_D; z++) {
			for (int x = 0; x < CHUNK_W; x++) {
				block& vox = chunk->blocks[(y * CHUNK_D + z) * CHUNK_W + x];
				unsigned int id = vox.id;

				if (!id) {
					continue;
				}

				float uvsize = 1.0f / 16.0f;
				float u = (id % 16) * uvsize;
				float v = 1 - ((1 + id / 16) * uvsize);

				if (!is_blocked(x, y + 1, z)) {
					constexpr glm::vec3 offsets[4] = {
						{-0.5f, 0.5f, -0.5f},
						{-0.5f, 0.5f,  0.5f},
						{ 0.5f, 0.5f,  0.5f},
						{ 0.5f, 0.5f, -0.5f}
					};
					emit_face(x, y, z, offsets, u, v, uvsize);
				}
				if (!is_blocked(x, y - 1, z)) {
					constexpr glm::vec3 offsets[4] = {
						{-0.5f, -0.5f, -0.5f},
						{-0.5f, -0.5f,  0.5f},
						{ 0.5f, -0.5f,  0.5f},
						{ 0.5f, -0.5f, -0.5f}
					};
					emit_face(x, y, z, offsets, u, v, uvsize, true);
				}

				if (!is_blocked(x + 1, y, z)) {
					constexpr glm::vec3 offsets[4] = {
						{0.5f, -0.5f, -0.5f},
						{0.5f, -0.5f,  0.5f},
						{ 0.5f, 0.5f,  0.5f},
						{ 0.5f, 0.5f, -0.5f}
					};
					emit_face(x, y, z, offsets, u, v, uvsize, true);
				}
				if (!is_blocked(x - 1, y, z)) {
					constexpr glm::vec3 offsets[4] = {
						{-0.5f, -0.5f, -0.5f},
						{-0.5f, -0.5f,  0.5f},
						{-0.5f, 0.5f,  0.5f},
						{-0.5f, 0.5f, -0.5f}
					};
					emit_face(x, y, z, offsets, u, v, uvsize);
				}

				if (!is_blocked(x, y, z + 1)) {
					constexpr glm::vec3 offsets[4] = {
						{0.5f, -0.5f, 0.5f},
						{-0.5f, -0.5f,  0.5f},
						{-0.5f, 0.5f,  0.5f},
						{0.5f, 0.5f, 0.5f}
					};
					emit_face(x, y, z, offsets, u, v, uvsize, true);
				}
				if (!is_blocked(x, y, z - 1)) {
					constexpr glm::vec3 offsets[4] = {
						{0.5f, -0.5f, -0.5f},
						{-0.5f, -0.5f,  -0.5f},
						{-0.5f, 0.5f,  -0.5f},
						{0.5f, 0.5f, -0.5f}
					};
					emit_face(x, y, z, offsets, u, v, uvsize);
				}
			}
		}
	}
}
