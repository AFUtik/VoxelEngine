#include "GlController.hpp"
#include "../model/Mesh.hpp"

#include <GL/glew.h>

void GlController::processAll() {
    {
        std::lock_guard<std::mutex> lk(meshDeleteMutex);
        if (!glDelete.empty()) {
            gl_delete_cmd pr = glDelete.front();
            glDelete.pop();

            glDeleteVertexArrays(1, &pr.vao);
            glDeleteBuffers(1, &pr.vbo);
        }
    }

    {
        std::lock_guard<std::mutex> lk(meshUpdateMutex);
        if (!glUpdate.empty()) {
            gl_update_cmd pr = glUpdate.front();
            glUpdate.pop();

            size_t size = sizeof(float) * VERTEX_SIZE * pr.vertex_count;

            glBindBuffer(GL_ARRAY_BUFFER, pr.vbo);
            void* ptr = glMapBufferRange(
                GL_ARRAY_BUFFER,
                0, size,
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
            );
            memcpy(ptr, pr.vertices, size);
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }
    }

    {
        std::lock_guard<std::mutex> lk(meshUploadMutex);
        while (!glUpload.empty()) {
            auto mesh = glUpload.front();
            glUpload.pop();

            if(!mesh || mesh->uploaded) continue;

            mesh->vertices = mesh->buffer->vertices.get_size();

            glGenVertexArrays(1, &mesh->VAO);
            glGenBuffers(1, &mesh->VBO);

            glBindVertexArray(mesh->VAO);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * mesh->buffer->vertices.get_size(), mesh->buffer->vertices.get_data(), GL_STATIC_DRAW);

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
}