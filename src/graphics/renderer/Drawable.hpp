//
// Created by 280325 on 8/8/2025.
//

#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <atomic>
#include <glm/ext.hpp>
#include <memory>
#include <GL/glew.h>

#include "../model/Mesh.hpp"
#include "../Transform.hpp"

#include "../Shader.hpp"
#include <iostream>

#include <shared_mutex>

class Camera;

class DrawableObject {
protected:
    Transform transform;
    
public:
    mutable std::shared_mutex meshMutex;
    std::shared_ptr<Mesh> mesh;
    Shader* shader = nullptr;
    
    DrawableObject() : mesh(nullptr) {}
    
    inline Transform& getTransform() {return transform;}

    void loadMesh(std::shared_ptr<Mesh> mesh);
    void unloadMesh();

    inline Mesh* getMesh() {return mesh.get();}
    inline std::shared_ptr<Mesh> getSharedMesh() {return mesh;}

    inline void loadShader(Shader* shader) {this->shader = shader;}

    virtual void draw(Camera* camera) {
        if(!shader) return;

        glUniformMatrix4fv(shader->model_loc, 1, GL_FALSE, glm::value_ptr(transform.model(camera)));
        if(mesh && mesh->isUploaded()) mesh->draw(3);
    };
};

#endif //DRAWABLE_HPP
