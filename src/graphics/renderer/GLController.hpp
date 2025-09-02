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
    uint32_t vertices;
    uint32_t verticesUpdated;
    Vertex*  verticesPtr;
};

class Mesh;

class GlController {
public:
    std::queue<gl_delete_cmd> glDelete;
    std::queue<std::shared_ptr<Mesh>> glUpdate;
    std::queue<std::shared_ptr<Mesh>> glUpload;

    std::mutex meshDeleteMutex;
    std::mutex meshUpdateMutex;
    std::mutex meshUploadMutex;

    void processAll();
};

#endif 