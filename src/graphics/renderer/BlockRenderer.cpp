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

#include <mutex>
#include <shared_mutex>


void BlockRenderer::render() {
    {
        std::lock_guard lk(mesher.meshUploadMutex);
        while (!mesher.meshUploadQueue.empty()) {
            Chunk* chunk = mesher.meshUploadQueue.front();
            
            chunk->chunk_draw.loadShader(shader);
            chunk->chunk_draw.getMesh()->upload_buffers();

            double px = static_cast<double>(chunk->x) * static_cast<double>(ChunkInfo::WIDTH)  + 0.5;
            double py = static_cast<double>(chunk->y) * static_cast<double>(ChunkInfo::HEIGHT) + 0.5;
            double pz = static_cast<double>(chunk->z) * static_cast<double>(ChunkInfo::DEPTH)  + 0.5;

            chunk->chunk_draw.getTransform().setPosition(glm::dvec3(px, py, pz));
            
            mesher.meshUploadQueue.pop();
        }
    }

	for(const auto& [chunkPos, chunk] : world->chunkMap) {
        if(chunk->isModified()) {
            {
                std::lock_guard lk(world->readyQueueMutex);
                world->readyChunks.push(chunk.get());
            }
            chunk->unmodify();
            world->readyCv.notify_one();
        }

        Mesh* mesh = chunk->chunk_draw.getMesh();
        if(mesh == nullptr || !mesh->isUploaded()) continue;

		chunk->chunk_draw.draw(camera);
	} 
}
