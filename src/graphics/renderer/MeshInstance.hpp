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
protected:
    Transform transform;
public:
    Shader* shader = nullptr;
    MeshPtr mesh   = nullptr;
    AABB aabb;
    
    MeshInstance() : aabb(glm::dvec3(0,0,0), glm::dvec3(0,0,0)) {}
    
    inline Transform& getTransform() {return transform;}

    virtual void draw(Camera *camera) {
        //if (!camera->getFrustum().boxInFrustum(aabb)) return;

        if (!shader) return;

        glUniformMatrix4fv(shader->model_loc, 1, GL_FALSE, glm::value_ptr(transform.model(camera)));
        if (mesh && mesh->isUploaded()) {
            mesh->draw();
        }
    }
};

#endif //DRAWABLE_HPP
