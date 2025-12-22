#include "GlController.hpp"
#include "../model/Mesh.hpp"

#include <GL/glew.h>
#include <mutex>
#include <shared_mutex>

void GlController::processAll() {
    {
        std::lock_guard<std::mutex> lk(meshUploadMutex);
        while (!glUpload.empty()) {
            auto mesh = glUpload.front();
            glUpload.pop();

            if(!mesh || mesh->uploaded) continue;

            mesh->vertices = mesh->buffer->vertices.get_size();
            mesh->verticesUpdated = mesh->vertices;

            glGenVertexArrays(1, &mesh->VAO);
            glGenBuffers(1, &mesh->VBO);

            glBindVertexArray(mesh->VAO);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * mesh->buffer->vertices.get_size(), mesh->buffer->vertices.get_data(), GL_DYNAMIC_DRAW);

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
        }
    }
    
    {
        std::lock_guard<std::mutex> lk(meshUpdateMutex);
        while (!glUpdate.empty()) {
            auto pr = glUpdate.front();
            glUpdate.pop();

            size_t size = sizeof(float) * VERTEX_SIZE * pr->verticesUpdated;

            //std::lock_guard<std::mutex> l(pr->mutex);
            glBindBuffer(GL_ARRAY_BUFFER, pr->VBO);
            if(pr->verticesUpdated > pr->vertices) {
                glBufferData(GL_ARRAY_BUFFER, size, pr->buffer->vertices.get_data(), GL_DYNAMIC_DRAW);
                pr->vertices = pr->verticesUpdated;
            } else {
                glBufferSubData(GL_ARRAY_BUFFER, 0, size, pr->buffer->vertices.get_data());
            }
        }
    }

    {
        std::lock_guard<std::mutex> lk(meshDeleteMutex);
        while (!glDelete.empty()) {
            gl_delete_cmd pr = glDelete.front();
            glDelete.pop();

            glDeleteVertexArrays(1, &pr.vao);
            glDeleteBuffers(1, &pr.vbo);
        }
    }
}