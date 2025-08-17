//
// Created by 280325 on 8/11/2025.
//

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>

class Frustum;
class Shader;
class Camera;
class Drawable;

class Renderer {
protected:    
    std::vector<Drawable*> drawable; 

    Camera* camera = nullptr; 
    Shader* shader = nullptr;
    Frustum* frustum = nullptr;

    friend class DrawContext;
public:
    Renderer(Camera* camera, Shader* shader, Frustum* frustum) : camera(camera), shader(shader), frustum(frustum) {}
    
    Renderer() = default;

    virtual void render() {};
};

#endif //RENDitERER_HPP
