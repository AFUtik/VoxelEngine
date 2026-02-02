#ifndef MESHINSTANCE_HPP
#define MESHINSTANCE_HPP

#include <GL/glew.h>

#include "../model/Mesh.hpp"
#include "../Transform.hpp"
#include "../Shader.hpp"
#include "../Camera.hpp"

#include <physics.hpp>

class Camera;

class MeshInstance {
public:
    Transform transform;
    Shader* shader = nullptr;
    MeshPtr mesh   = nullptr;
    AABB aabb;

    bool has_aabb = false;

    MeshInstance() {};

    inline void setPosition(const Vector3& vector) {
        transform.setPosition(vector);
        if (has_aabb) aabb.translate(vector);
    }

    virtual void draw(Camera *camera) {
        if (has_aabb && !camera->getFrustum().boxInFrustum(aabb)) return;

        if (!shader) return;

        glUniformMatrix4fv(shader->model_loc, 1, GL_FALSE, glm::value_ptr(transform.model(camera)));
        if (mesh && mesh->isUploaded()) {
            mesh->draw();
        }
    }
};

class MeshGroup {
    std::vector<MeshInstance> instances;
    AABB aabb;

    bool has_aabb = false;

    virtual void draw(Camera* camera) {
        if (!has_aabb || !camera->getFrustum().boxInFrustum(aabb)) return;

        for (auto &&meshInstance : instances) {
            if (!camera->getFrustum().boxInFrustum(meshInstance.aabb)) return;
        }
    }
};

#endif //DRAWABLE_HPP
