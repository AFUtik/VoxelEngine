#include "Drawable.hpp"
#include <memory>
#include <mutex>
#include <shared_mutex>

void DrawableObject::loadMesh(std::shared_ptr<Mesh> mesh) {
    std::unique_lock lock(meshMutex);
    this->mesh = std::move(mesh);
}

std::shared_ptr<Mesh> DrawableObject::getSharedMesh() {
    std::shared_lock lock(meshMutex);
    return mesh;
}