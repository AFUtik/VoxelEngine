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

    // --- 1. Сбор чанков для загрузки мешей ---
    {
        std::lock_guard lk(mesher.meshUploadMutex);
        while (!mesher.meshUploadQueue.empty()) {
            auto pr = mesher.meshUploadQueue.front();
            pr.second->chunk_draw.loadMesh(pr.first);
            toUpload.push_back(pr.second);
            mesher.meshUploadQueue.pop();
        }
    }

    // --- 2. Обработка загруженных мешей ---
    for (auto &sp : toUpload) {
        if (!sp) continue;

        std::shared_ptr<Mesh> mesh;
        {
            std::unique_lock<std::shared_mutex> lock(sp->dataMutex);
            mesh = sp->chunk_draw.getSharedMesh(); // shared_ptr гарантирует жизнь объекта
            if (!mesh) continue;

            sp->chunk_draw.loadShader(shader);

            double px = double(sp->x) * double(ChunkInfo::WIDTH)  + 0.5;
            double py = double(sp->y) * double(ChunkInfo::HEIGHT) + 0.5;
            double pz = double(sp->z) * double(ChunkInfo::DEPTH)  + 0.5;
            sp->chunk_draw.getTransform().setPosition(glm::dvec3(px, py, pz));

            if (!mesh->isUploaded())
                mesh->upload_buffers();
        }
        // draw можно вызвать без lock, если draw не модифицирует shared данные
        sp->chunk_draw.draw(camera);
    }

    std::vector<std::shared_ptr<Chunk>> chunksToDraw;
    {
        std::shared_lock<std::shared_mutex> mapLock(world->chunkMapMutex);
        for (const auto& [chunkPos, chunk] : world->chunkMap) {
            if (!chunk) continue;

            bool modified = false;
            std::shared_ptr<Mesh> mesh;

            modified = chunk->isModified();
            {
                std::shared_lock<std::shared_mutex> wl(chunk->dataMutex);
                mesh = chunk->chunk_draw.getSharedMesh();
            }

            if (modified) {
                {
                    std::lock_guard lk(world->readyQueueMutex);
                    world->readyChunks.push(chunk);
                    chunk->unmodify();
                }
                world->readyCv.notify_one();
            }

            if (mesh && mesh->isUploaded()) {
                chunksToDraw.push_back(chunk);
            }
        }
    }

    for (auto &chunk : chunksToDraw) chunk->chunk_draw.draw(camera);
    
}
