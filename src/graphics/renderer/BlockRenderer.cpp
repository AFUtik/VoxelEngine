#include "BlockRenderer.hpp"

#include "../Frustum.hpp"
#include "../../logic/blocks/Chunk.hpp"
#include "../../logic/blocks/ChunkInfo.hpp"
#include "../../logic/LogicSystem.hpp"

#include "../../graphics/Transform.hpp"

#include <glm/ext.hpp>
#include <memory>
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
        
        auto mesh = sp->drawable.getSharedMesh();
        if (!mesh) continue;

        sp->drawable.loadShader(shader);
        double px = double(sp->x) * double(ChunkInfo::WIDTH)  + 0.5;
        double py = double(sp->y) * double(ChunkInfo::HEIGHT) + 0.5;
        double pz = double(sp->z) * double(ChunkInfo::DEPTH)  + 0.5;
        sp->drawable.getTransform().setPosition(glm::dvec3(px, py, pz)); 
    } 

    {
        std::shared_lock<std::shared_mutex> mapLock(world->chunkMapMutex);
        for (const auto& [chunkPos, chunk] : world->chunkMap) {
            if (!chunk) continue;

            if(frustum->boxInFrustum(chunk->min, chunk->max)) chunk->drawable.draw(camera);
        }
    }
}
