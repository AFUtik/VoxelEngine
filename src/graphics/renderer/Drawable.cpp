#include "Drawable.hpp"
#include <memory>
#include <mutex>
#include <shared_mutex>

void DrawableObject::loadMesh(std::unique_ptr<Mesh> mesh) {
    std::unique_lock lock(meshMutex);
    this->mesh = std::move(mesh);
}