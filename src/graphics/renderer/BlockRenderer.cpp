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
#include <memory>

#include "../../lighting/LightMap.hpp"
#include "../../graphics/Transform.hpp"
#include "../../blocks/ChunkInfo.hpp"

#include "../Frustum.hpp"
#include "../../graphics/Camera.hpp"

void BlockRenderer::render() {
	for(const auto& [chunkPos, chunk] : world->chunkMap) {
		if (!chunk->chunk_draw.getMesh()) {
			mesher.makeChunk(chunk.get());
			
			chunk->chunk_draw.loadShader(shader);
			chunk->chunk_draw.getMesh()->upload_buffers();
			chunk->chunk_draw.getTransform().setPosition(glm::dvec3(chunk->x * ChunkInfo::WIDTH + 0.5, chunk->y * ChunkInfo::HEIGHT + 0.5f, chunk->z * ChunkInfo::DEPTH + 0.5f));
		}

		// if(frustum->boxInFrustum(chunk->min, chunk->max)) {
			chunk->chunk_draw.draw(camera);
		// }
	}
}
