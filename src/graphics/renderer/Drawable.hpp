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
    std::shared_ptr<Mesh> mesh;
    Transform transform;

    std::atomic<bool> modified{true};
public:
    mutable std::shared_mutex meshMutex;
    Shader* shader = nullptr;
    
    DrawableObject() : mesh(nullptr) {}

	inline void modify() { 
		modified.store(true, std::memory_order_relaxed); 
	}

	inline void unmodify() {
		modified.store(false, std::memory_order_relaxed); 
	}

	inline bool isModified() const { return modified.load(std::memory_order_relaxed); }
    
    inline Transform& getTransform() {return transform;}

    void loadMesh(const std::shared_ptr<Mesh> &mesh);
    inline Mesh* getMesh() {return mesh.get();}
    inline std::shared_ptr<Mesh> getSharedMesh() {return mesh;}

    inline void loadShader(Shader* shader) {this->shader = shader;}

    virtual void draw(Camera* camera) {
        if(!shader) return;

        glUniformMatrix4fv(shader->model_loc, 1, GL_FALSE, glm::value_ptr(transform.model(camera)));
        mesh->draw(3);
    };
};

#endif //DRAWABLE_HPP
