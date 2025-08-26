#include "Drawable.hpp"
#include <mutex>
#include <shared_mutex>

void DrawableObject::loadMesh(Mesh* mesh) {
    std::unique_lock<std::shared_mutex> wlock(meshMutex);
    this->mesh.reset(mesh);
}