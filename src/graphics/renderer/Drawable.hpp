//
// Created by 280325 on 8/8/2025.
//

#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <memory>
#include <GL/glew.h>

#include "../model/Mesh.hpp"

class DrawableObject {
protected:

public:
    std::unique_ptr<Mesh> mesh;
    glm::mat4 model;
    DrawableObject() : mesh(new Mesh), model(glm::mat4(1.0f)) {}

    inline Mesh* getMesh() {return mesh.get();}
    inline glm::mat4& getTransform() {return model;}

    inline void draw() {mesh->draw(GL_TRIANGLES);}
};

#endif //DRAWABLE_HPP
