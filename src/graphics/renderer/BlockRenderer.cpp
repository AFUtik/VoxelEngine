#include "BlockRenderer.hpp"

#include "../Frustum.hpp"
#include "../../logic/blocks/Chunk.hpp"
#include "../../logic/blocks/ChunkInfo.hpp"
#include "../../logic/World.hpp"

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
            auto pr = mesher.meshUploadQueue.front();
            mesher.meshUploadQueue.pop();

            auto &chunk = pr.first;
            auto &mesh  = pr.second;

            glGenVertexArrays(1, &mesh->VAO);
            glGenBuffers(1, &mesh->VBO);
            
            // VBO
            glBindVertexArray(mesh->VAO);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * mesh->buffer.size(), mesh->buffer.data(), GL_STATIC_DRAW);

            // EBO
            glGenBuffers(1, &mesh->EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size()*sizeof(GLuint), mesh->indices.data(), GL_STATIC_DRAW);

            // attributes
            uint32_t offset = 0;
            uint32_t i = 0;
            while(ATTRIBUTES[i]) {
                const uint32_t &size = ATTRIBUTES[i];
                glVertexAttribPointer(i, size, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(GLfloat), (GLvoid*)(offset * sizeof(float)));
                glEnableVertexAttribArray(i);
                offset += size;
                i++;
            }
            glBindVertexArray(0);
            
            mesh->uploaded = true;
            mesh->vertices = mesh->buffer.size();
            mesh->clearBuffers();

            chunk->drawable.loadMesh(mesh);
            toUpload.push_back(chunk);
        }
    }
    mesher.glController->processAll();

    for (auto &sp : toUpload) {
        if (!sp) continue;
        sp->drawable.loadShader(shader);
        double px = double(sp->x) * double(ChunkInfo::WIDTH);
        double py = double(sp->y) * double(ChunkInfo::HEIGHT);
        double pz = double(sp->z) * double(ChunkInfo::DEPTH);
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
