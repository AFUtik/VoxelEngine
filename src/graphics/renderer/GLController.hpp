#ifndef GLCONTROLLER_HPP
#define GLCONTROLLER_HPP

#include "../vertex/VertexInfo.hpp"
#include <deque>
#include <memory>
#include <mutex>

struct gl_delete_cmd {
    uint32_t vbo; 
    uint32_t vao;
    uint32_t ebo;
};

struct gl_update_cmd {
    uint32_t vbo; 
    uint32_t vertices;
    uint32_t verticesUpdated;
    Vertex*  verticesPtr;
};

class Mesh;

class GlController {
public:
    std::deque<gl_delete_cmd> glDelete;
    std::deque<std::shared_ptr<Mesh>> glUpdate;
    std::deque<std::shared_ptr<Mesh>> glUpload;

    std::mutex meshDeleteMutex;
    std::mutex meshUpdateMutex;
    std::mutex meshUploadMutex;

    std::shared_ptr<Mesh> createMesh();
    void queueUpload(std::shared_ptr<Mesh> mesh);
    void queueFree  (std::shared_ptr<Mesh> mesh);

    void processAll();
};

#endif 