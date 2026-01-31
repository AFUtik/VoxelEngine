//
// Created by 280325 on 8/11/2025.
//

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>

class Frustum;
class Shader;
class Camera;
class MeshInstance;
class Client; 

class Renderer {
protected:    
    std::vector<MeshInstance*> drawable; 
       
    Client* client = nullptr;
    Camera* camera = nullptr; 

    friend class DrawContext;
public:
    Renderer(Client* client, Camera* camera) : client(client), camera(camera) {}
    
    Renderer() = default;
    
    virtual void render() {};
};

#endif //RENDitERER_HPP
