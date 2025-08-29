#ifndef GLCONTROLLER_HPP
#define GLCONTROLLER_HPP

#include "../vertex/VertexInfo.hpp"
#include <queue>
#include <memory>
#include <mutex>

struct gl_delete_cmd {
    uint32_t vbo; 
    uint32_t vao;
};

struct gl_update_cmd {
    uint32_t vbo; 
    uint32_t vertex_count;
    Vertex*  vertices;
};

class Mesh;

class GlController {
public:
    std::queue<gl_delete_cmd> glDelete;
    std::queue<gl_update_cmd> glUpdate;
    std::queue<Mesh*> glUpload;

    std::mutex meshDeleteMutex;
    std::mutex meshUploadMutex;

    void processAll();
};

#endif 