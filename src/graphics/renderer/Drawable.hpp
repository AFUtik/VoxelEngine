//
// Created by 280325 on 8/8/2025.
//

#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <memory>
#include <GL/glew.h>

#include "../model/Mesh.hpp"
#include "../Transform.hpp"

class DrawableObject {
protected:
    std::unique_ptr<Mesh> mesh;
    
    Transform transform;
public:
    DrawableObject() : mesh(new Mesh) {}

    inline Mesh* getMesh() {return mesh.get();}
    inline Transform& getTransform() {return transform;}

    inline void draw() {mesh->draw(GL_TRIANGLES);}
};

#endif //DRAWABLE_HPP
