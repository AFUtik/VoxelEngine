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
		if (!chunk->chunk_draw.getMesh() || chunk->isModified()) {

			mesher.makeChunk(chunk.get());
			
			chunk->chunk_draw.loadShader(shader);
			chunk->chunk_draw.getMesh()->upload_buffers();

			double px = static_cast<double>(chunk->x) * static_cast<double>(ChunkInfo::WIDTH)  + 0.5;
			double py = static_cast<double>(chunk->y) * static_cast<double>(ChunkInfo::HEIGHT) + 0.5;
			double pz = static_cast<double>(chunk->z) * static_cast<double>(ChunkInfo::DEPTH)  + 0.5;

			chunk->chunk_draw.getTransform().setPosition(glm::dvec3(px, py, pz));

			chunk->unmodify();
		}

		// if(frustum->boxInFrustum(chunk->min, chunk->max)) {
			chunk->chunk_draw.draw(camera);
		// }
	}
}
