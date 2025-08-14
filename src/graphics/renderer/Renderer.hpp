//
// Created by 280325 on 8/11/2025.
//

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>

class Shader;
class Camera;
class Drawable;

class Renderer {
    std::vector<Drawable*> drawable; 

    Camera* camera; 
    Shader* shader;
public:
    Renderer(Camera* camera, Shader* shader) : camera(camera), shader(shader) {}
};

#endif //RENDitERER_HPP
