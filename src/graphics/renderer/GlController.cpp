#include "GlController.hpp"
#include "../model/Mesh.hpp"

#include <GL/glew.h>
#include <mutex>
#include <shared_mutex>

#include <iostream>

void GlController::processAll() {
    {
        std::lock_guard<std::mutex> lk(meshDeleteMutex);
        while (!glDelete.empty()) {
            gl_delete_cmd pr = glDelete.front();
            glDelete.pop();

            glDeleteVertexArrays(1, &pr.vao);
            glDeleteBuffers(1, &pr.vbo);
            glDeleteBuffers(1, &pr.ebo);
        }
    }
    
    {
        std::lock_guard<std::mutex> lk(meshUploadMutex);
        while (!glUpload.empty()) {
            auto mesh = glUpload.front();
            glUpload.pop();
            
            if(!mesh || mesh->uploaded) continue;

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
        }
    }

    
}