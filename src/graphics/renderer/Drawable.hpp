//
// Created by 280325 on 8/8/2025.
//

#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <glm/ext.hpp>
#include <memory>
#include <GL/glew.h>

#include "../model/Mesh.hpp"
#include "../Transform.hpp"

#include "../Shader.hpp"
#include <iostream>

class Camera;

class DrawableObject {
protected:
    std::unique_ptr<Mesh> mesh;

    Transform transform;
    
public:
    Shader* shader = nullptr;
    DrawableObject() : mesh(nullptr) {}
    
    inline Mesh* getMesh() {return mesh.get();}
    inline Transform& getTransform() {return transform;}

    inline void loadMesh(Mesh* mesh) {this->mesh.reset(mesh);}
    inline void loadShader(Shader* shader) {this->shader = shader;}

    virtual void draw(Camera* camera) {
        glUniformMatrix4fv(shader->model_loc, 1, GL_FALSE, glm::value_ptr(transform.model(camera)));
        mesh->draw(3);
    };
};

#endif //DRAWABLE_HPP
