#ifndef DRAWCONTEXT_HPP
#define DRAWCONTEXT_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

#include "Renderer.hpp"
#include <iostream>

class DrawContext {
    std::unique_ptr<Renderer> renderer_sample;

    std::unordered_map<std::string, std::unique_ptr<Renderer>> renderers;
    std::vector<Renderer*> enabled_renderers;
public:
    DrawContext(Renderer* sample) : renderer_sample(sample) {}

    template<typename T>
    void registerRenderer(std::string location, T* renderer) {
        static_assert(std::is_base_of<Renderer, T>::value, "T must derive from ObjectRenderer class");
        if (!renderer) throw std::runtime_error("Error: renderer is nullptr!");
        
        renderer->camera  = renderer_sample->camera;
        renderer->shader  = renderer_sample->shader;
        renderer->frustum = renderer_sample->frustum;
        
        renderers.emplace(location, std::unique_ptr<T>(renderer));
        enabled_renderers.push_back(renderer);
    }

    inline void disableRenderer(std::string location) {
        
    }

    inline void enableRenderer(std::string location) {

    }

    inline void render() {
        for(Renderer* renderer : enabled_renderers) renderer->render();
    }
};

#endif