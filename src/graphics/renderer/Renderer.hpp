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
    virtual void render() = 0;
};

#endif //RENDitERER_HPP
