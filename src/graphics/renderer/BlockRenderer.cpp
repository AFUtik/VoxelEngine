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
            auto& pr = mesher.meshUploadQueue.front();
            pr.second->chunk_draw.loadMesh(pr.first);
            toUpload.push_back(pr.second);
            mesher.meshUploadQueue.pop();
        }
    }
    
    for (auto &sp : toUpload) {
        if(!sp) continue;

        Mesh* mesh;
        {
            std::unique_lock<std::shared_mutex> sl(sp->chunk_draw.meshMutex);
            mesh = sp->chunk_draw.getMesh();
            
            sp->chunk_draw.loadShader(shader);
            double px = double(sp->x) * double(ChunkInfo::WIDTH)  + 0.5;
            double py = double(sp->y) * double(ChunkInfo::HEIGHT) + 0.5;
            double pz = double(sp->z) * double(ChunkInfo::DEPTH)  + 0.5;
            sp->chunk_draw.getTransform().setPosition(glm::dvec3(px, py, pz));

            if(!mesh->isUploaded()) mesh->upload_buffers();
        }
    }

	{
        std::shared_lock<std::shared_mutex> mapLock(world->chunkMapMutex);
        for (const auto& [chunkPos, chunk] : world->chunkMap) {
            
            bool modified;
            {
                std::shared_lock<std::shared_mutex> dl(chunk->dataMutex);
                modified = chunk->isModified();
            }
            
            if (modified) {
                {
                    std::lock_guard lk(world->readyQueueMutex);
                    world->readyChunks.push(chunk);
                }

                chunk->unmodify();

                world->readyCv.notify_one();
            }

            {
                std::shared_lock<std::shared_mutex> dl(chunk->dataMutex);
                Mesh* mesh = chunk->chunk_draw.getMesh();
                if (mesh == nullptr || !mesh->isUploaded()) continue;  
            }
            chunk->chunk_draw.draw(camera);
        }
    }
}
