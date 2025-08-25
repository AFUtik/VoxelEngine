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
    std::vector<std::weak_ptr<Chunk>> toUpload;
    {
        std::lock_guard lk(mesher.meshUploadMutex); // короткий лок
        while (!mesher.meshUploadQueue.empty()) {
            // предполагаем, что queue хранит std::shared_ptr<Chunk>
            toUpload.push_back(mesher.meshUploadQueue.front());
            mesher.meshUploadQueue.pop();
        }
    }

    // --- 2) Выполним тяжелую работу (OpenGL upload) без удержания mutex'а
    for (auto &weak : toUpload) {
        if (auto sp = weak.lock()) {
            {
                if(sp->chunk_draw.getMesh()->isUploaded()) continue;

                sp->chunk_draw.loadShader(shader);
                sp->chunk_draw.getMesh()->upload_buffers();
                double px = double(sp->x) * double(ChunkInfo::WIDTH)  + 0.5;
                double py = double(sp->y) * double(ChunkInfo::HEIGHT) + 0.5;
                double pz = double(sp->z) * double(ChunkInfo::DEPTH)  + 0.5;
                sp->chunk_draw.getTransform().setPosition(glm::dvec3(px, py, pz));
            }
        }
    }

	{
        std::shared_lock<std::shared_mutex> mapLock(world->chunkMapMutex);
        for (const auto& [chunkPos, chunk] : world->chunkMap) {
            bool modified;
            {
                std::shared_lock<std::shared_mutex> dl(chunk->dataMutex);
                modified = chunk->isModified(); // если isModified читает atomic, то это тоже ок
            }
            
            if (modified) {
                {
                    std::lock_guard lk(world->readyQueueMutex);
                    world->readyChunks.push(chunk);
                }
                {
                    std::unique_lock<std::shared_mutex> dl(chunk->dataMutex);
                    chunk->unmodify();
                }
                world->readyCv.notify_one();
            }

            // нарисовать — только если mesh загружен
            Mesh* mesh = nullptr;
            {
                std::shared_lock<std::shared_mutex> dl(chunk->dataMutex);
                mesh = chunk->chunk_draw.getMesh();
                if (mesh == nullptr || !mesh->isUploaded()) continue;
                
            }
            chunk->chunk_draw.draw(camera); // draw сам по себе должен быть потокобезопасен / в main thread с контекстом
        }
    }
}
