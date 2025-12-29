#include "Drawable.hpp"
#include <memory>
#include <mutex>
#include <shared_mutex>

void DrawableObject::loadMesh(std::shared_ptr<Mesh> mesh) {
    this->mesh = std::move(mesh);
}

 void DrawableObject::unloadMesh() {
    this->mesh.reset();
 }