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
    std::vector<std::shared_ptr<Chunk>> toUpload;
    {
        std::lock_guard lk(mesher.meshUploadMutex);
        while (!mesher.meshUploadQueue.empty()) {
            auto &pr = mesher.meshUploadQueue.front();
            toUpload.push_back(pr);
            mesher.meshUploadQueue.pop();
        }
    }

    mesher.glController->processAll();

    for (auto &sp : toUpload) {
        if (!sp) continue;
        
        Mesh* mesh = sp->chunk_draw.getMesh();
        if (!mesh) continue;

        sp->chunk_draw.loadShader(shader);
        double px = double(sp->x) * double(ChunkInfo::WIDTH)  + 0.5;
        double py = double(sp->y) * double(ChunkInfo::HEIGHT) + 0.5;
        double pz = double(sp->z) * double(ChunkInfo::DEPTH)  + 0.5;
        sp->chunk_draw.getTransform().setPosition(glm::dvec3(px, py, pz)); 
    }

    std::vector<std::shared_ptr<Chunk>> chunksToDraw;
    {
        std::shared_lock<std::shared_mutex> mapLock(world->chunkMapMutex);
        for (const auto& [chunkPos, chunk] : world->chunkMap) {
            if (!chunk) continue;

            Mesh* mesh;
            {
                std::shared_lock<std::shared_mutex> wl(chunk->dataMutex);
                mesh = chunk->chunk_draw.getMesh();
            }

            if (chunk->isDirty()) {
                {
                    std::lock_guard lk(world->readyQueueMutex);
                    world->readyChunks.push(chunk);
                }
                world->readyCv.notify_one();
            }

            if (mesh && mesh->isUploaded()) chunksToDraw.push_back(chunk);
        }
    }
    for (auto &chunk : chunksToDraw) chunk->chunk_draw.draw(camera);
    
}
